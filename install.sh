#!/usr/bin/env bash
set -e

if [ "$EUID" -ne 0 ]; then
  echo "Erreur : Veuillez executer l'installateur avec sudo (sudo ./install.sh)."
  exit 1
fi

echo "=========================================="
echo "    Installation de vLia Ecosystem"
echo "=========================================="

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_DIR="/etc/vlia"
DATA_DIR="/var/lib/vlianet"

echo "[1/5] Preparation des repertoires systeme..."
mkdir -p "$CONFIG_DIR"
mkdir -p "$DATA_DIR"

echo "[2/5] Configuration du modele LLM..."

SELECTED_MODEL=""
if command -v ollama &> /dev/null; then
    echo "Analyse des modeles Ollama disponibles sur la machine..."
    MODELS=$(ollama list | tail -n +2 | awk '{print $1}')

    if [ -n "$MODELS" ]; then
        echo "Modeles detectes :"
        select M in $MODELS "Saisie manuelle"; do
            if [ "$M" == "Saisie manuelle" ]; then
                read -p "Nom du modele Ollama : " SELECTED_MODEL
                break
            elif [ -n "$M" ]; then
                SELECTED_MODEL="$M"
                break
            fi
        done
    fi
fi

if [ -z "$SELECTED_MODEL" ]; then
    read -p "Aucun modele detecte. Entrez le nom du modele Ollama (ex: qwen2.5-coder:latest) : " SELECTED_MODEL
fi

cat <<EOF > "$CONFIG_DIR/vliasys.json"
{
    "model": "$SELECTED_MODEL",
    "socket_path": "/tmp/vliasys.sock"
}
EOF
echo "Configuration enregistree dans $CONFIG_DIR/vliasys.json (modele: $SELECTED_MODEL)"

if [ ! -f "$CONFIG_DIR/vlianet.env" ]; then
cat <<EOF > "$CONFIG_DIR/vlianet.env"
PORT=4001
IDENTITY_FILE=$DATA_DIR/node_identity.bytes
# BOOTSTRAP_PEER=/ip4/x.x.x.x/tcp/4001
EOF
fi

echo "[3/5] Compilation de vliasys et vlianet..."

systemctl stop vliasys vlianet 2>/dev/null || true

# Compilation vliasys
if [ -d "$SCRIPT_DIR/vliasys" ]; then
    echo "-> Build vliasys (C++)..."
    cd "$SCRIPT_DIR/vliasys"
    mkdir -p build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j$(nproc)
    cp vliasys /usr/local/bin/
    echo "vliasys installe dans /usr/local/bin/vliasys"
else
    echo "Avertissement : Dossier vliasys introuvable dans $SCRIPT_DIR"
fi

if [ -d "$SCRIPT_DIR/vlianet" ]; then
    echo "-> Build vlianet (Rust)..."
    cd "$SCRIPT_DIR/vlianet"

    if [ -n "$SUDO_USER" ]; then
        sudo -u "$SUDO_USER" bash -c 'source "$HOME/.cargo/env" 2>/dev/null || true; cargo build --release'
    else
        cargo build --release
    fi

    cp target/release/vlianet /usr/local/bin/
    echo "vlianet installe dans /usr/local/bin/vlianet"
else
    echo "Avertissement : Dossier vlianet introuvable dans $SCRIPT_DIR"
fi

echo "[4/5] Installation du CLI vlia..."

cat <<'EOF' > /usr/local/bin/vlia
#!/usr/bin/env bash
set -e

case "$1" in
  status)
    echo "=== Statut vLia System ==="
    systemctl status vliasys --no-pager || true
    echo ""
    systemctl status vlianet --no-pager || true
    ;;

  start)
    echo "Demarrage des services vLia..."
    sudo systemctl start vliasys vlianet
    ;;

  stop)
    echo "Arret des services vLia..."
    sudo systemctl stop vlianet vliasys
    ;;

  restart)
    echo "Redemarrage des demons..."
    sudo systemctl restart vliasys vlianet
    ;;

  logs)
    sudo journalctl -u vliasys -u vlianet -f
    ;;

  off-vliasys)
    echo "Coupure du runtime local (vliasys)..."
    sudo systemctl stop vliasys
    ;;

  on-vliasys)
    echo "Demarrage du runtime local (vliasys)..."
    sudo systemctl start vliasys
    ;;

  config)
    echo "Modification de la configuration vliasys (/etc/vlia/vliasys.json)..."
    sudo ${EDITOR:-nano} /etc/vlia/vliasys.json
    echo "Appliquer les changements : vlia restart"
    ;;

  *)
    echo "Usage: vlia {status|start|stop|restart|logs|off-vliasys|on-vliasys|config}"
    exit 1
    ;;
esac
EOF

chmod +x /usr/local/bin/vlia
echo "CLI vlia installe dans /usr/local/bin/vlia"

echo "[5/5] Configuration des services systemd..."

cat <<EOF > /etc/systemd/system/vliasys.service
[Unit]
Description=vLia Runtime Service
After=network.target

[Service]
Type=simple
ExecStart=/usr/local/bin/vliasys
Restart=always
RestartSec=2
TimeoutStopSec=3s

[Install]
WantedBy=multi-user.target
EOF

cat <<EOF > /etc/systemd/system/vlianet.service
[Unit]
Description=vLia P2P Network Daemon
After=vliasys.service
Requires=vliasys.service

[Service]
Type=simple
EnvironmentFile=/etc/vlia/vlianet.env
ExecStart=/usr/local/bin/vlianet -p \$PORT -i \$IDENTITY_FILE
Restart=always
RestartSec=2
TimeoutStopSec=3s

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable vliasys vlianet
systemctl restart vliasys vlianet

echo "=========================================="
echo "    Installation terminee avec succes."
echo "=========================================="
echo "Verifier l'etat : vlia status"
echo "Suivre les logs : vlia logs"
