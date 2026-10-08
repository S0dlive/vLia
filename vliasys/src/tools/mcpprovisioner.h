#ifndef VLIASYS_MCPPROVISIONER_H
#define VLIASYS_MCPPROVISIONER_H

#include <filesystem>
#include <spdlog/spdlog.h>
#include "mcpserverspec.h"
#include "../sandbox/sandbox.h"

class mcpProvisioner {
public:
    static bool ensureServerInstalled(const sandboxConfig& baseConfig, const mcpServerSpec& spec) {
        if (spec.repoUrl.empty() && spec.installCmd.empty()) {
            spdlog::info("MCP Server '" + spec.name + "' est pré-installé ou natif.");
            return true;
        }

        std::filesystem::path targetDir = baseConfig.workspacePath / "mcp_servers" / spec.name;

        if (std::filesystem::exists(targetDir) && !std::filesystem::is_empty(targetDir)) {
            spdlog::info("MCP Server '" + spec.name + "' déjà présent dans " + targetDir.string());
            return true;
        }

        spdlog::info("Provisioning MCP Server '" + spec.name + "'...");

        sandboxConfig setupCfg = baseConfig;
        setupCfg.allowNetwork = true;
        sandbox setupBox(setupCfg);

        std::string targetInContainer = "/workspace/mcp_servers/" + spec.name;
        std::string cmd;

        if (!spec.repoUrl.empty()) {
            cmd = "mkdir -p /workspace/mcp_servers && git clone " + spec.repoUrl + " " + targetInContainer;
            if (!spec.installCmd.empty()) {
                cmd += " && cd " + targetInContainer + " && " + spec.installCmd;
            }
        } else {
            cmd = spec.installCmd;
        }

        auto res = setupBox.runInSandbox("/usr/bin/sh", {"-c", cmd});
        if (res.exitCode != 0) {
            spdlog::error("Failed to provision MCP server '" + spec.name + "': " + res.stderr_output);
            return false;
        }

        spdlog::info("MCP Server '" + spec.name + "' provisionné avec succès!");
        return true;
    }
};

#endif // VLIASYS_MCPPROVISIONER_H