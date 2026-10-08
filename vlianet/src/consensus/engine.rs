use crate::consensus::types::ConsensusMessage;
use sha2::{Digest, Sha256};
use std::collections::HashMap;
use tracing::{info, warn};

#[derive(Default)]
pub struct JobState {
    pub prompt: String,
    pub quorum: usize,
    pub commits: HashMap<String, String>,            
    pub reveals: HashMap<String, (String, String)>,  
}

#[derive(Default)]
pub struct ConsensusEngine {
    pub jobs: HashMap<String, JobState>,
}

impl ConsensusEngine {
    pub fn new() -> Self {
        Self::default()
    }
    pub fn handle_message(&mut self, msg: ConsensusMessage) {
        match msg {
            ConsensusMessage::JobProposal { job_id, prompt, quorum, .. } => {
                info!("New Job Proposal received: ID = {}", job_id);
                self.jobs.entry(job_id).or_insert_with(|| JobState {
                    prompt,
                    quorum,
                    commits: HashMap::new(),
                    reveals: HashMap::new(),
                });
            }
            ConsensusMessage::JobCommit { job_id, worker_peer_id, commit_hash } => {
                if let Some(state) = self.jobs.get_mut(&job_id) {
                    info!("Received commit for Job {} from Peer {}", job_id, worker_peer_id);
                    state.commits.insert(worker_peer_id, commit_hash);
                }
            }
            ConsensusMessage::JobReveal { job_id, worker_peer_id, raw_result, nonce } => {
                if let Some(state) = self.jobs.get_mut(&job_id) {
                    if let Some(expected_hash) = state.commits.get(&worker_peer_id) {
                        let mut hasher = Sha256::new();
                        hasher.update(format!("{}{}", raw_result, nonce).as_bytes());
                        let calculated_hash = hex::encode(hasher.finalize());

                        if calculated_hash == *expected_hash {
                            info!("Valid reveal for Job {} from Peer {}", job_id, worker_peer_id);
                            state.reveals.insert(worker_peer_id, (raw_result, nonce));
                        } else {
                            warn!("Invalid reveal from Peer {}: Hash mismatch!", worker_peer_id);
                        }
                    }
                }
            }
        }
    }

    pub fn try_evaluate_consensus(&self, job_id: &str) -> Option<String> {
        let state = self.jobs.get(job_id)?;
        if state.reveals.len() < state.quorum {
            return None;
        }

        let mut frequency: HashMap<String, usize> = HashMap::new();
        for (result, _) in state.reveals.values() {
            *frequency.entry(result.clone()).or_insert(0) += 1;
        }

        frequency.into_iter().max_by_key(|(_, count)| *count).map(|(res, _)| res)
    }
}