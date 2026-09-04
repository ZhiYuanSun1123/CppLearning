#!/usr/bin/env bash
set -euo pipefail

if ! command -v clang-format >/dev/null 2>&1; then
    echo "error: clang-format was not found" >&2
    exit 2
fi

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${project_root}"

files=()
while IFS= read -r -d '' file; do
    files+=("${file}")
done < <(
    find include src app tests \
        -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) \
        -print0
)

if [[ ${#files[@]} -eq 0 ]]; then
    echo "error: no C++ files found to format" >&2
    exit 3
fi

clang-format -i "${files[@]}"
echo "Formatted ${#files[@]} C++ files"
