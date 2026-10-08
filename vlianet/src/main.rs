use anyhow::Result;
use clap::Parser;
use identity::NodeIdentity;
use ipc::client::VliaSysIpcClient;
use libp2p::futures::StreamExt;
use libp2p::gossipsub;
use libp2p::swarm::SwarmEvent;
use libp2p::{tcp, yamux, Multiaddr, SwarmBuilder};
use p2p::behaviour::VliaBehaviour;
use sha2::{Digest, Sha256};
use std::path::PathBuf;
use tracing::{error, info, Level};
use tracing_subscriber::FmtSubscriber;

use consensus::engine::ConsensusEngine;
use consensus::types::ConsensusMessage;

mod consensus;
mod identity;
mod ipc;
mod p2p;

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    #[arg(short = 'p', long, default_value_t = 4001)]
    port: u16,
    
    #[arg(short = 'i', long, default_value = "./node_identity.bytes")]
    identity_file: PathBuf,

    #[arg(short = 'c', long)]
    peer: Option<Multiaddr>,
}

#[tokio::main]
async fn main() -> Result<()> {
    let args = Args::parse();

    let subscriber = FmtSubscriber::builder()
        .with_max_level(Level::INFO)
        .finish();
    tracing::subscriber::set_global_default(subscriber)?;

    info!("Starting vlianet P2P node daemon...");

    let identity = NodeIdentity::load_or_generate(&args.identity_file)?;
    let local_peer_id = identity.peer_id();
    info!("Local PeerID: {}", local_peer_id);
    
    let mut swarm = SwarmBuilder::with_existing_identity(identity.keypair.clone())
        .with_tokio()
        .with_tcp(
            tcp::Config::default(),
            libp2p::noise::Config::new,
            yamux::Config::default,
        )?
        .with_behaviour(|key| VliaBehaviour::new(key).unwrap())?
        .with_swarm_config(|c| c.with_idle_connection_timeout(std::time::Duration::from_secs(60)))
        .build();

    let topic = gossipsub::IdentTopic::new("vlianet-consensus-v1");
    swarm.behaviour_mut().gossipsub.subscribe(&topic)?;
    info!("Subscribed to consensus topic 'vlianet-consensus-v1'");

    let mut consensus_engine = ConsensusEngine::new();
    let ipc_client = VliaSysIpcClient::new("/tmp/vliasys.sock");

    let listen_addr: Multiaddr = format!("/ip4/0.0.0.0/tcp/{}", args.port).parse()?;
    swarm.listen_on(listen_addr.clone())?;
    info!("vlianet listening on {}", listen_addr);
    
    if let Some(peer_addr) = args.peer {
        info!("Dialing bootstrap peer at {}...", peer_addr);
        swarm.dial(peer_addr)?;
    }

    loop {
        tokio::select! {
            _ = tokio::signal::ctrl_c() => {
                info!("Shutdown signal received, stopping vlianet...");
                break;
            }
            
            event = swarm.select_next_some() => match event {
                SwarmEvent::NewListenAddr { address, .. } => {
                    info!("Node listening on: {}", address);
                }
                SwarmEvent::ConnectionEstablished { peer_id, .. } => {
                    info!("Peer connected: {}", peer_id);
                    let maybe_addr = swarm.listeners().next().cloned();
                    if let Some(listen_addr) = maybe_addr {
                        swarm.behaviour_mut().kademlia.add_address(&peer_id, listen_addr);
                    }
                }
                SwarmEvent::Behaviour(p2p::behaviour::VliaBehaviourEvent::Gossipsub(gossipsub::Event::Message { message, .. })) => {
                    if let Ok(consensus_msg) = serde_json::from_slice::<ConsensusMessage>(&message.data) {
                        info!("Received message: {:?}", consensus_msg);

                        match &consensus_msg {
                            ConsensusMessage::JobProposal { job_id, prompt, .. } => {
                                info!("Processing JobProposal '{}' via vliasys IPC...", job_id);
                                match ipc_client.execute_task(job_id, prompt).await {
                                    Ok(response) => {
                                        info!("vliasys output received: {}", response.output);

                                        let nonce = format!("{:x}", rand::random::<u64>());
                                        let mut hasher = Sha256::new();
                                        hasher.update(format!("{}{}", response.output, nonce).as_bytes());
                                        let commit_hash = hex::encode(hasher.finalize());

                                        let commit_msg = ConsensusMessage::JobCommit {
                                            job_id: job_id.clone(),
                                            worker_peer_id: local_peer_id.to_string(),
                                            commit_hash,
                                        };
                                        if let Err(e) = swarm.behaviour_mut().gossipsub.publish(topic.clone(), serde_json::to_vec(&commit_msg)?) {
                                            error!("Failed to publish Commit: {:?}", e);
                                        }

                                        let reveal_msg = ConsensusMessage::JobReveal {
                                            job_id: job_id.clone(),
                                            worker_peer_id: local_peer_id.to_string(),
                                            raw_result: response.output,
                                            nonce,
                                        };
                                        if let Err(e) = swarm.behaviour_mut().gossipsub.publish(topic.clone(), serde_json::to_vec(&reveal_msg)?) {
                                            error!("Failed to publish Reveal: {:?}", e);
                                        }
                                    }
                                    Err(e) => {
                                        error!("IPC execution failed: {:?}", e);
                                    }
                                }
                            }
                            _ => {}
                        }
                        consensus_engine.handle_message(consensus_msg);
                    }
                }
                _ => {}
            }
        }
    }

    info!("vlianet daemon exited cleanly.");
    Ok(())
}