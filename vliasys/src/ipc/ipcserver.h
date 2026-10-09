//
// Created by baptisteluciani on 08/10/2026.
//

#ifndef VLIASYS_IPCSERVER_H
#define VLIASYS_IPCSERVER_H
#include <string>
#include <atomic>
#include <thread>
#include "../runtime/agentruntime.h"

class ipcServer {
public:
    ipcServer(agentRuntime& runtime, std::string socketPath = "/tmp/vliasys.sock");
    ~ipcServer();

    void start();
    void stop();

private:
    void run();
    void handleClient(int clientFd);

    agentRuntime& m_runtime;
    std::string m_socketPath;
    std::atomic<bool> m_running{false};
    int m_serverFd{-1};
    std::thread m_workerThread;
};

#endif //VLIASYS_IPCSERVER_H
