//
// Created by baptisteluciani on 07/10/2026.
//

#ifndef VLIASYS_SANDBOXCONFIG_H
#define VLIASYS_SANDBOXCONFIG_H
#include <filesystem>
#include <unordered_map>
#include <vector>

struct sandboxConfig {
    std::string agentId;
    std::filesystem::path workspacePath;
    std::vector<std::pair<std::string, std::filesystem::path>> sharedPaths;
    bool allowNetwork = false;
    std::unordered_map<std::string, std::string> environment = {
        {"PATH", "/usr/local/bin:/usr/bin:/bin"},
        {"HOME", "/workspace"},
        {"TMPDIR", "/tmp"},
    };
    std::string bwrapBinary = "bwrap";
};


#endif //VLIASYS_SANDBOXCONFIG_H
