#include <iostream>

#include "logging/logging.h"
#include "process/process.h"
#include "spdlog/spdlog.h"
#include "sandbox/sandbox.h"


int main() {
    logging::initLogging();

    const auto projectRoot = std::filesystem::current_path();
    sandboxConfig cfg;
    cfg.agentId = "agent-alpha";
    cfg.workspacePath = projectRoot / "workspaces" / "agent-alpha";
    cfg.sharedPaths.emplace_back("projects", projectRoot / "workspaces" / "shared" / "projects");

    sandbox box(cfg);

    auto r1 = box.runInSandbox("/usr/bin/sh", {"-c", "echo 'hello from sandbox' > /workspace/out.txt && ls -la /workspace && cat /shared/projects/README.md"});
    spdlog::info("exit: " + std::to_string(r1.exitCode) + "\nstdout:\n" + r1.stdout_output);

    auto r2 = box.runInSandbox("/usr/bin/cat", {"/workspace/out.txt"});
    spdlog::info("content: " + r2.stdout_output);

    return 0;
}