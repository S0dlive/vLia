//
// Created by baptisteluciani on 07/10/2026.
//

#ifndef VLIASYS_BASHTOOL_H
#define VLIASYS_BASHTOOL_H
#include "tool.h"
#include "../sandbox/sandbox.h"
#include "toolResult.h"

class bashTool : public tool {
public:
    explicit bashTool(sandbox& box) : m_sandbox(box) {}

    std::string getName() const override { return "execute_bash"; }

    std::string getDescription() const override {
        return "Execute a bash command safely inside the agent's isolated workspace.";
    }

    json getParametersSchema() const override {
        return {
            {"type", "object"},
            {"properties", {
                    {"command", {
                        {"type", "string"},
                        {"description", "The bash command to run in /workspace"}
                    }}
            }},
            {"required", {"command"}}
        };
    }

    toolResult execute(const json& args) override {
        if (!args.contains("command") || !args["command"].is_string()) {
            return {false, "", "Missing or invalid 'command' argument"};
        }

        std::string cmd = args["command"].get<std::string>();
        auto result = m_sandbox.runInSandbox("/usr/bin/sh", {"-c", cmd});

        bool ok = (result.exitCode == 0);
        return {
            ok,
            result.stdout_output,
            result.stderr_output
        };
    }

private:
    sandbox& m_sandbox;
};


#endif //VLIASYS_BASHTOOL_H
