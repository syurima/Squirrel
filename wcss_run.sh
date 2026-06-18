#!/bin/bash
#SBATCH -N1                                   # Liczba węzłów
#SBATCH --job-name=squirrel_${DBMS}_${USE_OLD_SQUIRREL}_run       # Nazwa zadania
#SBATCH -p lem-gpu-short                      # Nazwa partycji
#SBATCH --gres=gpu:hopper:1                   # Potrzebne zasoby GPU

# This script is used in the GitHub Actions workflow to run the fuzzing job on WCSS.

DBMS=${1:-sqlite}
export USE_OLD_SQUIRREL=${2:-1}
export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 # to avoid issues with AFL++ when running in a non-privileged container environment.


export AFL_WORKERS=${3:-0} # number ofAFL workers (0 means auto‑detect based on allocated CPUs)
export SLURM_CORES=${4:-12} # number of CPU cores allocated for the Slurm job
export USE_LLM=${5:-1} # Whether to enable LLM support in the fuzzing job (1 to enable, 0 to disable)

# If AFL_WORKERS is 0, default to using all allocated cores - reserved cores.
RESERVED_CORES=2 # Keep some cores free for system tasks and the main fuzzing process.
if [ "$AFL_WORKERS" -eq 0 ]; then
  export AFL_WORKERS=$((SLURM_CORES - RESERVED_CORES))
fi

# Disable CPU affinity inside the container (AFL++ cannot set affinity in Apptainer)
export AFL_NO_AFFINITY=1

# Find the Apptainer image
IMAGE_DIR="${HOME}/images"
APPTAINER_IMAGE="${IMAGE_DIR}/squirrel-${DBMS}_${USE_OLD_SQUIRREL}.sif"

if [[ ! -f "$APPTAINER_IMAGE" ]]; then
  echo "Image $APPTAINER_IMAGE not found. You need to run the build job first to create the image."
  exit 1
fi

# Create a unique results directory for this run
LLM_SUFFIX="NoLLM"
if [ "$USE_LLM" -eq 1 ]; then
  LLM_SUFFIX="LLM"
fi

OLD_SQUIRREL_SUFFIX="NEW"
if [ "$USE_OLD_SQUIRREL" -eq 1 ]; then
  OLD_SQUIRREL_SUFFIX="OLD"
fi

RESULTS_DIR="${HOME}/results/${DBMS}_${OLD_SQUIRREL_SUFFIX}_${LLM_SUFFIX}_$(date +%Y-%m-%d_%H-%M-%S)"
mkdir -p "$RESULTS_DIR"

# Run the Apptainer
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