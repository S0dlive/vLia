cmake_minimum_required(VERSION 3.20)
project(vliasys)

set(CMAKE_CXX_STANDARD 20)

find_package(CURL REQUIRED)

include_directories(${CURL_INCLUDE_DIRS})

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
        src/config/globalconfig.h
        src/tools/tool.cpp
        src/tools/tool.h
        src/tools/toolResult.h
        src/tools/bashtool.h
        src/tools/mcpclient.cpp
        src/tools/mcpclient.h
        src/tools/toolRegistry.h
        src/process/interactiveprocess.cpp
        src/process/interactiveprocess.h
        src/tools/mcpserverspec.h
        src/tools/mcpprovisioner.h
        src/runtime/ollamaclient.h
        src/runtime/agentruntime.h
        src/tools/mcptool.h
)

include(FetchContent)
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.15.0
)
FetchContent_MakeAvailable(spdlog)

target_link_libraries(vliasys PUBLIC
        spdlog::spdlog
        ${CURL_LIBRARIES}
)