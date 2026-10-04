cmake_minimum_required(VERSION 4.3)
project(vliasys)

set(CMAKE_CXX_STANDARD 20)



add_executable(vliasys
        src/main.cpp
        src/logging/logging.cpp
        src/logging/logging.h
        src/process/process.cpp
        src/process/process.h
        src/process/processresult.h)

include(FetchContent)
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.15.0
)
FetchContent_MakeAvailable(spdlog)

target_link_libraries(vliasys PUBLIC spdlog::spdlog)