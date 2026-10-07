//
// Created by baptisteluciani on 07/10/2026.
//

#ifndef VLIASYS_TOOL_H
#define VLIASYS_TOOL_H

#include <string>
#include <nlohmann/json.hpp>
#include "toolResult.h"

using json = nlohmann::json;

class tool {
public:
    virtual ~tool() = default;

    [[nodiscard]] virtual std::string getName() const = 0;
    [[nodiscard]] virtual std::string getDescription() const = 0;
    [[nodiscard]] virtual json getParametersSchema() const = 0;

    virtual toolResult execute(const json& args) = 0;
};


#endif //VLIASYS_TOOL_H
