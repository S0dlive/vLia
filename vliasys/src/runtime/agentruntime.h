// agentRuntime.h
#ifndef VLIASYS_AGENTRUNTIME_H
#define VLIASYS_AGENTRUNTIME_H

#include <memory>
#include <spdlog/spdlog.h>
#include "../config/configmanager.h"
#include "../sandbox/sandbox.h"
#include "../tools/toolRegistry.h"
#include "../tools/bashtool.h"
#include "../tools/mcptool.h"
#include "../tools/mcpclient.h"
#include "../tools/mcpprovisioner.h"
#include "ollamaclient.h"

class agentRuntime {
public:
    agentRuntime(configManager& cfgMgr, std::string agentId)
        : m_cfgMgr(cfgMgr), m_agentId(std::move(agentId)) {}

    void setup() {
        auto globalCfg = m_cfgMgr.getGlobalConfig();
        m_sandboxCfg = m_cfgMgr.createSandboxConfigForAgent(m_agentId);

        m_sandbox = std::make_unique<sandbox>(m_sandboxCfg);

        m_registry.registerTool(std::make_shared<bashTool>(*m_sandbox));

        for (const auto& mcpSpec : globalCfg.mcpServers) {
            spdlog::info("Provisioning MCP server: " + mcpSpec.name);
            if (mcpProvisioner::ensureServerInstalled(m_sandboxCfg, mcpSpec)) {
                auto client = std::make_shared<mcpClient>(*m_sandbox, mcpSpec.executable, mcpSpec.args);
                if (client->initialize()) {
                    m_mcpClients.push_back(client);
                    json toolsList = client->listTools();

                    for (const auto& t : toolsList) {
                        std::string name = t.value("name", "");
                        std::string desc = t.value("description", "");
                        json schema = t.value("inputSchema", json::object());

                        m_registry.registerTool(std::make_shared<mcpTool>(client, name, desc, schema));
                        spdlog::info("Registered MCP Tool '" + name +"' from server " + mcpSpec.name);
                    }
                } else {
                    spdlog::error("Failed to initialize MCP client for " + mcpSpec.name);
                }
            }
        }

        m_ollama = std::make_unique<ollamaClient>(globalCfg.ollamaEndpoint, globalCfg.defaultModel);
    }

    std::string runUserQuery(const std::string& userPrompt, int maxTurns = 10) {
    json messages = json::array();
    messages.push_back({
        {"role", "system"},
        {"content", "You are an autonomous agent operating safely inside a sandboxed environment. Use tools when needed."}
    });
    messages.push_back({
        {"role", "user"},
        {"content", userPrompt}
    });

    for (int turn = 0; turn < maxTurns; ++turn) {
        spdlog::info("Agent turn " +  std::to_string(turn + 1) + "/" + std::to_string(maxTurns));
        json response = m_ollama->chat(messages, m_registry.getToolsSchema());

        json msg = response["message"];
        messages.push_back(msg);

        json toolCalls = json::array();

        if (msg.contains("tool_calls") && !msg["tool_calls"].empty()) {
            toolCalls = msg["tool_calls"];
        }
        else if (msg.contains("content") && msg["content"].is_string()) {
            std::string content = msg["content"].get<std::string>();

            size_t startPos = content.find('{');
            size_t endPos = content.rfind('}');

            if (startPos != std::string::npos && endPos != std::string::npos && endPos > startPos) {
                std::string jsonCandidate = content.substr(startPos, endPos - startPos + 1);
                try {
                    json parsed = json::parse(jsonCandidate);
                    if (parsed.contains("name") && parsed.contains("arguments")) {
                        toolCalls.push_back({
                            {"function", {
                                {"name", parsed["name"]},
                                {"arguments", parsed["arguments"]}
                            }}
                        });
                    }
                } catch (...) {
                }
            }
        }

        if (!toolCalls.empty()) {
            for (const auto& call : toolCalls) {
                std::string toolName = call["function"]["name"];
                json toolArgs = call["function"]["arguments"];

                spdlog::info("Executing tool call: {" + toolName + "} with args " + toolArgs.dump());
                toolResult res = m_registry.callTool(toolName, toolArgs);

                messages.push_back({
                    {"role", "tool"},
                    {"content", res.success ? res.output : ("Error: " + res.error)}
                });
            }
        } else {
            return msg.value("content", "");
        }
    }

    return "Max execution turns reached without final response.";
}

private:
    configManager& m_cfgMgr;
    std::string m_agentId;
    sandboxConfig m_sandboxCfg;
    std::unique_ptr<sandbox> m_sandbox;
    toolRegistry m_registry;
    std::vector<std::shared_ptr<mcpClient>> m_mcpClients;
    std::unique_ptr<ollamaClient> m_ollama;
};

#endif // VLIASYS_AGENTRUNTIME_H