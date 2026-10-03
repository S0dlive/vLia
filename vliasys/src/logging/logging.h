//
// Created by baptisteluciani on 03/10/2026.
//

#ifndef VLIASYS_LOGGING_H
#define VLIASYS_LOGGING_H
#include "spdlog/logger.h"


struct logging {
    static void initLogging();
    static void activeDebugLogging();
    static void activeInfoLogging();
};


#endif //VLIASYS_LOGGING_H
