//
// Created by baptisteluciani on 03/10/2026.
//

#ifndef VLIASYS_LOGGING_H
#define VLIASYS_LOGGING_H
#include "spdlog/logger.h"


class logging {
public:
    logging();
    ~logging();
    void initLogging();
    void activeDebugLogging();
    void activeInfoLogging();
};


#endif //VLIASYS_LOGGING_H
