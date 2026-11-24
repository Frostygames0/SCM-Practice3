#!/usr/bin/env sh
set -e
cd "$(dirname "$0")"

source=${1:-examples/vector_min_1.asm}
first=${2:-0}
last=${3:-4}
name=$(basename "$source" .asm)

cmake -S . -B build
cmake --build build
mkdir -p out

build/bin/asm -i "$source" -r "out/$name.bin" -t
echo ''
build/bin/vm -p "out/$name.bin" -d "out/$name.xml" -r "$first" "$last"
cat "out/$name.xml"
