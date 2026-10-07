//
// Created by baptisteluciani on 03/10/2026.
//

#ifndef VLIASYS_PROCESS_H
#define VLIASYS_PROCESS_H
#include <string>
#include <vector>
#include <sys/types.h>
#include "processresult.h"


class process {
public:
    process();
    ~process();
    pid_t getPid();
    processResult runProcess(const std::string& executable, const std::vector<std::string>& args);
private:
    pid_t p_pid;
};


#endif //VLIASYS_PROCESS_H
