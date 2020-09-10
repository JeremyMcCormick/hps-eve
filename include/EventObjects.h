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

            static void findSimTrackerHits(std::vector<EVENT::SimTrackerHit*>& list,
                                           EVENT::LCCollection* hits,
                                           EVENT::MCParticle* p);

        private:

            EventDisplay* app_;

            TStyle ecalStyle_;
    };
}

#endif
