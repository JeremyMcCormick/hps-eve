#ifndef HPS_EVENTOBJECTS_H_
#define HPS_EVENTOBJECTS_H_ 1

// LCIO
#include "EVENT/LCEvent.h"

// ROOT
#include "TEveManager.h"

// LCIO
#include "EVENT/LCObject.h"

namespace hps {

    class EventObjects {

        public:

            EventObjects();

            void setEvent(EVENT::LCEvent* event);

            void add(TEveManager* manager);

            void build();

            const std::vector<TEveElement*>& getElementList();

        private:

            TEveElementList* convertSimTrackerHits(EVENT::LCCollection* coll);

            EVENT::LCEvent* event_;
            std::vector<TEveElement*> elements_;
    };
}

#endif
