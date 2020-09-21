#include "EventObjects.h"

// C++ standard library
#include <cmath>
#include <sstream>

// HPS
#include "DetectorGeometry.h"
#include "EventDisplay.h"
#include "EventObjects.h"

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
            app_(app),
            pdgdb_(TDatabasePDG::Instance()) {
    }

    void EventObjects::build(TEveManager* manager, EVENT::LCEvent* event) {
        if (checkVerbosity()) {
            std::cout << "[ EventObjects ] Set new LCIO event: " << event->getEventNumber() << std::endl;
        }

        EVENT::LCCollection* simTrackerHits = event->getCollection("TrackerHits");

        const std::vector<std::string>* collNames = event->getCollectionNames();
        for (std::vector<std::string>::const_iterator it = collNames->begin();
                it != collNames->end();
                it++) {
            auto name = *it;
            if (app_->excludeCollection(name)) {
                if (checkVerbosity(2)) {
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
            } else if (typeName == LCIO::CLUSTER) {
                elements = createCalClusters(coll);
            } else if (typeName == LCIO::TRACK) {
                elements = createReconTracks(coll);
            }
            if (elements != nullptr) {
                elements->SetElementName(name.c_str());
                if (checkVerbosity(2)) {
                    std::cout << "[ EventObjects ] Adding elements: " << name << std::endl;
                }
                elements->SetPickableRecursively(true);
                manager->AddElement(elements);
            }
        }

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
        if (checkVerbosity(2)) {
            std::cout << "[ EventObjects ] ECAL min, max hit energy: " << min << ", " << max << std::endl;
        }
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
            if (checkVerbosity(4)) {
                std::cout << "[ EventObjects ] Looking for ECAL crystal at: ("
                        << x << ", " << y << ", " << z << ")" << std::endl;
            }
            geo->CdTop();
            TGeoNode* node = geo->FindNode((double)x, (double)y, (double)z);
            if (node != nullptr) {
                if (checkVerbosity(4)) {
                    std::cout << "[ EventObjects ] Found geo node: " << node->GetName() << std::endl;
                }
            } else {
                std::cerr << "[ EventObjects ] [ ERROR ] No geo node found for cal hit!" << std::endl;
                continue;
            }
            TEveElement* element = DetectorGeometry::toEveElement(geo, node);

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
    TEveCompound* EventObjects::createMCParticles(EVENT::LCCollection *coll,
                                                  EVENT::LCCollection *simTrackerHits) {

        if (checkVerbosity(2)) {
            std::cout << "[ EventObjects ] Building MCParticle collection with size: "
                << coll->getNumberOfElements() << std::endl;
        }

        TEveCompound *mcTracks = new TEveCompound();
        //TEveTrackList* mcTracks = new TEveTrackList();
        //mcTracks->SetMainColor(kRed);
        mcTracks->OpenCompound();

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

        std::map<EVENT::MCParticle*, TEveCompound*> particleMap;

        for (int i = 0; i < coll->getNumberOfElements(); i++) {

            TEveCompound *compound = new TEveCompound();
            compound->OpenCompound();

            EVENT::MCParticle *mcp = dynamic_cast<EVENT::MCParticle*>(coll->getElementAt(i));
            particleMap[mcp] = compound;

            float charge = mcp->getCharge();
            double energy = mcp->getEnergy();

            double px = mcp->getMomentum()[0];
            double py = mcp->getMomentum()[1];
            double pz = mcp->getMomentum()[2];
            double p = sqrt(px*px + py*py + pz*pz);

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

            if (checkVerbosity(4)) {
                std::cout << "[ EventObjects ] Processing MCParticle: charge = " << charge
                        << "; vertex = (" << x << ", " << y << ", " << z
                        << "); " << "momentum = (" << px << ", " << py << ", "
                        << pz << "); " << "gen_status = " << mcp->getGeneratorStatus()
                        << std::endl;
            }

            TEveRecTrack *recTrack = new TEveRecTrack();
            recTrack->fV.Set(TEveVector(x, y, z));
            recTrack->fP.Set(px, py, pz);
            recTrack->fSign = charge;

            TEveTrack *track = new TEveTrack(recTrack, nullptr);
            if (charge != 0.0) {
                if (checkVerbosity(4)) {
                    std::cout << "[ EventObjects ] Adding charged track" << std::endl;
                }
                track->SetPropagator(propsetCharged);
                track->SetMainColor(kRed);
                //track->SetMarkerColor(kRed);
                //track->SetMarkerStyle(kFullCircle);
                //track->SetMarkerSize(1);
            } else {
                if (checkVerbosity(4)) {
                    std::cout << "[ EventObjects ] Adding neutral track" << std::endl;
                }
                track->SetPropagator(propsetNeutral);
                track->SetMainColor(kYellow);
                //track->SetMarkerColor(kYellow);
                //track->SetMarkerStyle(kFullCircle);
                //track->SetMarkerSize(1);
            }

            track->SetElementTitle(Form("MC Particle\n"
                    "(x, y, z) = (%.3f, %.3f, %.3f)\n"
                    "(Px, Py, Pz) = (%.3f, %.3f, %.3f)\n"
                    "Charge = %.3f, Energy = %.3f, Length = %.3f\n"
                    "P = %.3f",
                    x, y, z,
                    px, py, pz,
                    charge, energy, length,
                    p));

            if (pdg) {
                compound->SetElementName(pdg->GetName());
            } else {
                std::cerr << "[ EventObjects ] [ ERROR ] Unknown PDG code: " << mcp->getPDG() << std::endl;
                compound->SetElementName("Unknown");
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

            if (length > this->lengthCut_) {
                track->MakeTrack(false);
            } else {
                if (checkVerbosity(4)) {
                    std::cout << "[ EventObjects] Skipping MakeTrack() for particle with path length: " << length << std::endl;
                }
            }

            compound->AddElement(track);
            compound->CloseCompound();
        }

        for (auto it = particleMap.begin(); it != particleMap.end(); it++) {
            auto p = it->first;
            auto c = it->second;
            if (p->getParents().size() > 0) {
                auto fnd = particleMap.find(p->getParents()[0]);
                if (fnd != particleMap.end()) {
                    auto parCompound = fnd->second;
                    parCompound->OpenCompound();
                    parCompound->AddElement(c);
                    parCompound->CloseCompound();
                } else {
                    std::cerr << "[ EventObjects ] [ ERROR ] Failed to find parent compound object!" << std::endl;
                }
            } else {
                // Top-level particles with no parents.
                mcTracks->AddElement(c);
            }
        }

        mcTracks->SetRnrSelfChildren(true, true);
        mcTracks->CloseCompound();

        if (checkVerbosity(2)) {
            std::cout << "[ EventObjects ] Done building MCParticle collection!" << std::endl;
        }
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

        if (checkVerbosity(2)) {
            std::cout << "[ EventObjects ] Creating clusters: " << coll->getNumberOfElements() << std::endl;
        }

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

            if (checkVerbosity(4)) {
                std::cout << "[ EventObjects ] Adding cluster at: ("
                        << x << "," << y << ", " << z << ")" << std::endl;
            }

            TEvePointSet* p = new TEvePointSet(1);
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
                if (node != nullptr) {
                    if (checkVerbosity(4)) {
                        std::cout << "[ EventObjects ] Found geo node: " << node->GetName() << std::endl;
                    }
                } else {
                    std::cerr << "[ EventObjects ] [ ERROR ] No geo node found for cal hit at: ("
                            << x << ", " << y << ", " << z << ")"
                            << std::endl;
                    continue;
                }
                TEveElement* element = DetectorGeometry::toEveElement(geo, node);
                element->SetMainColor(color);
                p->AddElement(element);
            }
            ++currColor;
        }

        if (checkVerbosity(2)) {
            std::cout << "[ Event Objects ] Done creating clusters!" << std::endl;
        }

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

        // TODO: rotate momentum 90 deg clockwise around Y to display correctly...
        // https://root.cern/root/html606/classTRotation.html
        // https://root.cern.ch/root/html534/guides/users-guide/PhysicsVectors.html

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

            std::cout << "[ EventObjects ] Making track with (px, py, pz) = ("
                    << px << ", " << py << ", " << pz << ")"
                    << std::endl;

            TEveRecTrack *recTrack = new TEveRecTrack();
            recTrack->fV.Set(TEveVector(refPoint[0], refPoint[1], refPoint[2]));
            //recTrack->fP.Set(px, py, pz);
            recTrack->fP.Set(p);
            recTrack->fSign = charge;

            TEveTrack *eveTrack = new TEveTrack(recTrack, nullptr);
            eveTrack->SetPropagator(propsetCharged);
            eveTrack->SetMainColor(kGreen);

            /*
            eveTrack->SetElementTitle(Form("Recon Track\n"
                    "(x, y, z) = (%.3f, %.3f, %.3f)\n"
                    "(Px, Py, Pz) = (%.3f, %.3f, %.3f)\n"
                    "Charge = %.3f, Energy = %.3f, Length = %.3f\n"
                    "P = %.3f",
                    x, y, z,
                    px, py, pz,
                    charge, energy, length,
                    p));
            */

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

} /* namespace hps */
