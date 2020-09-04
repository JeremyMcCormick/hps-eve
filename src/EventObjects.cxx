#include "EventObjects.h"

// HPS
#include "DetectorGeometry.h"

// LCIO
#include "EVENT/LCIO.h"
#include "EVENT/LCCollection.h"
#include "EVENT/SimTrackerHit.h"
#include "EVENT/SimCalorimeterHit.h"

// ROOT
#include "TEveElement.h"
#include "TEvePointSet.h"
#include "TColor.h"

using EVENT::LCIO;

namespace hps {

    EventObjects::EventObjects(TGeoManager* geo, std::set<std::string> excludeColls) :
            geo_(geo), excludeColls_(excludeColls) {
        ecalStyle_.SetPalette(kThermometer);
    }

    void EventObjects::build(TEveManager* manager, EVENT::LCEvent* event) {
        std::cout << "[ EventObjects ] : Set new LCIO event with event num: " << event->getEventNumber() << std::endl;
        const std::vector<std::string>* collNames = event->getCollectionNames();
        for (std::vector<std::string>::const_iterator it = collNames->begin();
                it != collNames->end();
                it++) {
            auto name = *it;
            if (excludeCollection(name)) {
                std::cout << "[ EventObjects ] : Ignoring excluded collection: " << name << std::endl;
                continue;
            }
            EVENT::LCCollection* coll = event->getCollection(name);
            TEveElementList* elements = nullptr;
            auto typeName = coll->getTypeName();
            if (typeName == LCIO::SIMTRACKERHIT) {
                elements = createSimTrackerHits(coll);
            } else if (typeName == LCIO::SIMCALORIMETERHIT) {
                elements = createSimCalorimeterHits(coll);
            }
            if (elements != nullptr) {
                elements->SetName(name.c_str());
                std::cout << "[ EventObjects ] : Adding elements: " << name << std::endl;
                manager->AddElement(elements);
            }
        }
    }

    TEveElementList* EventObjects::createSimTrackerHits(EVENT::LCCollection* coll) {
        TEveElementList* elements = new TEveElementList();
        for (int i=0; i<coll->getNumberOfElements(); i++) {
            EVENT::SimTrackerHit* hit = dynamic_cast<EVENT::SimTrackerHit*>(coll->getElementAt(i));
            auto x = hit->getPosition()[0];
            auto y = hit->getPosition()[1];
            auto z = hit->getPosition()[2];
            auto dEdx = hit->getdEdx();
            auto hitTime = hit->getTime();
            TEvePointSet* p = new TEvePointSet(1);
            p->SetName("SimTrackerHit");
            p->SetMarkerStyle(kStar);
            p->SetMarkerSize(0.5);
            p->SetPoint(0, x/10.0, y/10.0, z/10.0);
            p->SetMarkerColor(3);
            p->SetTitle(Form("Simulated Tracker Hit\n"
                              "(x, y, z) = (%.3f, %.3f, %.3f)\n"
                              "Time = %f, dEdx = %E",
                              x, y, z, hitTime, dEdx));
            elements->AddElement(p);
        }
        return elements;
    }

    TEveElementList* EventObjects::createSimCalorimeterHits(EVENT::LCCollection* coll) {
        float min = 999;
        float max = -999;
        for (int i=0; i<coll->getNumberOfElements(); i++) {
            EVENT::SimCalorimeterHit* hit = dynamic_cast<EVENT::SimCalorimeterHit*>(coll->getElementAt(i));
            auto energy = hit->getEnergy();
            if (energy < min) {
                min = energy;
            }
            if (energy > max) {
                max = energy;
            }
        }
        std::cout << "[ EventObjects ] : ECAL min, max hit energy: " << min << ", " << max << std::endl;
        min = min * 100;
        max = max * 100;

        TEveElementList* elements = new TEveElementList();
        for (int i=0; i<coll->getNumberOfElements(); i++) {
            EVENT::SimCalorimeterHit* hit = dynamic_cast<EVENT::SimCalorimeterHit*>(coll->getElementAt(i));
            auto energy = hit->getEnergy();
            auto pos = hit->getPosition();
            auto x = pos[0]/10.0;
            auto y = pos[1]/10.0;
            auto z = pos[2]/10.0;
            auto hitTime = hit->getTimeCont(0);
            std::cout << "[ EventObjects ] : Looking for ECAL crystal at: ("
                    << x << ", " << y << ", " << z << ")" << std::endl;
            geo_->CdTop();
            TGeoNode* node = geo_->FindNode((double)x, (double)y, (double)z);
            if (node != nullptr) {
                std::cout << "[ EventObjects ] : Found geo node: " << node->GetName() << std::endl;
            }
            TEveElement* element = DetectorGeometry::toEveElement(geo_, node);
            auto energyScaled = energy * 100;
            element->SetMainColor(TColor::GetColorPalette((energyScaled - min)/(max - min) * gStyle->GetNumberOfColors()));
            element->SetElementTitle(Form("Simulated Calorimeter Hit\n"
                    "(x, y, z) = (%.3f, %.3f, %.3f)\n"
                    "Time = %f, Energy = %E, Contribs = %d",
                    x, y, z, hitTime, energy, hit->getNMCContributions()));
            elements->AddElement(element);
        }
        return elements;
    }

    bool EventObjects::excludeCollection(const std::string& collName) {
        return excludeColls_.find(collName) != excludeColls_.end();
    }

} /* namespace hps */
