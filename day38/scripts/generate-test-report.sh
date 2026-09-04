#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${project_root}"

mkdir -p report

cmake --preset debug
cmake --build --preset debug
ctest --test-dir build/debug \
    --output-on-failure \
    --output-junit "${project_root}/report/test-results.xml"

echo "Test report: ${project_root}/report/test-results.xml"
