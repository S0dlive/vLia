#include <iostream>

#include "logging/logging.h"
#include "spdlog/spdlog.h"


int main() {
    logging log;
    log.initLogging();

    return 0;
}
