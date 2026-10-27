#!/usr/bin/env bash

# builds and runs every bench
# run from repo root: forcebench/run.sh

set -euo pipefail

OUT="$(mktemp -d)"
trap 'rm -rf "$OUT"' EXIT

VARIANTS=(original arrayutils_before_fix soa_vectorized_all_pairs soa_newton3_scalar soa_newton3_diagonal aos_newton3_macro submitted)

echo "compiler,flags,variant,ns_per_n2" > forcebench/results.csv

for variant in "${VARIANTS[@]}"; do
  case "$variant" in
    arrayutils_before_fix)  includes="-I forcebench/legacy";  extra="";;
    soa_*)                  includes="-I src";                extra="src/ParticleContainer.cpp" ;;
    submitted)              includes="-I src"                 extra="src/ParticleContainer.cpp src/FileReader.cpp src/outputWriter/XYZWriter.cpp";;
    *)                      includes="-I src";                extra="";;
  esac

  for CXX in g++ clang++; do
    for flags in "-O3" "-O3 -march=native"; do
      exe="$OUT/$variant"
      $CXX -std=c++20 $flags -DNDEBUG $includes "forcebench/variants/$variant.cpp" $extra -o "$exe"

      read -r name ns _ cs <<< "$("$exe")"
      echo "$CXX $flags: $name $ns cs: $cs"
      echo "$CXX,$flags,$name,$ns" >> forcebench/results.csv
    done
  done
done
