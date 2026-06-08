#!/usr/bin/env bash
set -euo pipefail

# Usage: ./scripts/run_docker.sh <dbms>

DBMS=${1:-sqlite}
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

echo "Starting container: ${CONTAINER_NAME}"
echo "Results will be copied to: ${OUTPUT_PATH}"

if [ "${2:-}" = "benchmark" ]; then
  echo "Running mutator benchmark..."
  docker run --rm -it \
    --entrypoint bash \
    -e AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 \
    "$DOCKER_IMAGE" \
    -c "cd /home/Squirrel && ./build/tests/mutator_benchmark UCB1"
  exit 0
else
  set +e
  docker run -i \
    --name "$CONTAINER_NAME" \
    -e AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 \
    "$DOCKER_IMAGE"
  DOCKER_EXIT_CODE=$?
  set -e
fi

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
