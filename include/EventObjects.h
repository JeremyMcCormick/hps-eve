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

    class EventObjects : public Verbosity {

        public:

            EventObjects(DetectorGeometry* det,
                         std::set<std::string> excludeColls,
                         double bY);

            virtual ~EventObjects();

            void build(TEveManager* manager, EVENT::LCEvent* event);

            bool excludeCollection(const std::string& collName);

        private:

            TEveElementList* createSimTrackerHits(EVENT::LCCollection* coll);

            TEveElementList* createSimCalorimeterHits(EVENT::LCCollection* coll);

            TEveCompound* createMCParticles(EVENT::LCCollection* coll,
                                            EVENT::LCCollection* simTrackerHits);

            static void findSimTrackerHits(std::vector<EVENT::SimTrackerHit*>& list,
                                           EVENT::LCCollection* hits,
                                           EVENT::MCParticle* p);

        private:

            //TGeoManager* geo_;
            DetectorGeometry* det_;

            std::set<std::string> excludeColls_;

            TStyle ecalStyle_;

            // Fixed magnetic field Y component.
            double bY_;
    };
}

#endif
