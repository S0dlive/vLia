//
// Created by baptisteluciani on 07/10/2026.
//

#include "interactiveprocess.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

void interactiveProcess::writeLine(const std::string& line) {
    std::string data = line + "\n";
    ::write(stdinFd, data.c_str(), data.size());
}

std::string interactiveProcess::readLine() {
    std::string line;
    char c;
    while (::read(stdoutFd, &c, 1) > 0) {
        if (c == '\n') break;
        line += c;
    }
    return line;
}

void interactiveProcess::close() {
    if (stdinFd != -1) ::close(stdinFd);
    if (stdoutFd != -1) ::close(stdoutFd);
    if (pid > 0) {
        int status;
        ::waitpid(pid, &status, WNOHANG);
    }
}

