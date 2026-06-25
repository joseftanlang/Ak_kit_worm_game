#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
WASM_DIR="$ROOT/wasm"
OUT_DIR="$ROOT/js"

if command -v emcc >/dev/null 2>&1; then
  make -C "$WASM_DIR"
  echo "Built with local emcc."
  exit 0
fi

if command -v docker >/dev/null 2>&1; then
  echo "Building with Emscripten Docker image..."
  docker run --rm \
    -v "$ROOT:/src" \
    -w /src/wasm \
    emscripten/emsdk:3.1.51 \
    make
  echo "Built with Docker emscripten."
  exit 0
fi

echo "Error: install Emscripten or Docker to build WASM." >&2
exit 1
