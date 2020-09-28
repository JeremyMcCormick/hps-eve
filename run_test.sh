#!/bin/sh

if [ ! -n "$ROOTSYS" ]; then
    echo "ERROR: ROOTSYS not set!" 
    exit 1
fi

if [ -z "$1" ]; then
    echo "ERROR: Missing at least one LCIO file"
    exit 1
fi

. install/bin/hps-eve-env.sh
echo $LD_LIBRARY_PATH

./install/bin/hps-eve -l 6 -b 1.034 -e HodoscopeHits -e TrackerHitsECal -e HodoscopePreprocessedHits $@
