#!/bin/bash
#SBATCH -N1                                   # Liczba węzłów
#SBATCH -c12                                   # Liczba rdzeni cpu  
#SBATCH --mem=16gb                            # Ilość pamięci RAM
#SBATCH --job-name=squirrel_${DBMS}_${USE_OLD_SQUIRREL}_run       # Nazwa zadania
#SBATCH -p lem-gpu-short                      # Nazwa partycji
#SBATCH --gres=gpu:hopper:1                   # Potrzebne zasoby GPU

# This script is used in the GitHub Actions workflow to run the fuzzing job on WCSS.

DBMS=${1:-sqlite}
export USE_OLD_SQUIRREL=${2:-1}
export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 # to avoid issues with AFL++ when running in a non-privileged container environment.


IMAGE_DIR="${HOME}/images"
APPTAINER_IMAGE="${IMAGE_DIR}/squirrel-${DBMS}_${USE_OLD_SQUIRREL}.sif"
RESULTS_DIR="${HOME}/results/${DBMS}_${USE_OLD_SQUIRREL}_$(date +%Y-%m-%d_%H-%M-%S)"

if [[ ! -f "$APPTAINER_IMAGE" ]]; then
  echo "Image $APPTAINER_IMAGE not found. You need to run the build job first to create the image."
  exit 1
fi

mkdir -p "$RESULTS_DIR"

echo "=== Running fuzzing job with Apptainer image ${APPTAINER_IMAGE} ==="
# --nv is needed to enable GPU support in the container. don't delete it!!
apptainer run \
    --nv \
    -B "${RESULTS_DIR}:/tmp/fuzz" \
    "${APPTAINER_IMAGE}"
APPTAINER_EXIT_CODE=$?

echo "=== Fuzzing job finished (exit code $APPTAINER_EXIT_CODE) ==="

if [[ $APPTAINER_EXIT_CODE -ne 0 ]]; then
    echo "Apptainer job failed."
    exit $APPTAINER_EXIT_CODE
fi

echo "Results are available in ${RESULTS_DIR}"