#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
source scripts/env.sh
cmake --preset dev
cmake --build --preset dev --parallel 4
exec build/dev/modellbauprojekt "$@"
