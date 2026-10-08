use libp2p::gossipsub::{self, MessageAuthenticity, ValidationMode};
use libp2p::kad::{self, store::MemoryStore};
use libp2p::swarm::NetworkBehaviour;
use libp2p::{identity::Keypair, ping, PeerId};
use std::hash::{Hash, Hasher};
use std::time::Duration;
use libp2p::autonat;

#[derive(NetworkBehaviour)]
pub struct VliaBehaviour {
    pub ping: ping::Behaviour,
    pub kademlia: kad::Behaviour<MemoryStore>,
    pub gossipsub: gossipsub::Behaviour,
    pub autonat: autonat::Behaviour,
}

impl VliaBehaviour {
    pub fn new(keypair: &Keypair) -> anyhow::Result<Self> {
        let peer_id = keypair.public().to_peer_id();

        let ping = ping::Behaviour::new(ping::Config::default());

        let store = MemoryStore::new(peer_id);
        let mut kad_config = kad::Config::default();
        kad_config.set_query_timeout(Duration::from_secs(10));
        let kademlia = kad::Behaviour::with_config(peer_id, store, kad_config);

        let message_id_fn = |message: &gossipsub::Message| {
            let mut s = std::collections::hash_map::DefaultHasher::new();
            message.data.hash(&mut s);
            gossipsub::MessageId::from(s.finish().to_string())
        };

        let gossipsub_config = gossipsub::ConfigBuilder::default()
            .heartbeat_interval(Duration::from_secs(1))
            .validation_mode(ValidationMode::Strict)
            .allow_self_origin(true) // Permet de recevoir ses propres messages
            .build()
            .map_err(|e| anyhow::anyhow!("Gossipsub config error: {}", e))?;
        let gossipsub = gossipsub::Behaviour::new(
            MessageAuthenticity::Signed(keypair.clone()),
            gossipsub_config,
        )
            .map_err(|e| anyhow::anyhow!("Gossipsub init error: {}", e))?;
        let autonat = autonat::Behaviour::new(peer_id, autonat::Config::default());
        Ok(Self {
            ping,
            kademlia,
            gossipsub,
            autonat,
        })
    }
}