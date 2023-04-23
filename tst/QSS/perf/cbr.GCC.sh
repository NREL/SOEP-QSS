#!/bin/bash
# GCC Build Release

if [ -f $1.cc ]; then
  CxxSource=$1.cc
  OutputSpec="-o $1"
  shift
fi

g++ -pipe -std=c++20 -pedantic -fopenmp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -DNDEBUG -O3 -fno-stack-protector -finline-limit=2000 -s $CxxSource $@ $OutputSpec -lQSS
