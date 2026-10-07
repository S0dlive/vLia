//
// Created by baptisteluciani on 07/10/2026.
//

#ifndef VLIASYS_MCPSERVERSPEC_H
#define VLIASYS_MCPSERVERSPEC_H

#include <string>
#include <vector>

struct mcpServerSpec {
    std::string name;
    std::string repoUrl;

    std::string installCmd;

    std::string executable;
    std::vector<std::string> args;
};
#endif //VLIASYS_MCPSERVERSPEC_H
