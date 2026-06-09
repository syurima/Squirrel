#!/usr/bin/env bash
set -euo pipefail

# Usage: ./scripts/build_docker.sh <dbms> [--remote]
# Examples:
#   ./scripts/build_docker.sh sqlite
#   ./scripts/build_docker.sh sqlite --remote

DBMS=${1:-sqlite}
USE_REMOTE=0
if [[ ${2:-} == "--remote" ]]; then
  USE_REMOTE=1
fi

DOCKERFILE="scripts/dockers/${DBMS}/Dockerfile"
if [ ! -f "$DOCKERFILE" ]; then
  echo "Dockerfile not found for DBMS: $DBMS"
  exit 2
fi

# Build from repo root so the build context contains everything
docker build \
  --build-arg USE_REMOTE=${USE_REMOTE} \
  -t squirrel-${DBMS} \
  -f "$DOCKERFILE" .
