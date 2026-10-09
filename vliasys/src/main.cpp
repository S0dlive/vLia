#include <iostream>
#include <fstream>
#include <csignal>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <random>
#include <sstream>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include "config/configmanager.h"
#include "logging/logging.h"
#include "runtime/agentruntime.h"
#include "ipc/ipcserver.h"

using json = nlohmann::json;

std::atomic<bool> g_running{true};
std::condition_variable g_cv;
std::mutex g_mutex;

void signalHandler(int signal) {
    spdlog::info("Signal de fermeture reçu (" + std::to_string(signal) + "), arrêt de vliasys...");
    g_running = false;
    g_cv.notify_all();
}

std::string generateAgentId() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, 0xFFFF);

    std::stringstream ss;
    ss << "agent-"
       << std::hex << std::setfill('0')
       << std::setw(4) << dis(gen) << "-"
       << std::setw(4) << dis(gen);
    return ss.str();
}

int main() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    logging::initLogging();
    logging::activeDebugLogging();

    spdlog::info("Démarrage de vLia Runtime & Serveur IPC...");

    std::string configPath = "/etc/vlia/vliasys.json";
    std::string socketPath = "/tmp/vliasys.sock";
    std::string model = "";
    std::string agentId = "";

    json configJson;
    bool configLoaded = false;

    std::ifstream configFile(configPath);
    if (configFile.is_open()) {
        try {
            configFile >> configJson;
            model = configJson.value("model", "");
            agentId = configJson.value("agent_id", "");
            socketPath = configJson.value("socket_path", socketPath);
            configLoaded = true;
            spdlog::info("Configuration chargée depuis " + configPath);
        } catch (const std::exception& e) {
            spdlog::warn("Erreur de parsing du fichier " + configPath + " (" + std::string(e.what()) + ").");
        }
        configFile.close();
    } else {
        spdlog::warn("Fichier de configuration " + configPath + " introuvable.");
    }

    if (model.empty()) {
        spdlog::error("Aucun modèle configuré dans " + configPath + " !");
        spdlog::error("Veuillez spécifier le modèle à utiliser (ex: 'model': 'qwen2.5-coder:latest') via 'vlia config'.");
        return 1;
    }

    if (agentId.empty()) {
        agentId = generateAgentId();
        spdlog::info("Aucun Agent ID trouvé. Génération automatique : [" + agentId + "]");

        if (configLoaded) {
            try {
                configJson["agent_id"] = agentId;
                std::ofstream outFile(configPath);
                if (outFile.is_open()) {
                    outFile << configJson.dump(4);
                    outFile.close();
                    spdlog::info("Agent ID sauvegardé dans " + configPath);
                }
            } catch (const std::exception& e) {
                spdlog::warn("Impossible de sauvegarder l'Agent ID dans la configuration : " + std::string(e.what()));
            }
        }
    }

    try {
        configManager cfgMgr(model);
        cfgMgr.load();
        agentRuntime runtime(cfgMgr, agentId);
        spdlog::info("Initialisation de l'environnement avec le modèle [" + model + "] et l'Agent ID [" + agentId + "]...");
        runtime.setup();
        ipcServer server(runtime, socketPath);
        server.start();
        spdlog::info("vliasys est prêt et attend des commandes IPC sur [" + socketPath + "].");
        std::unique_lock<std::mutex> lock(g_mutex);
        g_cv.wait(lock, [] { return !g_running.load(); });
        spdlog::info("Fermeture du serveur IPC...");
        server.stop();
    } catch (const std::exception& e) {
        spdlog::error("Erreur fatale : " + std::string(e.what()));
        return 1;
    }

    spdlog::info("vliasys arrêté proprement.");
    return 0;
}