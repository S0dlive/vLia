# vLia

vLia is an open-source framework designed to run, isolate, and interconnect autonomous AI agents via P2P. The project focuses on security, modularity, and lightweight background system execution on Linux.

Status: Active Alpha. Core runtime, P2P network daemon, systemd integration, and installer operational.

## Architecture Overview

The system runs via background systemd services and a centralized management CLI:

- **vliasys**: C++20 core runtime managing agent lifecycles, Bubblewrap (bwrap) sandboxing, local Ollama LLM execution, MCP tool integration, and UNIX socket IPC.
- **vlianet**: Rust daemon powered by `libp2p` handling P2P discovery (Kademlia DHT), Gossipsub messaging, and consensus across nodes.
- **vlia**: System CLI to deploy, monitor, and manage configurations and services in real-time.

```text
/etc/vlia/
├── vliasys.json         # Ollama model & IPC socket settings
└── vlianet.env          # Port & identity configuration
``` 

## Features & Roadmap
### Core Runtime (vliasys)
[x] POSIX process management & Bubblewrap (bwrap) sandboxing

[x] MCP server provisioning & JSON-RPC STDIO client integration

[x] Ollama ReAct execution loop via libcurl

[x] UNIX socket IPC API server

[x] Auto-generated agent identity and dynamic configuration persistence

[ ] Add persistent memory system (local vector database / RAG)

### Networking Layer (vlianet)
[x] P2P node identity & libp2p Swarm setup

[x] Gossipsub consensus topic subscription

[x] IPC bridge to vliasys for remote job execution

[ ] Implement node reputation and trust-scoring mechanism

[ ] Advanced distributed consensus rules

### System & CLI (vlia)
[x] Automated systemd unit integration

[x] Single-command interactive installer (install.sh)

[x] Integrated control CLI (vlia status, vlia restart, vlia logs)

## Prerequisites
- Linux systemd distribution (Fedora, Debian, Ubuntu, Arch)
- C++20 compiler (gcc >= 10 or clang >= 12) & CMake >= 3.20
- Rust toolchain (cargo / rustup)
- Bubblewrap (bwrap), libcurl, spdlog
- Ollama running locally (http://127.0.0.1:11434)

## Installation & Quickstart
Clone the repository and run the automated installer:
```bash
git clone [https://github.com/S0dlive/vLia.git](https://github.com/S0dlive/vLia.git)
cd vLia
chmod +x install.sh
sudo ./install.sh
```
The installer automatically detects local Ollama models, compiles both vliasys and vlianet, creates system configuration files in /etc/vlia/, and registers systemd daemons.

## CLI Usage
Manage background services using the unified vlia command:
```bash
vlia status                # Check status of vliasys and vlianet
vlia logs                  # Stream combined logs in real-time
vlia restart               # Restart both daemons
vlia off-vliasys           # Stop only the local runtime
vlia config                # Edit runtime JSON configuration
```
