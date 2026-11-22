#!/usr/bin/env sh
set -e
cd "$(dirname "$0")"

cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
