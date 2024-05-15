#!/usr/bin/env bash
set -euo pipefail

cmake -B build -S . "-DCMAKE_TOOLCHAIN_FILE=../../../vcpkg/scripts/buildsystems/vcpkg.cmake"

cmake --build build -j 4