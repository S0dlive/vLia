//
// Created by baptisteluciani on 07/10/2026.
//

#ifndef VLIASYS_INTERACTIVEPROCESS_H
#define VLIASYS_INTERACTIVEPROCESS_H
#include <sys/types.h>
#include <string>

struct  interactiveProcess {
    pid_t pid{-1};
    int stdinFd{-1};
    int stdoutFd{-1};

    void writeLine(const std::string& line);
    std::string readLine();
    void close();
};


#endif //VLIASYS_INTERACTIVEPROCESS_H
