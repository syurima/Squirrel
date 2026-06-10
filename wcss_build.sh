#!/bin/bash
#SBATCH -N1                                   # Liczba węzłów
#SBATCH -c5                                   # Liczba rdzeni cpu  
#SBATCH --mem=16gb                            # Ilość pamięci RAM
#SBATCH --time=3:00:00                        # Limit czasowy na zadanie
#SBATCH --job-name=squirrel_${DBMS}_build     # Nazwa zadania
#SBATCH -p lem-cpu-short                      # Nazwa partycji

set -euo pipefail

# This script is used in the GitHub Actions workflow to build the project container in WCSS.

DBMS=${1:-sqlite}
# USE_REMOTE=0
# if [[ ${2:-} == "--remote" ]]; then
#   USE_REMOTE=1
# fi

APPTAINER_DEFINITION="scripts/apptainers/${DBMS}.def"
APPTAINER_IMAGE="${PWD}/squirrel-${DBMS}.sif"

echo "=== Building Apptainer image ${APPTAINER_IMAGE} ==="
apptainer build $APPTAINER_IMAGE $APPTAINER_DEFINITION

if [[ ! -f "$APPTAINER_IMAGE" ]]; then
  echo "Failed to build Apptainer image. Expected image not found at $APPTAINER_IMAGE"
  exit 1
fi

echo "=== Build finished ==="