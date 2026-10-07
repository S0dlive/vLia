//
// Created by baptisteluciani on 07/10/2026.
//

#ifndef VLIASYS_SANDBOX_H
#define VLIASYS_SANDBOX_H
#include "sandboxconfig.h"
#include "../process/process.h"


class sandbox {
public:
    explicit sandbox(sandboxConfig config);

    ~sandbox() = default;

    processResult runInSandbox(const std::string &executable,
                               const std::vector<std::string> &args);

    [[nodiscard]] std::vector<std::string> buildBwrapArgs(
        const std::string &executable,
        const std::vector<std::string> &args) const;

private:

    void ensureDirectories() const;

    sandboxConfig config;
};

#endif //VLIASYS_SANDBOX_H
