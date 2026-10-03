//
// Created by baptisteluciani on 03/10/2026.
//

#include "logging.h"
#include <spdlog/spdlog.h>
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"


void logging::initLogging() {
    auto max_size = 1048576 * 5;
    auto max_file = 5;

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/rotating.txt",max_size, max_file);

    std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink };
    auto logger = std::make_shared<spdlog::logger>("vLia", sinks.begin(), sinks.end());

    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
    spdlog::flush_on(spdlog::level::info);
}

logging::~logging() {
    spdlog::shutdown();
}

logging::logging() {

}

void logging::activeDebugLogging() {
    spdlog::set_level(spdlog::level::debug);
}

void logging::activeInfoLogging() {
    spdlog::set_level(spdlog::level::info);
}
