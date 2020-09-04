#include "EventObjects.h"

// HPS
#include "DetectorGeometry.h"

// LCIO
#include "EVENT/LCIO.h"
#include "EVENT/LCCollection.h"
#include "EVENT/SimTrackerHit.h"
#include "EVENT/SimCalorimeterHit.h"
#include "EVENT/MCParticle.h"

// ROOT
#include "TEveElement.h"
#include "TEvePointSet.h"
#include "TEveCompound.h"
#include "TColor.h"
#include "TEveTrackPropagator.h"
#include "TEveVSDStructs.h"
#include "TEveTrack.h"

using EVENT::LCIO;

namespace hps {

    EventObjects::EventObjects(TGeoManager* geo, std::set<std::string> excludeColls, double bY, int verbose) :
            geo_(geo), excludeColls_(excludeColls), verbose_(verbose), bY_(bY) {
        ecalStyle_.SetPalette(kThermometer);
    }

    void EventObjects::build(TEveManager* manager, EVENT::LCEvent* event) {
        if (verbose_) {
            std::cout << "[ EventObjects ] Set new LCIO event with event num: " << event->getEventNumber() << std::endl;
        }

        EVENT::LCCollection* simTrackerHits = event->getCollection("TrackerHits");

        const std::vector<std::string>* collNames = event->getCollectionNames();
        for (std::vector<std::string>::const_iterator it = collNames->begin();
                it != collNames->end();
                it++) {
            auto name = *it;
            if (excludeCollection(name)) {
                if (verbose_) {
                    std::cout << "[ EventObjects ] Ignoring excluded collection: " << name << std::endl;
                }
                continue;
            }
            EVENT::LCCollection* coll = event->getCollection(name);
            TEveElement* elements = nullptr;
            auto typeName = coll->getTypeName();
            if (typeName == LCIO::SIMTRACKERHIT) {
                elements = createSimTrackerHits(coll);
            } else if (typeName == LCIO::SIMCALORIMETERHIT) {
                elements = createSimCalorimeterHits(coll);
            } else if (typeName == LCIO::MCPARTICLE) {
                elements = createMCParticles(coll, simTrackerHits);
            }
            if (elements != nullptr) {
                elements->SetElementName(name.c_str());
                if (verbose_) {
                    std::cout << "[ EventObjects ] Adding elements: " << name << std::endl;
                }
                manager->AddElement(elements);
            }
        }
    }

    bool EventObjects::excludeCollection(const std::string& collName) {
        return excludeColls_.find(collName) != excludeColls_.end();
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
        if (verbose_) {
            std::cout << "[ EventObjects ] ECAL min, max hit energy: " << min << ", " << max << std::endl;
        }
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
            if (verbose_) {
                std::cout << "[ EventObjects ] Looking for ECAL crystal at: ("
                        << x << ", " << y << ", " << z << ")" << std::endl;
            }
            geo_->CdTop();
            TGeoNode* node = geo_->FindNode((double)x, (double)y, (double)z);
            if (node != nullptr) {
                if (verbose_) {
                    std::cout << "[ EventObjects ] Found geo node: " << node->GetName() << std::endl;
                }
            } else {
                std::cerr << "[ EventObjects ] No geo node found!" << std::endl;
                continue;
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

    // REF: Druid src/BuildMCParticles.cc
    TEveCompound* EventObjects::createMCParticles(EVENT::LCCollection* coll,
                                                  EVENT::LCCollection* simTrackerHits) {

        TEveCompound *mcTracks = new TEveCompound();
        mcTracks->SetMainColor(kRed);
        mcTracks->OpenCompound();

        TEveTrackPropagator* propsetCharged = new TEveTrackPropagator();
        std::cout << "[ EventObjects ] Setting up mag field with bY: " << bY_ << std::endl;
        propsetCharged->SetMagFieldObj(new TEveMagFieldConst(0.0, bY_, 0.0));
        propsetCharged->SetDelta(0.01); // Step size

        propsetCharged->SetMaxR(150);
        propsetCharged->SetMaxZ(200);
        propsetCharged->SetMaxOrbs(10.0);

        propsetCharged->RefPMAtt().SetMarkerColor(kYellow);
        propsetCharged->RefPMAtt().SetMarkerStyle(kCircle);
        propsetCharged->RefPMAtt().SetMarkerSize(1.0);

        for (int i=0; i<coll->getNumberOfElements(); i++) {
            EVENT::MCParticle* p = dynamic_cast<EVENT::MCParticle*>(coll->getElementAt(i));
            float charge = p->getCharge();
            //const double* endpoint = p->getEndpoint();
            const double* momentum = p->getMomentum();
            const double* vertex = p->getVertex();

            double px = momentum[0];
            double py = momentum[1];
            double pz = momentum[2];
            double x = vertex[0];
            double y = vertex[1];
            double z = vertex[2];
            // Only look at charged gen particles for now...
            if (p->getGeneratorStatus() && charge != 0.0) {
                if (verbose_) {
                    std::cout << "[ EventObjects ] Processing MCParticle: charge = " << charge
                            << "; vertex = (" << vertex[0] << ", " << vertex[1] << ", " << vertex[3] << "); "
                            << "momentum = (" << momentum[0] << ", " << momentum[1] << ", " << momentum[3] << ")"
                            << "gen_status = " << p->getGeneratorStatus()
                            << std::endl;
                }

                TEveRecTrack* chargedTrack = new TEveRecTrack();
                chargedTrack->fV.Set(TEveVector(x, y, z));
                chargedTrack->fP.Set(px, py, pz);
                chargedTrack->fSign = charge;

                TEveTrack* track = new TEveTrack(chargedTrack, propsetCharged);
                track->SetMainColor(kRed);

                // Adding path marks commented out for now
                /*
                TEvePathMark* pm1 = new TEvePathMark(TEvePathMark::kReference);
                TEvePathMark* pm2 = new TEvePathMark(TEvePathMark::kReference);
                TEvePathMark* pm3 = new TEvePathMark(TEvePathMark::kDecay);


                std::vector<EVENT::SimTrackerHit*> particleHits;
                findSimTrackerHits(particleHits, simTrackerHits, p);
                if (particleHits.size() > 0) {
                    std::cout << "[ EventObjects ] : Found hits for particle: " << particleHits.size() << std::endl;
                    for (std::vector<EVENT::SimTrackerHit*>::iterator it = particleHits.begin();
                            it != particleHits.end();
                            it++) {
                        auto particleHit = *it;
                        const double* hitPosition = particleHit->getPosition();
                        TEveVector setHit(
                                (float) hitPosition[0]/10.0,
                                (float) hitPosition[1]/10.0,
                                (float) hitPosition[2]/10.0);
                        pm1->fV.Set(setHit);
                        track->AddPathMark(*pm1);
                    }
                }
                */
                track->SetRnrSelfChildren(true, true);
                mcTracks->AddElement(track);

                if (verbose_) {
                    std::cout << "[ EventObjects ] Done creating track!" << std::endl;
                }
            }
        }
        mcTracks->CloseCompound();
        mcTracks->SetRnrSelfChildren(true, true);
        return mcTracks;
    }

    void EventObjects::findSimTrackerHits(std::vector<EVENT::SimTrackerHit*>& list,
                                          EVENT::LCCollection* hits,
                                          EVENT::MCParticle* p) {
        int nhits = hits->getNumberOfElements();
        for (int i=0; i<nhits; i++) {
            EVENT::SimTrackerHit* hit = dynamic_cast<EVENT::SimTrackerHit*>(hits->getElementAt(i));
            if (hit->getMCParticle() == p) {
                list.push_back(hit);
            }
        }
    }

} /* namespace hps */
