//
// Created by baptisteluciani on 08/10/2026.
//

#include "ipcserver.h"
#include "../logging/logging.h"
#include <nlohmann/json.hpp>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <spdlog/spdlog.h>
#include <vector>
#include <sstream>

using json = nlohmann::json;

ipcServer::ipcServer(agentRuntime& runtime, std::string socketPath)
    : m_runtime(runtime), m_socketPath(std::move(socketPath)) {}

ipcServer::~ipcServer() {
    stop();
}

void ipcServer::start() {
    m_running = true;
    m_workerThread = std::thread(&ipcServer::run, this);
}

void ipcServer::stop() {
    if (!m_running) return;
    m_running = false;

    if (m_serverFd != -1) {
        close(m_serverFd);
        m_serverFd = -1;
    }
    unlink(m_socketPath.c_str());

    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void ipcServer::run() {
    unlink(m_socketPath.c_str());

    m_serverFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_serverFd < 0) {
        spdlog::error("[ipc] Échec de création de la socket UNIX");
        return;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, m_socketPath.c_str(), sizeof(addr.sun_path) - 1);

    if (bind(m_serverFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        spdlog::error("[ipc] Échec du bind sur " + m_socketPath);
        close(m_serverFd);
        return;
    }

    if (listen(m_serverFd, 5) < 0) {
        spdlog::error("[ipc] Échec du listen sur " + m_socketPath);
        close(m_serverFd);
        return;
    }

    spdlog::info("[ipc] Serveur IPC UNIX à l'écoute sur " + m_socketPath);

    while (m_running) {
        int clientFd = accept(m_serverFd, nullptr, nullptr);
        if (clientFd < 0) {
            if (!m_running) break;
            continue;
        }
        std::thread(&ipcServer::handleClient, this, clientFd).detach();
    }
}

void ipcServer::handleClient(int clientFd) {
    std::vector<char> buffer(4096);
    std::string rawData;

    while (true) {
        ssize_t bytesRead = read(clientFd, buffer.data(), buffer.size());
        if (bytesRead <= 0) break;
        rawData.append(buffer.data(), bytesRead);
        if (rawData.find('\n') != std::string::npos) break;
    }

    if (rawData.empty()) {
        close(clientFd);
        return;
    }

    try {
        json req = json::parse(rawData);
        std::string taskId = req.value("task_id", "unknown");
        std::string prompt = req.value("prompt", "");

        spdlog::info("[ipc] Requête reçue pour task '" + taskId + "' : " + prompt);

        std::string output = m_runtime.runUserQuery(prompt);

        json resp = {
            {"task_id", taskId},
            {"success", true},
            {"output", output},
            {"error", ""}
        };

        std::string responseStr = resp.dump() + "\n";
        write(clientFd, responseStr.c_str(), responseStr.size());

    } catch (const std::exception& e) {
        spdlog::error("[ipc] Erreur de traitement IPC : " + std::string(e.what()));
        json errResp = {
            {"task_id", "error"},
            {"success", false},
            {"output", ""},
            {"error", e.what()}
        };
        std::string responseStr = errResp.dump() + "\n";
        write(clientFd, responseStr.c_str(), responseStr.size());
    }

    close(clientFd);
}