//
// Created by baptisteluciani on 07/10/2026.
//

#ifndef VLIASYS_MCPCLIENT_H
#define VLIASYS_MCPCLIENT_H
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "../sandbox/sandbox.h"
#include "../process/process.h"

using json = nlohmann::json;

class mcpClient {
public:
    mcpClient(sandbox& box, const std::string& mcpExecutable, const std::vector<std::string>& args) {
        // Lancement du serveur MCP (Python / Node) à l'intérieur de bwrap
        m_proc = box.runInteractiveInSandbox(mcpExecutable, args);
    }

    ~mcpClient() {
        m_proc.close();
    }

    bool initialize() {
        json initReq = {
            {"jsonrpc", "2.0"},
            {"id", m_nextId++},
            {"method", "initialize"},
            {"params", {
                    {"protocolVersion", "2024-11-05"},
                    {"capabilities", json::object()},
                    {"clientInfo", {{"name", "vLia"}, {"version", "1.0"}}}
            }}
        };

        sendJson(initReq);
        auto resp = readJson();
        return resp.contains("result");
    }

    json listTools() {
        json req = {
            {"jsonrpc", "2.0"},
            {"id", m_nextId++},
            {"method", "tools/list"}
        };
        sendJson(req);
        auto resp = readJson();
        return resp.value("result", json::object()).value("tools", json::array());
    }

    json callTool(const std::string& name, const json& args) {
        json req = {
            {"jsonrpc", "2.0"},
            {"id", m_nextId++},
            {"method", "tools/call"},
            {"params", {
                    {"name", name},
                    {"arguments", args}
            }}
        };
        sendJson(req);
        auto resp = readJson();
        return resp.value("result", json::object());
    }

private:
    void sendJson(const json& j) {
        m_proc.writeLine(j.dump());
    }

    json readJson() {
        std::string raw = m_proc.readLine();
        if (raw.empty()) return json::object();
        return json::parse(raw);
    }

    interactiveProcess m_proc;
    int m_nextId{1};
};

#endif //VLIASYS_MCPCLIENT_H
