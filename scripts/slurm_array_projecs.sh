#!/bin/bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <seed>"
    exit 1
fi

SEED="$1"
EVENTS="${2:-10000}"  # Default to 10k events if not provided

OUTPUT_ROOT="/mnt/fast/nobackup/users/sy00917/dev/lircst-diffusion/build/output"
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
echo "Missing ${#missing[@]} / 200 projections:"
echo "${array_spec}"

sbatch \
    --array="${array_spec}" \
    geant4_array.slurm "${SEED}" "${EVENTS}"