#!/bin/bash
#SBATCH -N1                                   # Liczba węzłów
#SBATCH -c5                                   # Liczba rdzeni cpu  
#SBATCH --mem=16gb                            # Ilość pamięci RAM
#SBATCH --time=3:00:00                        # Limit czasowy na zadanie
#SBATCH --job-name=squirrel_${DBMS}_${USE_OLD_SQUIRREL}_build       # Nazwa zadania
#SBATCH -p lem-cpu-short                      # Nazwa partycji

# This script is used in the GitHub Actions workflow to build the project container in WCSS.

DBMS=${1:-sqlite}
export USE_OLD_SQUIRREL=${2:-1}

# Pull the base image from Docker Hub and convert it to SIF format for Apptainer 
# (need this for fakeroot to work)
DOCKER_IMAGE="docker://syurima/ubuntu-fakeroot:latest"
BASE_SIF="/tmp/ubuntu-fakeroot.sif"

echo "=== Pulling Docker image ${DOCKER_IMAGE} into ${BASE_SIF} ==="
apptainer pull ${BASE_SIF} ${DOCKER_IMAGE}
if [[ $? -ne 0 ]]; then
    echo "Failed to pull Docker image"
    exit 1
fi

# Build the actual container image
APPTAINER_DEFINITION="scripts/apptainers/${DBMS}.def"
APPTAINER_IMAGE="${PWD}/squirrel-${DBMS}_${USE_OLD_SQUIRREL}.sif"

echo "=== Building Apptainer image ${APPTAINER_IMAGE} ==="
apptainer build \
    --ignore-fakeroot-command \
    --env USE_OLD_SQUIRREL="${USE_OLD_SQUIRREL}" \
    "$APPTAINER_IMAGE" "$APPTAINER_DEFINITION"

if [[ ! -f "$APPTAINER_IMAGE" ]]; then
  echo "Failed to build Apptainer image. Expected image not found at $APPTAINER_IMAGE"
  exit 1
fi

echo "=== Build finished ==="