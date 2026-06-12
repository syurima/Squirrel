#!/bin/bash
#SBATCH -N1                                   # Liczba węzłów
#SBATCH --mem=32gb                            # Ilość pamięci RAM
#SBATCH --job-name=squirrel_${DBMS}_${USE_OLD_SQUIRREL}_run       # Nazwa zadania
#SBATCH -p lem-gpu-short                      # Nazwa partycji
#SBATCH --gres=gpu:hopper:2                   # Potrzebne zasoby GPU

# This script is used in the GitHub Actions workflow to run the fuzzing job on WCSS.

DBMS=${1:-sqlite}
export USE_OLD_SQUIRREL=${2:-1}
export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 # to avoid issues with AFL++ when running in a non-privileged container environment.

# Arguments from the GitHub workflow:
#   $3 – AFL workers (0 means auto‑detect based on allocated CPUs)
#   $4 – number of CPU cores allocated for the Slurm job (slurm_cores)
export AFL_WORKERS=${3:-0}
export SLURM_CORES=${4:-12}

# If AFL_WORKERS is 0, default to using all allocated cores - reserved cores.
RESERVED_CORES=2 # Keep some cores free for system tasks and the main fuzzing process.
if [ "$AFL_WORKERS" -eq 0 ]; then
  export AFL_WORKERS=$((SLURM_CORES - RESERVED_CORES))
fi

# Disable CPU affinity inside the container (AFL++ cannot set affinity in Apptainer)
export AFL_NO_AFFINITY=1


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