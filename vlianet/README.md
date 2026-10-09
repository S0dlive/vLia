# vlianet

`vlianet` is the Rust P2P networking daemon for the **vLia** ecosystem. Powered by `libp2p`, it handles node discovery, distributed consensus, topic-based message broadcasting (Gossipsub), and bridges remote task proposals to the local C++ runtime (`vliasys`) via UNIX socket IPC.

## Key Features

- **Libp2p Swarm**: Built on Tokio, TCP, Noise encryption, and Yamux multiplexing.
- **Node Identity**: Cryptographic keypair generation and persistence (`node_identity.bytes`).
- **Distributed Consensus**: Gossipsub messaging for task proposals, commits, and reveals on the `vlianet-consensus-v1` topic.
- **Kademlia DHT**: Automatic peer discovery and routing table maintenance.
- **IPC Bridge**: Communicates with local `vliasys` via `/tmp/vliasys.sock` to execute tasks and handle consensus verification.

## Configuration & Environment

`vlianet` is configured via environment variables, typically loaded from `/etc/vlia/vlianet.env`:

```env
PORT=4001
IDENTITY_FILE=/var/lib/vlianet/node_identity.bytes
# OPTIONAL: BOOTSTRAP_PEER=/ip4/x.x.x.x/tcp/4001
```
## CLI Flags
```text
Usage: vlianet [OPTIONS]

Options:
  -p, --port <PORT>                   Network port to listen on [default: 4001]
  -i, --identity-file <IDENTITY_FILE> Path to node keypair binary [default: ./node_identity.bytes]
  -c, --peer <PEER>                   Optional bootstrap Multiaddr peer to dial
  -h, --help                          Print help
  -V, --version                       Print version
```
## Requirements
-Linux OS
-Rust toolchain (Edition 2021 / Cargo)
-Running vliasys daemon listening on /tmp/vliasys.sock
-Build & Standalone Run

To compile and run vlianet manually:
```
cargo build --release
./target/release/vlianet -p 4001 -i /var/lib/vlianet/node_identity.bytes
```
## Systemd Integration
vlianet runs as a system daemon managed alongside vliasys:
```
vlia status           # View systemd status for both daemons
vlia logs             # Stream real-time logs for vliasys and vlianet
vlia restart          # Restart both services
```
