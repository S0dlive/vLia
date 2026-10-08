# vLia
vLia is an open-source framework designed to run, isolate, and interconnect autonomous AI agents. The project focuses on security, modularity, and lightweight local execution.

⚠️ Status: Work in Progress (Alpha). Core runtime under active development.

## 🏗️ Architecture Overview
The project is structured around three main components:

vliasys: The core C++20 runtime handling agent lifecycles, Bubblewrap (bwrap) sandboxing, local Ollama LLM execution, and Model Context Protocol (MCP) tool integration.

vlianet: The upcoming networking layer designed for P2P agent collaboration, consensus, and reputation management.

vLia Client: The CLI/Desktop user interface to control agents and interact with local or remote runtime nodes.

## 📋 Roadmap & Todo List
### Core Runtime (vliasys)
[x] POSIX process management & Bubblewrap (bwrap) sandboxing

[x] MCP server provisioning & JSON-RPC STDIO client integration

[x] Ollama ReAct execution loop via libcurl

[ ] Fix edge-case JSON-RPC STDIO buffer parsing & stderr leaks

[ ] Add persistent memory system (local vector database / RAG)

[ ] Expose an IPC API (gRPC / UNIX Sockets) for client communication

### Networking Layer (vlianet)
[ ] Design P2P protocol for agent-to-agent task delegation

[ ] Implement node reputation and trust-scoring mechanism

[ ] Define consensus rules for distributed agent responses

### Client & Interface
[ ] Build the lightweight CLI client

[ ] Implement real-time log streaming for sandboxed execution

[ ] Add session management and multi-node connection handling

## 🚀 Quickstart
Prerequisites
- C++20 compiler (gcc or clang)

- CMake >= 3.20

- bubblewrap

- libcurl

- Ollama running locally ([http://127.0.0.1:11434](http://127.0.0.1:11434))

Build
```sh
mkdir build && cd build
cmake ..
make
./vliasys
``` 
