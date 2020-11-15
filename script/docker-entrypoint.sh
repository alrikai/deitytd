#!/bin/bash
set -e

DTDDIR="/libdtd"

#pushd ${DTDDIR}

#rm -rf build >> /dev/null
#mkdir -p build
#pushd build
#cmake ..
#make -j4
#popd

#poetry build
#cd "$DTDDIR/Bindings"
#python3.8 setup.py develop
#cd "$DTDDIR"

export PYTHONPATH=$PYTHONPATH:"$DTDDIR"
exec "$@"
