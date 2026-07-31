#!/usr/bin/env bash
# Build and run the InkRecipes host tests with a plain host compiler.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="$(cd "$here/../.." && pwd)"

CXX="${CXX:-g++}"
out="$(mktemp -d)"
trap 'rm -rf "$out"' EXIT

echo "Compiling host tests (CXX=$CXX)"
"$CXX" -std=gnu++2a -Wall -Wextra -Werror \
  -I"$root/src" \
  "$root/src/core/Recipe.cpp" \
  "$here/test_core.cpp" \
  -o "$out/test_core"

"$out/test_core" "$root"/recipes/built/*.rcp
