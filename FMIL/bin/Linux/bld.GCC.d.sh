#!/bin/bash
# FMI Library Build (from clean build directory)

FMIL_SRC_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"/../../custom
FMIL_INS_DIR=/opt/FMIL.GCC.d

if [[ -f /usr/bin/nproc ]]; then
	ntask=$(/usr/bin/nproc)
else
	ntask=8
fi

# -DCMAKE_C_FLAGS_DEBUG="-march=native -O0 -ffloat-store -fsignaling-nans -ftrapv -ggdb"
sudo cmake -DFMILIB_INSTALL_PREFIX=$FMIL_INS_DIR $FMIL_SRC_DIR -DCMAKE_BUILD_TYPE=Debug
sudo make -j${ntask} VERBOSE=1 install

__dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source $__dir/hdr.sh $FMIL_INS_DIR
