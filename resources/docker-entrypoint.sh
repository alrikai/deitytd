#!/bin/bash
set -e

DTDDIR="/deitytd"
source activate deitytd-env
cd "$DTDDIR" && mkdir -p build
cd build && cmake ..
make -j4
cd "$DTDDIR/Bindings"
python setup.py develop
cd "$DTDDIR"

export PYTHONPATH=$PYTHONPATH:"$DTDDIR/build/lib"
exec "$@"
