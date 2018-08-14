#!/bin/bash
# GCC Build Release

if [ -f $1.cc ]; then
  CxxMainSource=$1.cc
  OutputSpec="-o $1"
  shift
fi

g++ -pipe -std=c++11 -pedantic -Wall -Wextra -m64 -march=native -DNDEBUG -Ofast -fno-stack-protector -finline-limit=1000 -s $CxxMainSource $@ $OutputSpec -lQSS
