#ifndef HPS_EVENTOBJECTS_H_
#define HPS_EVENTOBJECTS_H_ 1

// HPS
#include "EVENT/LCEvent.h"

// ROOT
#include "TEveManager.h"
#include "TStyle.h"
#include "TDatabasePDG.h"
#include "TEveTrack.h"

// LCIO
#include "EVENT/LCObject.h"
#include "EVENT/SimTrackerHit.h"
#include "Logger.h"

namespace hps {

    class DetectorGeometry;
    class EventDisplay;

    class EventObjects : public Logger {

        public:

            EventObjects(EventDisplay* app);

            virtual ~EventObjects();

            void build(TEveManager* manager, EVENT::LCEvent* event);

            void setMCPCut(double cut);

            void setTrackPCut(double cut);

            void setChi2Cut(double cut);

        private:

            TEveElementList* createSimTrackerHits(EVENT::LCCollection*);

            TEveElementList* createSimCalorimeterHits(EVENT::LCCollection*);

            TEveElementList* createMCParticles(EVENT::LCCollection*);

            TEveElementList* createCalClusters(EVENT::LCCollection*);

            TEveElementList* createReconTracks(EVENT::LCCollection*);

            TEveElementList* createTrackerHits(EVENT::LCCollection*);

            TEveElementList* createReconstructedParticles(EVENT::LCCollection*);

            TEveElementList* createVertices(EVENT::LCCollection*);

            /*
            static void findSimTrackerHits(std::vector<EVENT::SimTrackerHit*>& list,
                                           EVENT::LCCollection* hits,
                                           EVENT::MCParticle* p);
            */

            static TStyle createClusStyle();

            // static TStyle createParticleStyle();

            const std::vector<TEveElementList*> getElementsByType(const std::string& typeName);

            /**
             * Recursively process a set of TEveTrack objects to apply a P cut.
             */
            void applyPCut(TEveElement* element, double& cut);

            void applyChi2Cut(TEveElementList* trackList);

        private:

            EventDisplay* app_;

            // P cut for MCParticles
            double mcPCut{0.0};

            // P cut for Recon Tracks
            double trackPCut{0.0};

            // chi2 cut for Recon Tracks
            double chi2Cut_{9999.0};

            // Map of LCIO types to Eve element lists.
            std::map<std::string, std::vector<TEveElementList*>> typeMap_;

            TDatabasePDG* pdgdb_;
    };
}

#endif
