#!/bin/bash
#SBATCH -N1                                   # Liczba węzłów
#SBATCH -c5                                   # Liczba rdzeni cpu  
#SBATCH --mem=16gb                            # Ilość pamięci RAM
#SBATCH --job-name=squirrel_${DBMS}_run       # Nazwa zadania
#SBATCH -p lem-gpu-short                      # Nazwa partycji
#SBATCH --gres=gpu:hopper:1                   # Potrzebne zasoby GPU

# This script is used in the GitHub Actions workflow to run the fuzzing job on WCSS.

DBMS=${1:-sqlite}

APPTAINER_IMAGE="${PWD}/squirrel-${DBMS}.sif"

if [[ ! -f "$APPTAINER_IMAGE" ]]; then
  echo "Image $APPTAINER_IMAGE not found. You need to run the build job first to create the image."
  exit 1
fi

echo "=== Running fuzzing job with Apptainer image ${APPTAINER_IMAGE} ==="
# --nv is needed to enable GPU support in the container. don't delete it!!
apptainer run --nv $APPTAINER_IMAGE
echo "=== Fuzzing job finished ==="