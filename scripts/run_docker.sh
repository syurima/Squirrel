#!/usr/bin/env bash
set -euo pipefail

# Usage: ./scripts/run_docker.sh <dbms> [seed]

DBMS=${1:-sqlite}
# Optional deterministic seed (overrides SQUIRREL_SEED env). Default set below.
DEFAULT_SEED=42
SEED=${2:-}
DOCKER_IMAGE="squirrel-${DBMS}:latest"

# Setup local results path and create it if it doesn't exist
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_ROOT="${SCRIPT_DIR}/../output"
DATE=$(date +%Y-%m-%d)
# Generate a single random ID for this run (use uuidgen when available)
if command -v uuidgen >/dev/null 2>&1; then
	RANDOM_ID=$(uuidgen)
else
	RANDOM_ID="$(date +%s)-$RANDOM"
fi

# Use an underscore to separate date and the random id to avoid spaces in paths
OUTPUT_PATH="${OUTPUT_ROOT}/${DBMS}/${DATE}_${RANDOM_ID}"
mkdir -p "$OUTPUT_PATH"

CONTAINER_NAME="squirrel-${DBMS}-run-${RANDOM_ID//[^a-zA-Z0-9_.-]/-}"

# Determine seed: CLI arg > env SQUIRREL_SEED > default
if [ -n "$SEED" ]; then
	CHOSEN_SEED="$SEED"
elif [ -n "${SQUIRREL_SEED:-}" ]; then
	CHOSEN_SEED="$SQUIRREL_SEED"
else
	CHOSEN_SEED=$DEFAULT_SEED
fi

echo "Starting container: ${CONTAINER_NAME}"
echo "Using seed: ${CHOSEN_SEED} (settable via second arg or SQUIRREL_SEED env)"
echo "Results will be copied to: ${OUTPUT_PATH}"

# AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 potentialy to be removed in the future, but for now it prevents AFL from complaining
set +e
docker run -i \
	--name "$CONTAINER_NAME" \
	-e AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 \
	-e SQUIRREL_SEED="$CHOSEN_SEED" \
	$DOCKER_IMAGE
DOCKER_EXIT_CODE=$?
set -e

echo "Container stopped (exit code: ${DOCKER_EXIT_CODE}). Copying results..."
if docker cp "${CONTAINER_NAME}:/tmp/fuzz/." "$OUTPUT_PATH"; then
	echo "Results copied successfully to: ${OUTPUT_PATH}"
else
	echo "Warning: failed to copy /tmp/fuzz from container ${CONTAINER_NAME}" >&2
fi

docker stop "$CONTAINER_NAME" >/dev/null 2>&1 || true
docker rm "$CONTAINER_NAME" >/dev/null 2>&1 || true

if [ "$DOCKER_EXIT_CODE" -eq 130 ]; then
	echo "Fuzzing run stopped manually."
fi

exit "$DOCKER_EXIT_CODE"