// mcpTool.h
#ifndef VLIASYS_MCPTOOL_H
#define VLIASYS_MCPTOOL_H

#include "tool.h"
#include "../tools/mcpclient.h"

class mcpTool : public tool {
public:
    mcpTool(std::shared_ptr<mcpClient> client, std::string name, std::string description, json schema)
        : m_client(std::move(client)), m_name(std::move(name)), m_description(std::move(description)), m_schema(std::move(schema)) {}

    std::string getName() const override { return m_name; }
    std::string getDescription() const override { return m_description; }
    json getParametersSchema() const override { return m_schema; }

    toolResult execute(const json& args) override {
        try {
            json res = m_client->callTool(m_name, args);
            std::string textOutput;
            if (res.contains("content") && res["content"].is_array()) {
                for (const auto& item : res["content"]) {
                    if (item.value("type", "") == "text") {
                        textOutput += item.value("text", "") + "\n";
                    }
                }
            }
            bool isError = res.value("isError", false);
            return {!isError, textOutput, isError ? textOutput : ""};
        } catch (const std::exception& e) {
            return {false, "", e.what()};
        }
    }

private:
    std::shared_ptr<mcpClient> m_client;
    std::string m_name;
    std::string m_description;
    json m_schema;
};

#endif // VLIASYS_MCPTOOL_H