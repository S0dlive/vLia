cmake_minimum_required(VERSION 4.3)
project(vliasys)

set(CMAKE_CXX_STANDARD 20)



add_executable(vliasys
        src/main.cpp
        src/logging/logging.cpp
        src/logging/logging.h
        src/process/process.cpp
        src/process/process.h
        src/process/processresult.h
        src/sandbox/sandbox.cpp
        src/sandbox/sandbox.h
        src/sandbox/sandboxconfig.h
        src/config/configmanager.cpp
        src/config/configmanager.h
        src/config/globalconfig.h)

include(FetchContent)
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.15.0
)
FetchContent_MakeAvailable(spdlog)

target_link_libraries(vliasys PUBLIC spdlog::spdlog)