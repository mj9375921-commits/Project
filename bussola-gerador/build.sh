#!/bin/sh
# Compila o gerador da Bússola para WebAssembly (../docs/bussola/gerador.wasm).
# Precisa de: clang, lld, wasi-libc e libclang-rt-18-dev-wasm32 (apt).
set -e
cd "$(dirname "$0")"
clang --target=wasm32-wasi -O2 -mexec-model=reactor -I. \
  -Wl,--strip-all -Wl,--export=_initialize \
  bussola.c cubiomes/biomes.c cubiomes/finders.c cubiomes/generator.c cubiomes/layers.c \
  cubiomes/noise.c cubiomes/biomenoise.c \
  -o ../docs/bussola/gerador.wasm
echo "ok: ../docs/bussola/gerador.wasm"
