#include <iostream>

#include "logging/logging.h"
#include "process/process.h"
#include "spdlog/spdlog.h"


int main() {
    logging::initLogging();

    spdlog::info("test");
    process pro;
    auto t = pro.runProcess("ls", {"-l"});

    spdlog::info("result : " + t.stdout_output);
    return 0;
}
