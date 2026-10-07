//
// Created by baptisteluciani on 07/10/2026.
//

#ifndef VLIASYS_CONFIGMANAGER_H
#define VLIASYS_CONFIGMANAGER_H
#include <string>

#include "globalconfig.h"
#include "../sandbox/sandbox.h"


class configManager {
public :
    explicit configManager(std::string defaultModel);
    void load();
    [[nodiscard]]
    sandboxConfig createSandboxConfigForAgent(const std::string& agentId);
    [[nodiscard]]
    const globalConfig getGlobalConfig() const {return m_globalConfig;};
private :
    std::filesystem::path m_configPath;
    globalConfig m_globalConfig;
};


#endif //VLIASYS_CONFIGMANAGER_H
