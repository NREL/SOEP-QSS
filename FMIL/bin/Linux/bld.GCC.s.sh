#!/bin/bash
# FMI Library Build (from clean build directory)

FMIL_SRC_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"/../../custom
FMIL_INS_DIR=/opt/FMIL.GCC.s

if [[ -f /usr/bin/nproc ]]; then
	ntask=$(/usr/bin/nproc)
else
	ntask=8
fi

# -DCMAKE_C_FLAGS_RELWITHDEBINFO="-march=native -DNDEBUG -O3 -ffloat-store -fno-stack-protector -finline-limit=2000 -fno-omit-frame-pointer -ggdb"
sudo cmake -DFMILIB_INSTALL_PREFIX=$FMIL_INS_DIR $FMIL_SRC_DIR -DCMAKE_BUILD_TYPE=RelWithDebInfo
sudo make -j${ntask} VERBOSE=1 install

__dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source $__dir/hdr.sh $FMIL_INS_DIR
