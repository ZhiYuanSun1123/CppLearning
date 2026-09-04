#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${project_root}"

repeat_count="${1:-50}"

if ! [[ "${repeat_count}" =~ ^[1-9][0-9]*$ ]]; then
    echo "error: repeat count must be a positive integer" >&2
    exit 2
fi

cmake --preset debug
cmake --build --preset debug
ctest --test-dir build/debug \
    --repeat until-fail:"${repeat_count}" \
    --output-on-failure

echo "All tests remained stable for ${repeat_count} runs."
