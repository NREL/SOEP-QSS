#!/bin/bash

if [ -z "$1" ] ; then
  echo "Error: No argument passed to hdr.sh"
  exit 1
fi

FMIL_SRC_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"/../../custom
FMIL_INS_DIR=$1

sudo mkdir $FMIL_INS_DIR/include/FMI >/dev/null 2>&1
sudo mkdir $FMIL_INS_DIR/include/FMI1 >/dev/null 2>&1
sudo mkdir $FMIL_INS_DIR/include/FMI2 >/dev/null 2>&1
sudo mkdir $FMIL_INS_DIR/include/JM >/dev/null 2>&1
sudo mkdir $FMIL_INS_DIR/include/src >/dev/null 2>&1
sudo mkdir $FMIL_INS_DIR/include/src/FMI >/dev/null 2>&1
sudo mkdir $FMIL_INS_DIR/include/src/FMI2 >/dev/null 2>&1

sudo cp $FMIL_SRC_DIR/src/CAPI/include/FMI2/fmi2_capi.h $FMIL_INS_DIR/include/FMI2
sudo cp $FMIL_SRC_DIR/src/CAPI/src/FMI2/fmi2_capi_impl.h $FMIL_INS_DIR/include/src/FMI2
sudo cp $FMIL_SRC_DIR/src/Import/src/FMI/fmi_import_context_impl.h $FMIL_INS_DIR/include/FMI
sudo cp $FMIL_SRC_DIR/src/Import/src/FMI2/fmi2_import_impl.h $FMIL_INS_DIR/include/FMI2
sudo cp $FMIL_SRC_DIR/src/Util/src/FMI/fmi_util_options_impl.h $FMIL_INS_DIR/include/src/FMI
sudo cp $FMIL_SRC_DIR/src/XML/include/FMI/*.h $FMIL_INS_DIR/include/FMI
sudo cp $FMIL_SRC_DIR/src/XML/include/FMI1/*.h $FMIL_INS_DIR/include/FMI1
sudo cp $FMIL_SRC_DIR/src/XML/include/FMI2/*.h $FMIL_INS_DIR/include/FMI2
sudo cp $FMIL_SRC_DIR/ThirdParty/Expat/expat-2.4.8/lib/expat*.h $FMIL_INS_DIR/include
