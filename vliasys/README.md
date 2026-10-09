# vliasys

`vliasys` is the core C++20 runtime daemon for the **vLia** ecosystem. It manages agent lifecycles, sandboxed execution via Bubblewrap (`bwrap`), local LLM inference through Ollama, and tool integration using the Model Context Protocol (MCP). It exposes a local UNIX socket IPC interface for client control.

## Key Features

- **Sandboxed Execution**: Runs tasks in isolated Bubblewrap (`bwrap`) containers.
- **Local LLM Loop**: ReAct execution loop interacting directly with local Ollama instances.
- **MCP Tool Integration**: Standardized JSON-RPC STDIO client for external MCP tool provisioning.
- **UNIX Socket IPC**: Listens on `/tmp/vliasys.sock` for local client requests and P2P daemon bridges.
- **Dynamic Configuration**: Automatically generates and persists node identity (`agent_id`) in `/etc/vlia/vliasys.json`.

## Architecture & Configuration

`vliasys` reads its configuration on startup from `/etc/vlia/vliasys.json`:

```json
{
    "model": "qwen2.5-coder:latest",
    "agent_id": "agent-ec7e-5b96",
    "socket_path": "/tmp/vliasys.sock"
}
```
If agent_id is missing, vliasys automatically generates a new UUID and persists it back to the configuration file.

## Requirements
-Linux (systemd-based distribution recommended)
-C++20 compliant compiler (GCC >= 10, Clang >= 12)
-CMake >= 3.20
-Dependencies: bubblewrap, libcurl, spdlog, nlohmann_json
-Local Ollama daemon running on http://127.0.0.1:11434

## Build & Standalone Run
To compile and run vliasys manually without the main installer:

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
./vliasys
```
## Systemd Integration
vliasys is designed to run as a system daemon managed via systemctl or the unified vlia CLI:
```bash
vlia status           # Check service status
vlia off-vliasys      # Stop runtime
vlia on-vliasys       # Start runtime
```
