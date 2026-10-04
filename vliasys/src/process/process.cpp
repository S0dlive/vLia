//
// Created by baptisteluciani on 03/10/2026.
//

#include "process.h"

#include <stdexcept>
#include <unistd.h>
#include <sys/wait.h>

#include "spdlog/spdlog.h"

static std::string readFromFd(int fd) {
    std::string output;
    std::array<char, 256> buffer;
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer.data(), buffer.size())) > 0) {
        output.append(buffer.data(), bytesRead);
    }
    close(fd);
    return output;
} // C'est un helpeur que j'ai cherché (x

process::process() {

}

process::~process() {

}

processResult process::runProcess(const std::string &executable, const std::pmr::vector<std::string> &args) {
    int stdoutpipe[2];
    int stderrpipe[2];
    if (pipe(stdoutpipe) == -1 || pipe(stderrpipe) == -1) {
        spdlog::error("An error occurred while creating the pipe for the process " + executable);
        close(stdoutpipe[0]); close (stdoutpipe[1]);
        close(stderrpipe[1]); close (stderrpipe[0]);
        return processResult{
            -1,
            "",
            "error with pipe"
        };
    }

    pid_t pid = fork();
    if (pid < 0) {
        spdlog::error("An error occurred while forking the process " + executable);
        close(stdoutpipe[0]); close (stdoutpipe[1]);
        close(stderrpipe[1]); close (stderrpipe[0]);
        return processResult{
            -1,
            "error with fork",
            "error with fork"
        };
    }

    if (pid == 0) {
        dup2( stdoutpipe[1], STDOUT_FILENO );
        dup2( stderrpipe[1], STDERR_FILENO );
        close(stdoutpipe[0]); close (stdoutpipe[1]);
        close(stderrpipe[1]); close (stderrpipe[0]);

        std::vector<char*> execArgs;
        execArgs.push_back(const_cast<char*>(executable.c_str()));

        for (const auto &arg : args) {
            execArgs.push_back(const_cast<char*>(arg.c_str()));
        }

        execArgs.push_back(nullptr);
        execvp(executable.c_str(), execArgs.data());

        _exit(127);
    } else {
        close (stdoutpipe[1]);
        close(stderrpipe[1]);

        std::string stdoutStr = readFromFd(stdoutpipe[0]);
        std::string stderrStr = readFromFd(stderrpipe[0]);

        int status = 0;
        waitpid(pid, &status, 0);

        int exitCode = -1;
        if (WIFEXITED(status)) {
            exitCode = WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status)) {
            exitCode = 128 + WTERMSIG(status);
        }
        return processResult{
            exitCode,
            stdoutStr,
            stderrStr,
        };
    }

    return processResult{
        -1,
        "",
        "unknown error",
    };
}


