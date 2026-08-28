#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
WEB_BUILD_DIR="${PROJECT_DIR}/web-build"

if [ ! -d "${WEB_BUILD_DIR}" ] || [ ! -f "${WEB_BUILD_DIR}/index.html" ]; then
    echo "Error: web-build directory or web-build/index.html not found." >&2
    echo "Please run ./scripts/build-web.sh first." >&2
    exit 1
fi

PORT="${PORT:-8080}"
BIND="${BIND:-0.0.0.0}"

# Find available port if specified port is in use
while python3 -c "import socket; s = socket.socket(); s.bind(('${BIND}', ${PORT})); s.close()" 2>/dev/null; [ $? -ne 0 ]; do
    echo "Port ${PORT} is in use, checking next port..."
    PORT=$((PORT + 1))
done

echo "SERVER: running"
echo "PORT: ${PORT}"
echo "BIND: ${BIND}"
echo "LOCAL URL: http://localhost:${PORT}"

if [ -n "${WEB_HOST:-}" ]; then
    echo "CLOUD SHELL URL: https://${PORT}-${WEB_HOST}"
elif [ -n "${DEVSHELL_CLIENT_PORT:-}" ]; then
    echo "CLOUD SHELL URL: https://${PORT}-cs-${DEVSHELL_CLIENT_PORT}.cloudshell.dev"
elif [ -n "${DEVSHELL_PROJECT_ID:-}" ]; then
    echo "CLOUD SHELL URL: https://${PORT}-ide-${DEVSHELL_PROJECT_ID}.cloudshell.dev"
fi

cd "${WEB_BUILD_DIR}"
exec python3 -m http.server "${PORT}" --bind "${BIND}"
