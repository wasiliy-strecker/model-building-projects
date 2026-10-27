#!/usr/bin/env bash
# Optional machine-local SDK paths. No dependency on another application.
modellbau_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [[ -f "$modellbau_root/.tools/env.sh" ]]; then
  source "$modellbau_root/.tools/env.sh"
fi
if [[ -n "${MODELLBAUPROJEKT_QT_ROOT:-}" ]]; then
  export CMAKE_PREFIX_PATH="$MODELLBAUPROJEKT_QT_ROOT${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
  export PATH="$MODELLBAUPROJEKT_QT_ROOT/bin:$PATH"
fi
