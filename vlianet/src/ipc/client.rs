use anyhow::{Context, Result};
use serde::{Deserialize, Serialize};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::UnixStream;
use tracing::info;

#[derive(Serialize, Deserialize, Debug)]
pub struct VliaSysTaskRequest {
    pub task_id: String,
    pub prompt: String,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct VliaSysTaskResponse {
    pub task_id: String,
    pub success: bool,
    pub output: String,
    pub error: String,
}

pub struct VliaSysIpcClient {
    socket_path: String,
}

impl VliaSysIpcClient {
    pub fn new(socket_path: impl Into<String>) -> Self {
        Self {
            socket_path: socket_path.into(),
        }
    }

    pub async fn execute_task(&self, task_id: &str, prompt: &str) -> Result<VliaSysTaskResponse> {
        info!("Connecting to vliasys IPC socket at {}", self.socket_path);
        let mut stream = UnixStream::connect(&self.socket_path)
            .await
            .context("Failed to connect to vliasys IPC socket. Is vliasys running?")?;

        let req = VliaSysTaskRequest {
            task_id: task_id.to_string(),
            prompt: prompt.to_string(),
        };

        let mut payload = serde_json::to_vec(&req)?;
        payload.push(b'\n');

        stream.write_all(&payload).await?;

        let mut buffer = Vec::new();
        let mut tmp = [0u8; 1024]; 
        loop {
            let n = stream.read(&mut tmp).await?; 
            if n == 0 {
                break;
            }
            buffer.extend_from_slice(&tmp[..n]);
            if buffer.contains(&b'\n') {
                break;
            }
        }

        let response: VliaSysTaskResponse = serde_json::from_slice(&buffer)?;
        Ok(response)
    }
}