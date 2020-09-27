#!/bin/sh

if [ ! -n "$ROOTSYS" ]; then
    echo "ERROR: ROOTSYS not set!" 
    exit 1
fi

#. /work/slac/sw/cernroot/root-6.18.04-install/bin/thisroot.sh
. install/bin/hps-eve-env.sh
echo $LD_LIBRARY_PATH

event_file=/work/slac/data/lcio/hpsForward_e+_0.5GeV_z-7.5_0_SLIC-v06-00-01_QGSP_BERT_HPS-PhysicsRun2019-v2-4pt5_recon.slcio
#event_file=/work/slac/data/lcio/ap_1-0-100.slcio
#event_file=/work/slac/git/hps-projects/projects/mcparticle-hit-filter/ap_filtered.slcio
#event_file=/work/slac/data/lcio/hpsForward_e-_4.5GeV_z-7.5_0_SLIC-v06-00-01_QGSP_BERT_HPS-PhysicsRun2019-v2-4pt5_recon-0-100.slcio
#event_file=/work/slac/git/hps-projects/projects/mcparticle-hit-filter/ap_filtered.slcio

# Run with explicitly provided GDML file e.g. for offline or without curl and libxml2 enabled
#./install/bin/hps-eve -g ./HPS-PhysicsRun2019-v2-4pt5.gdml -v 0 -b 1.034 -e HodoscopeHits -e TrackerHitsECal -e HodoscopePreprocessedHits $event_file

# When GDML file is not provided, an LCDD file will be downloaded and GDML extracted from it.
./install/bin/hps-eve -v 6 -b 1.034 -e HodoscopeHits -e TrackerHitsECal -e HodoscopePreprocessedHits $event_file
