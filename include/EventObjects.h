#ifndef HPS_EVENTOBJECTS_H_
#define HPS_EVENTOBJECTS_H_ 1

// LCIO
#include "EVENT/LCEvent.h"

// ROOT
#include "TEveManager.h"
#include "TGeoManager.h"
#include "TStyle.h"

// LCIO
#include "EVENT/LCObject.h"

namespace hps {

    class EventObjects {

        public:

            EventObjects(TGeoManager* geo);

            void build(TEveManager* manager, EVENT::LCEvent* event);

        private:

            TEveElementList* createSimTrackerHits(EVENT::LCCollection* coll);

            TEveElementList* createSimCalorimeterHits(EVENT::LCCollection* coll);

            TGeoManager* geo_;

            TStyle ecalStyle_;
    };
}

#endif
