#!/bin/bash
# GCC Build Release

if [ -f $1.cc ]; then
  CxxSource=$1.cc
  OutputSpec="-o $1"
  shift
fi

g++ -pipe -std=c++17 -pedantic -Wall -Wextra -m64 -march=native -DNDEBUG -O3 -fno-stack-protector -finline-limit=1000 -s $CxxSource $@ $OutputSpec -lQSS
