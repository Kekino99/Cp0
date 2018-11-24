#!/usr/bin/env sh

INIT_WD=`pwd`

cd "`dirname \"${0}\"`"
SCRIPT_DIR=`pwd`

cd "${SCRIPT_DIR}/solvers/glucose-syrup-4.1/simp"

make clean
make r

mv glucose_release "${SCRIPT_DIR}/glucose"

