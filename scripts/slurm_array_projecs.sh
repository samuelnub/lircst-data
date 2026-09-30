#!/bin/bash
set -euo pipefail

echo "Usage: $0 <seed> <events (optional, default=10000)>"


SEED="$1"
EVENTS="${2:-10000}"  # Default to 10k events if not provided

OUTPUT_ROOT="/mnt/fast/nobackup/users/sy00917/dev/lircst-data/build/output"
OUTPUT_DIR="${OUTPUT_ROOT}/${SEED}"

mkdir -p "${OUTPUT_DIR}"

missing=()

for theta_idx in $(seq 0 199); do
    file="${OUTPUT_DIR}/ti${theta_idx}.npy"

    if [[ ! -f "${file}" ]]; then
        missing+=("${theta_idx}")
    fi
done

if [[ ${#missing[@]} -eq 0 ]]; then
    echo "Seed ${SEED}: all 200 projections already exist."
    exit 0
fi

# Convert Bash array:
#   (12 56 93 103)
#
# into Slurm syntax:
#   12,56,93,103

array_spec=$(IFS=,; echo "${missing[*]}")

echo "Seed: ${SEED}"
echo "Events: ${EVENTS}"
echo "Missing ${#missing[@]} / 200 projections:"
echo "${array_spec}"


PHAN_FILE="${OUTPUT_DIR}/phan.npy"

if [[ -f "${PHAN_FILE}" ]]; then
    echo "Phantom already exists."
    NEED_PHANTOM=false
else
    echo "Phantom does not exist; one array task will generate it."
    NEED_PHANTOM=true
fi


array_spec=$(IFS=,; echo "${missing[*]}")

MAX_CONCURRENT=10  # Limit to 10 concurrent jobs

sbatch \
    --array="${array_spec}%${MAX_CONCURRENT}" \
    --export=ALL,SEED="${SEED}",EVENTS="${EVENTS}",GENERATE_PHANTOM="${NEED_PHANTOM}" \
    geant4_array.slurm