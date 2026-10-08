use anyhow::{Context, Result};
use libp2p::identity::{self, Keypair};
use std::fs;
use std::path::Path;
use tracing::info;

pub struct NodeIdentity {
    pub keypair: Keypair,
}

impl NodeIdentity {
    pub fn load_or_generate(key_path: &Path) -> Result<Self> {
        if key_path.exists() {
            info!("Loading existing P2P identity key from {:?}", key_path);
            let bytes = fs::read(key_path).context("Failed to read identity key file")?;
            let keypair = Keypair::from_protobuf_encoding(&bytes)
                .context("Failed to decode protobuf keypair")?;
            Ok(Self { keypair })
        } else {
            info!("Generating new Ed25519 P2P identity key...");
            let keypair = Keypair::generate_ed25519();
            let bytes = keypair
                .to_protobuf_encoding()
                .context("Failed to encode keypair to protobuf")?;

            if let Some(parent) = key_path.parent() {
                fs::create_dir_all(parent)?;
            }
            fs::write(key_path, bytes)?;
            info!("Saved new identity key to {:?}", key_path);

            Ok(Self { keypair })
        }
    }

    pub fn peer_id(&self) -> libp2p::PeerId {
        self.keypair.public().to_peer_id()
    }
}