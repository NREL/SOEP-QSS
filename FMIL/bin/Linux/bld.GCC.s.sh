#!/bin/bash
# FMI Library Build (from clean build directory)

FMIL_SRC_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"/../../custom
FMIL_INS_DIR=/opt/FMIL.GCC.s

# -DCMAKE_C_FLAGS_RELWITHDEBINFO="-march=native -DNDEBUG -O3 -fno-stack-protector -fno-omit-frame-pointer -ggdb"
sudo cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DFMILIB_INSTALL_PREFIX=$FMIL_INS_DIR $FMIL_SRC_DIR
sudo make -j8 VERBOSE=1 install

__dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source $__dir/hdr.sh $FMIL_INS_DIR
