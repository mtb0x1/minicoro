#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
CC=${CC:-clang}
OUTPUT=${1:-"$ROOT/minicoro.wasm"}

if ! command -v "$CC" >/dev/null 2>&1; then
  printf 'error: C compiler not found: %s\n' "$CC" >&2
  exit 1
fi

case "$OUTPUT" in
  /*) ;;
  *) OUTPUT="$PWD/$OUTPUT" ;;
esac

mkdir -p "$(dirname -- "$OUTPUT")"
BUILD_DIR=$(mktemp -d)
trap 'rm -rf "$BUILD_DIR"' EXIT HUP INT TERM

TARGET_FLAGS='--target=wasm32-unknown-unknown -std=c89 -O2 -ffreestanding -fno-builtin -ffunction-sections -fdata-sections -DMCO_USE_VMEM_ALLOCATOR -DMCO_NO_MULTITHREAD'

"$CC" $TARGET_FLAGS -DMINICORO_IMPL -c "$ROOT/minicoro.c" -o "$BUILD_DIR/minicoro.o"
"$CC" $TARGET_FLAGS -I"$ROOT" -c "$ROOT/minicoro_wasm_demo.c" -o "$BUILD_DIR/minicoro_wasm_demo.o"

"$CC" --target=wasm32-unknown-unknown -nostdlib \
  "$BUILD_DIR/minicoro.o" "$BUILD_DIR/minicoro_wasm_demo.o" \
  -Wl,--no-entry \
  -Wl,--gc-sections \
  -Wl,--export-memory \
  -Wl,--initial-memory=131072 \
  -Wl,--max-memory=2147483648 \
  -Wl,--export=mco_demo_create_many \
  -Wl,--export=mco_demo_suspended_count \
  -Wl,--export=mco_demo_destroy_all \
  -o "$OUTPUT"

printf 'built %s\n' "$OUTPUT"