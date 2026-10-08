use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug, Clone)]
pub enum ConsensusMessage {
    JobProposal {
        job_id: String,
        prompt: String,
        initiator_peer_id: String,
        quorum: usize,
    },
    JobCommit {
        job_id: String,
        worker_peer_id: String,
        commit_hash: String,
    },
    JobReveal {
        job_id: String,
        worker_peer_id: String,
        raw_result: String,
        nonce: String,
    },
}