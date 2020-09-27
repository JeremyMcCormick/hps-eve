#include "EventObjects.h"

// C++ standard library
#include <cmath>
#include <sstream>

// HPS
#include "DetectorGeometry.h"
#include "EventDisplay.h"
#include "EventObjects.h"
#include "LCObjectUserData.h"

// LCIO
#include "EVENT/LCIO.h"
#include "EVENT/LCCollection.h"
#include "EVENT/SimTrackerHit.h"
#include "EVENT/SimCalorimeterHit.h"
#include "EVENT/MCParticle.h"
#include "EVENT/Cluster.h"
#include "EVENT/Track.h"

// ROOT
#include "TEveElement.h"
#include "TEvePointSet.h"
#include "TEveCompound.h"
#include "TColor.h"
#include "TEveTrackPropagator.h"
#include "TEveVSDStructs.h"
#include "TEveTrack.h"
#include "TEveRGBAPalette.h"
#include "TGeoBBox.h"
#include "TEveTrans.h"

using EVENT::LCIO;

namespace hps {

    EventObjects::EventObjects(EventDisplay* app) :
            Logger("EventObjects"),
            app_(app),
            pdgdb_(TDatabasePDG::Instance()) {

        // Set log level from main app.
        setLogLevel(app_->getLogLevel());
    }

    void EventObjects::build(TEveManager* manager, EVENT::LCEvent* event) {
        log(INFO) << "Set new LCIO event: " << event->getEventNumber() << std::endl;

        // Clear the map of types to element lists.
        typeMap_.clear();

        EVENT::LCCollection* simTrackerHits = event->getCollection("TrackerHits");

        const std::vector<std::string>* collNames = event->getCollectionNames();
        for (std::vector<std::string>::const_iterator it = collNames->begin();
                it != collNames->end();
                it++) {
            auto name = *it;
            if (app_->excludeCollection(name)) {
                log(FINE) << "Ignoring excluded collection: " << name << std::endl;
                continue;
            }
            EVENT::LCCollection* coll = event->getCollection(name);
            TEveElementList* elements = nullptr;
            auto typeName = coll->getTypeName();
            if (typeName == LCIO::SIMTRACKERHIT) {
                elements = createSimTrackerHits(coll);
            } else if (typeName == LCIO::SIMCALORIMETERHIT) {
                elements = createSimCalorimeterHits(coll);
            } else if (typeName == LCIO::MCPARTICLE) {
                elements = createMCParticles(coll, simTrackerHits);
            } else if (typeName == LCIO::CLUSTER) {
                elements = createCalClusters(coll);
            } else if (typeName == LCIO::TRACK) {
                elements = createReconTracks(coll);
            }
            if (elements != nullptr) {
                elements->SetElementName(name.c_str());
                log(FINE) << "Adding elements: " << name << std::endl;
                elements->SetPickableRecursively(true);
                manager->AddElement(elements);

                log(FINE) << "Mapping element list to type: " << typeName << std::endl;

                typeMap_[typeName].push_back(elements);
            } /*else {
                if (checkVerbosity()) {
                    std::cerr << "[ EventObjects ] : Unhandled collection: " << name << ":" << typeName << std::endl;
                }
            }*/
        }

        // Filter MCParticle objects from current P cut (makes P cut work when hitting next event button).
        setPCut(pcut_);

        //TEveText* text = createEventText(event);
        //manager->AddElement(text);
    }

    EventObjects::~EventObjects() {
    }

    TEveElementList* EventObjects::createSimTrackerHits(EVENT::LCCollection* coll) {
        TEveElementList* elements = new TEveElementList();
        for (int i=0; i<coll->getNumberOfElements(); i++) {
            EVENT::SimTrackerHit* hit = dynamic_cast<EVENT::SimTrackerHit*>(coll->getElementAt(i));
            auto x = hit->getPosition()[0];
            auto y = hit->getPosition()[1];
            auto z = hit->getPosition()[2];
            auto edep = hit->getEDep();
            auto hitTime = hit->getTime();
            TEvePointSet* p = new TEvePointSet(1);
            p->SetElementName("SimTrackerHit");
            p->SetMarkerStyle(kStar);
            p->SetMarkerSize(0.2);
            p->SetPoint(0, x/10.0, y/10.0, z/10.0);
            p->SetMarkerColor(3);
            p->SetTitle(Form("Simulated Tracker Hit\n"
                              "(x, y, z) = (%.3f, %.3f, %.3f)\n"
                              "Time = %f, dEdx = %E",
                              x, y, z, hitTime, edep));
            elements->AddElement(p);
        }
        return elements;
    }

    TEveElementList* EventObjects::createSimCalorimeterHits(EVENT::LCCollection* coll) {

        TStyle ecalStyle;
        ecalStyle.SetPalette(kTemperatureMap);

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

        log(FINE) << "ECAL min, max hit energy: " << min << ", " << max << std::endl;

        //min = min * 100;
        min = 0;
        max = max * 100;

        TGeoManager* geo = app_->getDetectorGeometry()->getGeoManager();
        TEveElementList* elements = new TEveElementList();
        for (int i=0; i<coll->getNumberOfElements(); i++) {
            EVENT::SimCalorimeterHit* hit = dynamic_cast<EVENT::SimCalorimeterHit*>(coll->getElementAt(i));
            auto energy = hit->getEnergy();
            auto pos = hit->getPosition();
            auto x = pos[0]/10.0;
            auto y = pos[1]/10.0;
            auto z = pos[2]/10.0;
            auto hitTime = hit->getTimeCont(0);
            log(FINEST) << "Looking for ECAL crystal at: ("
                    << x << ", " << y << ", " << z << ")" << std::endl;

            geo->CdTop();
            TGeoNode* node = geo->FindNode((double)x, (double)y, (double)z);
            if (node != nullptr && node != geo->GetTopNode()) {
                log(FINEST) << "Found geo node: " << node->GetName() << std::endl;
            } else {
                log("No geo node found for cal hit!", ERROR);
                continue;
            }
            TEveElement* element = DetectorGeometry::toEveElement(geo, node);
            element->SetElementName("SimCalorimeterHit");

            auto energyScaled = energy * 100;
            element->SetMainColor(ecalStyle.GetColorPalette((energyScaled - min)/(max - min) * ecalStyle.GetNumberOfColors()));
            element->SetElementTitle(Form("Simulated Calorimeter Hit\n"
                    "(x, y, z) = (%.3f, %.3f, %.3f)\n"
                    "Time = %f, Energy = %E, Contribs = %d",
                    x, y, z, hitTime, energy, hit->getNMCContributions()));
            elements->AddElement(element);
        }
        return elements;
    }

    // Based on Druid src/BuildMCParticles.cc
    TEveElementList* EventObjects::createMCParticles(EVENT::LCCollection *coll,
                                                     EVENT::LCCollection *simTrackerHits) {

        TEveElementList* mcTracks = new TEveElementList();

        log(FINE) << "Building MCParticle collection with size: "
                << coll->getNumberOfElements() << std::endl;

        TEveTrackPropagator *propsetCharged = new TEveTrackPropagator();

        propsetCharged->SetMagFieldObj(new TEveMagFieldConst(0.0, app_->getMagFieldY(), 0.0));
        propsetCharged->SetDelta(0.01);
        propsetCharged->SetMaxR(150);
        propsetCharged->SetMaxZ(200);
        propsetCharged->SetMaxOrbs(2.0);
        //propsetCharged->SetRnrReferences(true);
        //propsetCharged->RefPMAtt().SetMarkerColor(kRed);
        //propsetCharged->RefPMAtt().SetMarkerStyle(kFullCircle);
        //propsetCharged->RefPMAtt().SetMarkerSize(1);

        TEveTrackPropagator *propsetNeutral = new TEveTrackPropagator();
        propsetNeutral->SetMagFieldObj(new TEveMagFieldConst(0.0, app_->getMagFieldY(), 0.0));
        propsetNeutral->SetDelta(0.01);
        propsetNeutral->SetMaxR(150);
        propsetNeutral->SetMaxZ(200);
        propsetNeutral->SetMaxOrbs(1.0);

        std::map<EVENT::MCParticle*, TEveTrack*> particleMap;

        for (int i = 0; i < coll->getNumberOfElements(); i++) {

            EVENT::MCParticle *mcp = dynamic_cast<EVENT::MCParticle*>(coll->getElementAt(i));

            float charge = mcp->getCharge();
            double energy = mcp->getEnergy();

            double px = mcp->getMomentum()[0];
            double py = mcp->getMomentum()[1];
            double pz = mcp->getMomentum()[2];

            double x = mcp->getVertex()[0]/10.0;
            double y = mcp->getVertex()[1]/10.0;
            double z = mcp->getVertex()[2]/10.0;

            double endX = mcp->getEndpoint()[0]/10.0;
            double endY = mcp->getEndpoint()[1]/10.0;
            double endZ = mcp->getEndpoint()[2]/10.0;

            TEveVector vertex(x, y, z);
            TEveVector endpoint(endX, endY, endZ);

            float length = vertex.Distance(endpoint);

            TParticlePDG* pdg = pdgdb_->GetParticle(mcp->getPDG());

            log(FINEST) << "Processing MCParticle: charge = " << charge
                    << "; vertex = (" << x << ", " << y << ", " << z
                    << "); " << "momentum = (" << px << ", " << py << ", "
                    << pz << "); " << "gen_status = " << mcp->getGeneratorStatus()
                    << std::endl;

            TEveRecTrack *recTrack = new TEveRecTrack();
            recTrack->fV.Set(TEveVector(x, y, z));
            recTrack->fP.Set(px, py, pz);
            recTrack->fSign = charge;

            TEveTrack *track = new TEveTrack(recTrack, nullptr);
            if (pdg) {
                track->SetElementName(pdg->GetName());
            } else {
                log(WARNING) << "Unknown PDG code: " << mcp->getPDG() << std::endl;
                track->SetElementName("Unknown");
            }
            if (charge != 0.0) {
                track->SetPropagator(propsetCharged);
                track->SetMainColor(kRed);
                //track->SetMarkerColor(kRed);
                //track->SetMarkerStyle(kFullCircle);
                //track->SetMarkerSize(1);
            } else {
                track->SetPropagator(propsetNeutral);
                track->SetMainColor(kYellow);
                //track->SetMarkerColor(kYellow);
                //track->SetMarkerStyle(kFullCircle);
                //track->SetMarkerSize(1);
            }

            TVector3 p(px, py, pz);

            track->SetElementTitle(Form("MC Particle\n"
                    "(x, y, z) = (%.3f, %.3f, %.3f)\n"
                    "(Px, Py, Pz) = (%.3f, %.3f, %.3f)\n"
                    "Charge = %.3f, Energy = %.3f, Length = %.3f\n"
                    "P = %.3f",
                    x, y, z,
                    px, py, pz,
                    charge, energy, length,
                    p.Mag()));

            particleMap[mcp] = track;

            if (pdg) {
                track->SetElementName(pdg->GetName());
            } else {
                log(WARNING) << "Unknown PDG code: " << mcp->getPDG() << std::endl;
                track->SetElementName("Unknown");
            }

            //TEvePathMark* pm1 = new TEvePathMark(TEvePathMark::kReference);
            //TEvePathMark* pm3 = new TEvePathMark(TEvePathMark::kDecay);

            /*
            std::vector<EVENT::SimTrackerHit*> particleHits;
            findSimTrackerHits(particleHits, simTrackerHits, p);
            if (particleHits.size() > 0) {
                if (verbose_) {
                    std::cout << "[ EventObjects ] Found hits for particle: " << particleHits.size() << std::endl;
                }
                for (std::vector<EVENT::SimTrackerHit*>::iterator it = particleHits.begin();
                        it != particleHits.end(); it++) {
                    auto simHit = *it;
                    TEveVector hit(
                            (float) simHit->getPosition()[0]/10.0,
                            (float) simHit->getPosition()[1]/10.0,
                            (float) simHit->getPosition()[2]/10.0);
                    if (verbose_) {
                        std::cout << "[ EventObjects ] Adding path mark at: ("
                                << hit.fX << ", " << hit.fY << ", " << hit.fZ << ")"
                                << std::endl;
                    }
                    TEvePathMark* pm = new TEvePathMark(TEvePathMark::kReference);
                    pm->fV.Set(hit);
                    track->AddPathMark(*pm);
                }
                track->SetRnrPoints(true);
            }
            */

            // TODO: Make the length cut into a GUI setting.
            if (length > this->lengthCut_) {
                track->MakeTrack(false);
            } else {
                log(FINEST) << "Skipping track with length: " << length << std::endl;
            }

            track->SetUserData(new MCParticleUserData(mcp, p.Mag()));
        }

        for (auto it = particleMap.begin(); it != particleMap.end(); it++) {
            MCParticle* particle = it->first;
            TEveTrack* track = it->second;
            if (particle->getParents().size() > 0) {
                auto fnd = particleMap.find(particle->getParents()[0]);
                if (fnd != particleMap.end()) {
                    TEveTrack* parent = fnd->second;
                    parent->AddElement(track);
                }
            } else {
                // Top-level particles with no parents.
                mcTracks->AddElement(track);
            }
        }

        mcTracks->SetRnrSelfChildren(true, true);

        log("Done building MCParticle collection!", FINE);

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

    TEveElementList* EventObjects::createCalClusters(EVENT::LCCollection* coll) {

        log(FINE) << "Creating clusters: " << coll->getNumberOfElements() << std::endl;

        TGeoManager* geo = app_->getDetectorGeometry()->getGeoManager();

        TEveElementList* elements = new TEveElementList();

        TStyle clusStyle = createClusStyle();
        int nColors = clusStyle.GetNumberOfColors();
        int currColor = 0;
        for (int i = 0; i < coll->getNumberOfElements(); i++) {

            if (currColor > (nColors - 1)) {
                currColor = 0;
            }

            TEveCompound* compound = new TEveCompound();
            compound->SetElementName("Cluster");

            EVENT::Cluster* clus = (EVENT::Cluster*)coll->getElementAt(i);
            float x = clus->getPosition()[0]/10.0;
            float y = clus->getPosition()[1]/10.0;
            float z = clus->getPosition()[2]/10.0;


            log(FINEST) << "Adding cluster at: ("
                    << x << "," << y << ", " << z << ")" << std::endl;

            TEvePointSet* p = new TEvePointSet(1);
            p->SetElementName("Cluster");
            p->SetMarkerStyle(kStar);
            p->SetMarkerSize(3.0);
            p->SetPoint(0, x, y, z);
            int color = clusStyle.GetColorPalette(currColor);
            p->SetMarkerColor(color);
            elements->AddElement(p);

            auto hits = clus->getCalorimeterHits();
            for (EVENT::CalorimeterHitVec::const_iterator it = hits.begin();
                    it != hits.end();
                    it++) {
                auto hit = *it;
                auto pos = hit->getPosition();
                auto x = pos[0]/10.0;
                auto y = pos[1]/10.0;
                auto z = pos[2]/10.0;
                geo->CdTop();
                TGeoNode* node = geo->FindNode((double)x, (double)y, (double)z);
                if (node != nullptr && node != geo->GetTopNode()) {
                    log(FINEST) << "Found geo node: " << node->GetName() << std::endl;
                } else {
                    // This could happen with a bad hit position.
                    log(ERROR) << "No geo node found for cal hit at: ("
                            << x << ", " << y << ", " << z << ")"
                            << std::endl;
                    continue;
                }
                TEveElement* element = DetectorGeometry::toEveElement(geo, node);
                element->SetElementName("CalorimeterHit");
                element->SetMainColor(color);
                p->AddElement(element);
            }
            ++currColor;
        }

        log("Done creating clusters!", FINE);

        return elements;
    }

    TEveElementList* EventObjects::createReconTracks(EVENT::LCCollection* coll) {

        static double fieldConversion = 2.99792458e-4;

        auto elements = new TEveElementList();

        float bY = this->app_->getMagFieldY();

        TEveTrackPropagator *propsetCharged = new TEveTrackPropagator();
        propsetCharged->SetMagFieldObj(new TEveMagFieldConst(0.0, bY, 0.0));
        propsetCharged->SetDelta(0.01);
        propsetCharged->SetMaxR(150);
        propsetCharged->SetMaxZ(200);
        propsetCharged->SetMaxOrbs(2.0);

        for (int i = 0; i < coll->getNumberOfElements(); i++) {

            auto track = (EVENT::Track*) coll->getElementAt(i);

            auto ts = track->getTrackState(EVENT::TrackState::AtIP);

            float omega = ts->getOmega();
            double pt = bY * fieldConversion / abs(omega);

            double px = pt * cos(ts->getPhi());
            double py = pt * sin(ts->getPhi());
            double pz = pt * ts->getTanLambda();

            TVector3 p(px, py, pz);
            p.RotateY(-(TMath::Pi() / 2));
            p.RotateZ(-(TMath::Pi() / 2));

            track->getChi2();

            auto refPoint = track->getReferencePoint();
            double charge = omega > 0. ? charge = -1 : charge = 1;

            log(FINEST) << "Making track with (px, py, pz) = ("
                    << px << ", " << py << ", " << pz << ")"
                    << std::endl;

            TEveRecTrack *recTrack = new TEveRecTrack();
            recTrack->fV.Set(TEveVector(refPoint[0], refPoint[1], refPoint[2]));
            recTrack->fP.Set(p);
            recTrack->fSign = charge;

            TEveTrack *eveTrack = new TEveTrack(recTrack, nullptr);
            eveTrack->SetElementName("Track");
            eveTrack->SetPropagator(propsetCharged);
            eveTrack->SetMainColor(kGreen);

            eveTrack->SetElementTitle(Form("Recon Track\n"
                    "(x, y, z) = (%.3f, %.3f, %.3f)\n"
                    "(Px, Py, Pz) = (%.3f, %.3f, %.3f)\n"
                    "Charge = %.3f, P = %.3f, Chi2 = %.3f",
                    refPoint[0], refPoint[1], refPoint[2],
                    p.X(), p.Y(), p.Z(), charge, p.Mag(),
                    track->getChi2()));

            eveTrack->MakeTrack();

            elements->AddElement(eveTrack);
        }

        return elements;
    }

    TStyle EventObjects::createClusStyle() {
        Int_t clusPalette[12];
        clusPalette[0] = kRed;
        clusPalette[1] = kGreen;
        clusPalette[2] = kBlue;
        clusPalette[3] = kYellow;
        clusPalette[4] = kMagenta;
        clusPalette[5] = kCyan;
        clusPalette[6] = kOrange;
        clusPalette[7] = kSpring;
        clusPalette[8] = kTeal;
        clusPalette[9] = kAzure;
        clusPalette[10] = kViolet;
        clusPalette[11] = kPink;
        TStyle clusStyle;
        clusStyle.SetPalette(12, clusPalette);
        return clusStyle;
    }

    TEveText* EventObjects::createEventText(EVENT::LCEvent* event) {
        auto bbox = (TGeoBBox*) app_->getDetectorGeometry()->getGeoManager()->GetTopVolume()->GetShape();
        double y = bbox->GetDY() / 2;
        std::stringstream ss;
        ss << event->getDetectorName() << " : " << event->getRunNumber() << " : " << event->getEventNumber();
        TEveText* test = new TEveText(ss.str().c_str());
        test->SetFontSize(16);
        test->PtrMainTrans()->SetPos(0, y, 0);
        return test;
    }

    void EventObjects::setPCut(double pcut) {

        pcut_ = pcut;

        log(INFO) << "Setting new MCParticle P cut: " << pcut << std::endl;

        const std::vector<TEveElementList*>& particleLists = getElementsByType(std::string(LCIO::MCPARTICLE));
        if (particleLists.size() > 0) {
            for (std::vector<TEveElementList*>::const_iterator it = particleLists.begin();
                    it != particleLists.end(); it++) {
                TEveElementList* particleList = *(it);
                setPCut(particleList);
            }
        }
    }

    void EventObjects::setPCut(TEveElement* element) {
        if (element->GetUserData() != nullptr) {
            MCParticleUserData* particleData = (MCParticleUserData*)(element->GetUserData());
            if (particleData != nullptr) {
                double p = particleData->p();
                if (p < pcut_) {
                    log(FINEST) << "Cutting particle with P: " << p << std::endl;
                    element->SetRnrSelf(false);
                }
            }
        }
        for (TEveElement::List_i it = element->BeginChildren();
                it != element->EndChildren(); it++) {
            TEveTrack* track = dynamic_cast<TEveTrack*>(*(it));
            setPCut(track);
        }
    }

    const std::vector<TEveElementList*> EventObjects::getElementsByType(const std::string& typeName) {
        return typeMap_[typeName];
    }

} /* namespace hps */
