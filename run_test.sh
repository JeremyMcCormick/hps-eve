#!/bin/sh

. install/bin/hps-eve-env.sh

event_file=/work/slac/data/lcio/hpsForward_e+_0.5GeV_z-7.5_0_SLIC-v06-00-01_QGSP_BERT_HPS-PhysicsRun2019-v2-4pt5_recon.slcio

./install/bin/hps-eve -g ./HPS-PhysicsRun2019-v2-4pt5.gdml -v 0 -b 1.034 -e HodoscopeHits -e TrackerHitsECal -e HodoscopePreprocessedHits $event_file
