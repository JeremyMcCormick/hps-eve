#include "EventObjects.h"

// LCIO
#include "EVENT/LCIO.h"
#include "EVENT/LCCollection.h"
#include "EVENT/SimTrackerHit.h"

// ROOT
#include "TEveElement.h"
#include "TEvePointSet.h"

using EVENT::LCIO;

namespace hps {

    EventObjects::EventObjects() : event_{nullptr} {

    }

    void EventObjects::setEvent(EVENT::LCEvent* event) {
        elements_.clear(); // Don't delete objects as Eve will do this for us.
        event_ = event;

    }

    void EventObjects::build() {
        auto collNames = event_->getCollectionNames();
        for (std::vector<std::string>::const_iterator it = collNames->begin();
                it != collNames->end();
                it++) {
            auto name = *it;
            EVENT::LCCollection* coll = event_->getCollection(name);
            TEveElementList* elements = nullptr;
            if (coll->getTypeName().compare(LCIO::SIMTRACKERHIT) == 0) {
                elements = convertSimTrackerHits(coll);
            }
            if (elements != nullptr) {
                elements->SetName(name.c_str());
                elements_.push_back(elements);
            }
        }
    }

    TEveElementList* EventObjects::convertSimTrackerHits(EVENT::LCCollection* coll) {
        std::cout << "Converting SimTrackerHit collection with size: " << coll->getNumberOfElements() << std::endl;
        TEveElementList* elements = new TEveElementList();
        for (int i=0; i<coll->getNumberOfElements(); i++) {
            EVENT::SimTrackerHit* hit = dynamic_cast<EVENT::SimTrackerHit*>(coll->getElementAt(i));
            auto x = hit->getPosition()[0];
            auto y = hit->getPosition()[1];
            auto z = hit->getPosition()[2];
            //auto dEdx = hit->getdEdx();
            //auto time = hit->getTime();
            TEvePointSet* p = new TEvePointSet(1);
            p->SetName("SimTrackerHit");
            p->SetMarkerStyle(3);
            p->SetPoint(0, 0.1*x, 0.1*y, 0.1*z);
            p->SetMarkerColor(3);
            elements->AddElement(p);
            // TODO: set title (see Druid src/TrackerHits.cc)
        }
        std::cout << "Done converting SimTrackerHit collection!" << std::endl;
        return elements;
    }

    const std::vector<TEveElement*>& EventObjects::getElementList() {
        return elements_;
    }
}
