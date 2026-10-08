//
// Created by baptisteluciani on 07/10/2026.
//

#ifndef VLIASYS_MCPPROVISIONER_H
#define VLIASYS_MCPPROVISIONER_H

#include <filesystem>
#include <spdlog/spdlog.h>
#include "mcpserverspec.h"
#include "../sandbox/sandbox.h"

class mcpProvisioner {
public:
    static bool ensureServerInstalled(const sandboxConfig& baseConfig, const mcpServerSpec& spec) {
        std::filesystem::path targetDir = baseConfig.workspacePath / "mcp_servers" / spec.name;



        if (std::filesystem::exists(targetDir)) {
            spdlog::info("MCP Server '"+ spec.name + "' already installed in " +  targetDir.string());
            return true;
        }

        if (spec.repoUrl.empty()) {
            spdlog::info("MCP Server '"+ spec.name + "' est un serveur local, aucun clone Git requis.");
            return true;
        }

        spdlog::info("Provisioning MCP Server '" + spec.name + "' from " +  spec.repoUrl);

        sandboxConfig setupCfg = baseConfig;
        setupCfg.allowNetwork = true;
        sandbox setupBox(setupCfg);

        std::string targetInContainer = "/workspace/mcp_servers/" + spec.name;

        std::string cmd = "mkdir -p /workspace/mcp_servers && git clone " + spec.repoUrl + " " + targetInContainer;

        if (!spec.installCmd.empty()) {
            cmd += " && cd " + targetInContainer + " && " + spec.installCmd;
        }

        auto res = setupBox.runInSandbox("/usr/bin/sh", {"-c", cmd});
        if (res.exitCode != 0) {
            spdlog::error("Failed to provision MCP server '" +  spec.name + "': " + spec.name + " " + res.stderr_output);
            return false;
        }

        spdlog::info("MCP Server '"+  spec.name + "' successfully provisioned!");
        return true;
    }
};
#endif //VLIASYS_MCPPROVISIONER_H
