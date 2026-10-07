//
// Created by baptisteluciani on 07/10/2026.
//
#include "configmanager.h"
#include <fstream>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include "configmanager.h"

using json = nlohmann::json;

configManager::configManager(std::string defaultModel)
    : m_configPath("vlia_config.json") {
    m_globalConfig.defaultModel = std::move(defaultModel);
}

void configManager::load() {
    if (!std::filesystem::exists(m_configPath)) {
        spdlog::warn("Config file '" +  m_configPath.string() + "' not found, using default fallback settings");
        m_globalConfig.nodeId = "node-default";
        m_globalConfig.baseWorkspaceDir = "./workspaces";
        m_globalConfig.bwrapBinary = "/usr/bin/bwrap";
        m_globalConfig.defaultAllowNetwork = false;
        m_globalConfig.ollamaEndpoint = "http://127.0.0.1:11434";
        return;
    }

    try {
        std::ifstream file(m_configPath);
        json j;
        file >> j;

        m_globalConfig.nodeId = j.value("node_id", "node-default");
        m_globalConfig.baseWorkspaceDir = j.value("base_workspace_dir", "./workspaces");
        m_globalConfig.bwrapBinary = j.value("bwrap_binary", "/usr/bin/bwrap");

        if (j.contains("default_sandbox")) {
            const auto& sb = j["default_sandbox"];
            m_globalConfig.defaultAllowNetwork = sb.value("allow_network", false);

            if (sb.contains("shared_paths") && sb["shared_paths"].is_object()) {
                m_globalConfig.defaultSharedPaths.clear();
                for (const auto& [key, val] : sb["shared_paths"].items()) {
                    m_globalConfig.defaultSharedPaths.emplace_back(key, std::filesystem::path(val.get<std::string>()));
                }
            }
        }

        if (j.contains("runtime")) {
            m_globalConfig.ollamaEndpoint = j["runtime"].value("ollama_endpoint", "http://127.0.0.1:11434");
            if (j["runtime"].contains("default_model")) {
                m_globalConfig.defaultModel = j["runtime"]["default_model"].get<std::string>();
            }
        }

        spdlog::info("Configuration loaded from " + m_configPath.string());
    } catch (const std::exception& e) {
        spdlog::error("Failed to parse config file: " + std::string(e.what()));
        throw;
    }
}

sandboxConfig configManager::createSandboxConfigForAgent(const std::string& agentId) {
    if (agentId.empty()) {
        throw std::invalid_argument("agentId cannot be empty");
    }

    sandboxConfig cfg;
    cfg.agentId = agentId;
    cfg.bwrapBinary = m_globalConfig.bwrapBinary;
    cfg.allowNetwork = m_globalConfig.defaultAllowNetwork;

    cfg.workspacePath = std::filesystem::path(m_globalConfig.baseWorkspaceDir) / "agents" / agentId;
    cfg.sharedPaths = m_globalConfig.defaultSharedPaths;

    cfg.environment["AGENT_ID"] = agentId;
    cfg.environment["HOME"] = "/workspace";
    cfg.environment["TMPDIR"] = "/tmp";
    cfg.environment["PATH"] = "/usr/local/bin:/usr/bin:/bin";

    return cfg;
}