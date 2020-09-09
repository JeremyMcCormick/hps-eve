. /work/slac/sw/cernroot/root-6.18.04-install/bin/thisroot.sh
export LD_LIBRARY_PATH=$PWD/install/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/work/slac/git/lcio-current/install/lib:$LD_LIBRARY_PATH

./install/bin/hps-eve -g ./HPS-PhysicsRun2019-v1-4pt5.gdml -v 1 -b 1.034 -e HodoscopeHits -e TrackerHitsECal ./events.slcio
