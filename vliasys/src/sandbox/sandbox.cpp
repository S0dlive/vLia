//
// Created by baptisteluciani on 07/10/2026.
//

#include "sandbox.h"
#include <spdlog/spdlog.h>
#include <unistd.h>

namespace {
    std::string resolveExecutablePath(const std::string &binary) {
        if (binary.find('/') != std::string::npos) {
            return binary;
        }
        const char *pathEnv = std::getenv("PATH");
        if (pathEnv == nullptr) {
            return "";
        }
        std::istringstream ss(pathEnv);
        std::string dir;
        while (std::getline(ss, dir, ':')) {
            if (dir.empty()) dir = ".";
            std::string candidate = dir + "/" + binary;
            if (access(candidate.c_str(), X_OK) == 0) {
                return candidate;
            }
        }
        return "";
    }

}

void appendRootfsArgs(std::vector<std::string>& out) {
    out.emplace_back("--ro-bind");
    out.emplace_back("/usr");
    out.emplace_back("/usr");

    out.emplace_back("--ro-bind");
    out.emplace_back("/etc");
    out.emplace_back("/etc");

    out.emplace_back("--symlink");
    out.emplace_back("usr/lib");
    out.emplace_back("/lib");

    out.emplace_back("--symlink");
    out.emplace_back("usr/lib64");
    out.emplace_back("/lib64");

    out.emplace_back("--symlink");
    out.emplace_back("usr/bin");
    out.emplace_back("/bin");

    out.emplace_back("--symlink");
    out.emplace_back("usr/bin");
    out.emplace_back("/sbin");

    out.emplace_back("--proc");
    out.emplace_back("/proc");

    out.emplace_back("--dev");
    out.emplace_back("/dev");

    out.emplace_back("--tmpfs");
    out.emplace_back("/tmp");
}
sandbox::sandbox(sandboxConfig config) : config(std::move(config)) {
    if (this->config.agentId.empty()) {
        throw std::invalid_argument("sandboxConfig::agentId must not be empty");
    }
    if (this->config.workspacePath.empty()) {
        throw std::invalid_argument("sandboxConfig::workspacePath must not be empty");
    }

    const std::string resolved = resolveExecutablePath(config.bwrapBinary);
    if (resolved.empty()) {
        throw std::runtime_error(
            "sandbox[" + config.agentId +
            "]: bwrap not found or not executable ('" + config.bwrapBinary +
            "') — install it with: sudo apt install bubblewrap");
    }
    config.bwrapBinary = resolved;

    ensureDirectories();
}

void sandbox::ensureDirectories() const {
    std::error_code ec;
    std::filesystem::create_directories(config.workspacePath, ec);
    if (ec) {
        throw std::runtime_error("sandbox: cannot create directory: "
            + ec.message() +
            " " + config.workspacePath.string());
    }
    for (const auto &[name, hostPath] : config.sharedPaths) {
        std::filesystem::create_directories(hostPath, ec);
        if (ec) {
            throw std::runtime_error("sandbox: cannot create shared dir " +
                                     hostPath.string() + ": " + ec.message());
        }
    }
}

std::vector<std::string> sandbox::buildBwrapArgs(
    const std::string &executable,
    const std::vector<std::string> &args) const {

    std::vector<std::string> bwrapArgs;
    bwrapArgs.reserve(16 + config.sharedPaths.size() * 3 +
                      config.environment.size() * 3 + args.size());

    appendRootfsArgs(bwrapArgs);

    bwrapArgs.emplace_back("--unshare-pid");

    if (!config.allowNetwork) {
        bwrapArgs.emplace_back("--unshare-net");
    }

    bwrapArgs.emplace_back("--clearenv");
    for (const auto &[key, value] : config.environment) {
        bwrapArgs.emplace_back("--setenv");
        bwrapArgs.emplace_back(key);
        bwrapArgs.emplace_back(value);
    }

    bwrapArgs.emplace_back("--bind");
    bwrapArgs.emplace_back(config.workspacePath.string());
    bwrapArgs.emplace_back("/workspace");

    for (const auto &[name, hostPath] : config.sharedPaths) {
        bwrapArgs.emplace_back("--bind");
        bwrapArgs.emplace_back(hostPath.string());
        bwrapArgs.emplace_back("/shared/" + name);
    }

    bwrapArgs.emplace_back("--");
    bwrapArgs.emplace_back(executable);
    for (const auto &arg : args) {
        bwrapArgs.emplace_back(arg);
    }

    return bwrapArgs;
}



processResult sandbox::runInSandbox(const std::string &executable,
                                    const std::vector<std::string> &args) {
    const auto bwrapArgs = buildBwrapArgs(executable, args);

    std::string joined;
    for (const auto &a : bwrapArgs) {
        joined += a + " ";
    }
    spdlog::info("sandbox cmd: " + joined);

    spdlog::info("sandbox[" + config.agentId + "]: launching "+ executable + " in bubble");

    process proc;
    processResult result = proc.runProcess(config.bwrapBinary, bwrapArgs);

    if (result.exitCode == 127) {
        spdlog::error("sandbox[" + config.agentId + "]: executable '" + executable +
                      "' or bwrap not found (exit 127)");
    } else {
        spdlog::info("sandbox[" + config.agentId + "]: finished with exit code " +
                     std::to_string(result.exitCode));
    }
    return result;
}