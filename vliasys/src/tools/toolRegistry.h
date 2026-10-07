//
// Created by baptisteluciani on 07/10/2026.
//

#ifndef VLIASYS_TOOLREGISTRY_H
#define VLIASYS_TOOLREGISTRY_H
#include <memory>
#include <unordered_map>
#include "tool.h"

class toolRegistry {
public:
    void registerTool(std::shared_ptr<tool> t) {
        m_tools[t->getName()] = t;
    }

    json getToolsSchema() const {
        json schemas = json::array();
        for (const auto& [name, t] : m_tools) {
            schemas.push_back({
                {"type", "function"},
                {"function", {
                    {"name", t->getName()},
                    {"description", t->getDescription()},
                    {"parameters", t->getParametersSchema()}
                }}
            });
        }
        return schemas;
    }

    toolResult callTool(const std::string& name, const json& args) {
        auto it = m_tools.find(name);
        if (it == m_tools.end()) {
            return {false, "", "Tool not found: " + name};
        }
        return it->second->execute(args);
    }

private:
    std::unordered_map<std::string, std::shared_ptr<tool>> m_tools;
};


#endif //VLIASYS_TOOLREGISTRY_H
