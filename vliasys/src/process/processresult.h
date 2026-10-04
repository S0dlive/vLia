//
// Created by baptisteluciani on 04/10/2026.
//

#ifndef VLIASYS_PROCESSRESULT_H
#define VLIASYS_PROCESSRESULT_H
#include <string>

struct processResult {
    int exitCode = -1;
    std::string stdout_output;
    std::string stderr_output;
};

#endif //VLIASYS_PROCESSRESULT_H
