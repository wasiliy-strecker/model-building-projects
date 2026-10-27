#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
source scripts/env.sh
modellbau_preset="${1:-dev}"
mapfile -d '' modellbau_files < <(find src tests -type f \( -name '*.h' -o -name '*.cpp' \) -print0)
clang-format --dry-run --Werror "${modellbau_files[@]}"
cmake --preset "$modellbau_preset"
cmake --build --preset "$modellbau_preset" --parallel 4
ctest --preset "$modellbau_preset"
if [[ "$modellbau_preset" != asan ]]; then
  mapfile -d '' modellbau_sources < <(find src -name '*.cpp' -print0)
  clang-tidy -p "build/$modellbau_preset" "${modellbau_sources[@]}" --quiet
fi
git diff --check
