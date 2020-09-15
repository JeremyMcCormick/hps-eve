#ifndef HPS_EVENTOBJECTS_H_
#define HPS_EVENTOBJECTS_H_ 1

// HPS
#include "Verbosity.h"

// LCIO
#include "EVENT/LCEvent.h"

// ROOT
#include "TEveManager.h"
#include "TGeoManager.h"
#include "TStyle.h"
#include "TEveTrack.h"
#include "TDatabasePDG.h"

// LCIO
#include "EVENT/LCObject.h"
#include "EVENT/SimTrackerHit.h"

namespace hps {

    class DetectorGeometry;
    class EventDisplay;

    class EventObjects : public Verbosity {

        public:

            EventObjects(EventDisplay* app);

            virtual ~EventObjects();

            void build(TEveManager* manager, EVENT::LCEvent* event);

        private:

            TEveElementList* createSimTrackerHits(EVENT::LCCollection* coll);

            TEveElementList* createSimCalorimeterHits(EVENT::LCCollection* coll);

            TEveCompound* createMCParticles(EVENT::LCCollection* coll,
                                            EVENT::LCCollection* simTrackerHits);

            TEveElementList* createCalClusters(EVENT::LCCollection* coll);

            TEveElementList* createReconTracks(EVENT::LCCollection* coll);

            static void findSimTrackerHits(std::vector<EVENT::SimTrackerHit*>& list,
                                           EVENT::LCCollection* hits,
                                           EVENT::MCParticle* p);

            static TStyle createClusStyle();

        private:

            EventDisplay* app_;

            // Length cut for displaying particle trajectories in centimeters
            double lengthCut_{1.0};

            // TODO: momentum cut

            TDatabasePDG* pdgdb_;
    };
}

#endif
