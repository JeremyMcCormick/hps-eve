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
#include "TEveText.h"

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

            void setPCut(double pcut);

        private:

            TEveElementList* createSimTrackerHits(EVENT::LCCollection* coll);

            TEveElementList* createSimCalorimeterHits(EVENT::LCCollection* coll);

            TEveElementList* createMCParticles(EVENT::LCCollection *coll,
                                               EVENT::LCCollection *simTrackerHits);

            TEveElementList* createCalClusters(EVENT::LCCollection* coll);

            TEveElementList* createReconTracks(EVENT::LCCollection* coll);

            TEveText* createEventText(EVENT::LCEvent* event);

            static void findSimTrackerHits(std::vector<EVENT::SimTrackerHit*>& list,
                                           EVENT::LCCollection* hits,
                                           EVENT::MCParticle* p);

            static TStyle createClusStyle();

            const std::vector<TEveElementList*> getElementsByType(const std::string& typeName);

            /**
             * Recursively process a set of TEveTrack objects with associated MCParticle user data
             * to apply a P cut.
             */
            void setPCut(TEveElement* element);

        private:

            EventDisplay* app_;

            // Length cut for displaying particle trajectories in centimeters (hard-coded for now)
            double lengthCut_{1.0};

            // P cut for MCParticles which can be set in GUI
            double pcut_{0.0};

            // Map of LCIO types to Eve element lists.
            std::map<std::string, std::vector<TEveElementList*>> typeMap_;

            TDatabasePDG* pdgdb_;
    };
}

#endif
