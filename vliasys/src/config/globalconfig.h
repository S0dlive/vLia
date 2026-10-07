//
// Created by baptisteluciani on 07/10/2026.
//

#ifndef VLIASYS_GLOBALCONFIG_H
#define VLIASYS_GLOBALCONFIG_H
#include <filesystem>
#include <vector>

#include "../tools/mcpserverspec.h"

struct mcpConfigSpec {
    std::string name;
    bool enabled{true};
    std::string repoUrl;
    std::string installCmd;
    std::string executable;
    std::vector<std::string> args;
};
struct globalConfig {
    std::string nodeId;
    std::filesystem::path baseWorkspaceDir;
    std::string bwrapBinary;
    bool defaultAllowNetwork{false};
    std::vector<std::pair<std::string, std::filesystem::path>> defaultSharedPaths;
    std::string ollamaEndpoint;
    std::string defaultModel;
    std::vector<mcpServerSpec> mcpServers;
};

#endif //VLIASYS_GLOBALCONFIG_H
