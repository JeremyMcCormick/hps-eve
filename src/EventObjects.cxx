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
#include "EVENT/ReconstructedParticle.h"
#include "IMPL/LCCollectionVec.h"
#include "EVENT/Vertex.h"

// ROOT
#include "TEveElement.h"
#include "TEvePointSet.h"
#include "TEveCompound.h"
#include "TColor.h"
#include "TEveTrackPropagator.h"
#include "TEveVSDStructs.h"
#include "TEveTrack.h"
#include "TEveRGBAPalette.h"

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

        const std::vector<std::string>* collNames = event->getCollectionNames();
        for (std::vector<std::string>::const_iterator it = collNames->begin();
                it != collNames->end();
                it++) {
            auto collectionName = *it;
            EVENT::LCCollection* collection = event->getCollection(collectionName);
            if (app_->excludeCollection(collectionName, collection)) {
                log(FINE) << "Excluded collection: " << collectionName << std::endl;
                continue;
            }
            TEveElementList* elements = nullptr;
            auto typeName = collection->getTypeName();
            if (typeName == LCIO::SIMTRACKERHIT) {
                elements = createSimTrackerHits(collection);
            } else if (typeName == LCIO::SIMCALORIMETERHIT) {
                elements = createSimCalorimeterHits(collection);
            } else if (typeName == LCIO::MCPARTICLE) {
                elements = createMCParticles(collection);
            } else if (typeName == LCIO::CLUSTER) {
                elements = createCalClusters(collection);
            } else if (typeName == LCIO::TRACK) {
                elements = createReconTracks(collection);
            } else if (typeName == LCIO::RECONSTRUCTEDPARTICLE) {
                elements = createReconstructedParticles(collection);
            }
            if (elements != nullptr) {
                elements->SetElementName(collectionName.c_str());
                elements->SetPickableRecursively(true);
                manager->AddElement(elements);
                typeMap_[typeName].push_back(elements);
                log(FINE) << "Added elements from collection: " << collectionName << std::endl;
            }
        }

        // Apply current MCParticle P cut
        //setMCPCut(mcPcut_);

        // Apply current Track P cut
        //setTrackPCut(trackPcut_);
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
    TEveElementList* EventObjects::createMCParticles(EVENT::LCCollection *coll) {

        TEveElementList* mcTracks = new TEveElementList();

        log(FINE) << "Building MCParticle collection with size: "
                << coll->getNumberOfElements() << std::endl;

        TEveTrackPropagator *propsetCharged = new TEveTrackPropagator();

        propsetCharged->SetMagFieldObj(new TEveMagFieldConst(0.0, app_->getMagFieldY(), 0.0));
        propsetCharged->SetDelta(0.01);
        propsetCharged->SetMaxR(150);
        propsetCharged->SetMaxZ(200);
        propsetCharged->SetMaxOrbs(2.0);
        propsetCharged->SetFitDecay(true);

        TEveTrackPropagator *propsetNeutral = new TEveTrackPropagator();
        propsetNeutral->SetMagFieldObj(new TEveMagFieldConst(0.0, app_->getMagFieldY(), 0.0));
        propsetNeutral->SetDelta(0.01);
        propsetNeutral->SetMaxR(150);
        propsetNeutral->SetMaxZ(200);
        propsetNeutral->SetMaxOrbs(1.0);
        propsetNeutral->SetFitDecay(true);

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
            } else {
                track->SetPropagator(propsetNeutral);
                track->SetMainColor(kYellow);
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

            // Decay point
            TEvePathMark* pmDecay = new TEvePathMark(TEvePathMark::kDecay);
            const double* endPoint = mcp->getEndpoint();
            if (endPoint != nullptr) {
                TEveVector v(endPoint[0]/10., endPoint[1]/10., endPoint[2]/10.);
                TEvePathMark* pm3 = new TEvePathMark(TEvePathMark::kDecay, v);
                track->AddPathMark(*pm3);
            }

            track->MakeTrack(false);

            track->SetUserData(new TrackUserData(mcp, p.Mag()));
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
        propsetCharged->SetFitReferences(true);
        /*
        propsetCharged->SetFitDaughters(true);
        propsetCharged->SetFitCluster2Ds(false);
        propsetCharged->SetFitDecay(false);
        propsetCharged->SetFitLineSegments(false);
        */

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

            // For "rotated" hits, use this correction from PF
            // x->z, y->x, z->y

            // Path marks for hits
            /*
            eveTrack->SetMarkerColor(kGreen);
            eveTrack->SetMarkerStyle(kFullCircle);
            eveTrack->SetMarkerSize(1);
            const EVENT::TrackerHitVec& hits = track->getTrackerHits();
            for (EVENT::TrackerHitVec::const_iterator it = hits.begin(); it != hits.end();
                    it++) {
                EVENT::TrackerHit* hit = *it;
                const double* hitPos = hit->getPosition();
                TEveVector v(hitPos[0]/10.0, hitPos[1]/10., hitPos[2]/10.);
                TEvePathMark* pm = new TEvePathMark(TEvePathMark::kReference, v, hit->getTime());
                eveTrack->AddPathMark(*pm);
                log(FINEST) << "Added TrackerHit path mark at: ("
                        << hitPos[0] << ", " << hitPos[1] << ", " << hitPos[2] << ") [mm]"
                        << std::endl;
            }
            eveTrack->SetRnrPoints(true);
            */

            eveTrack->SetElementTitle(Form("Recon Track\n"
                    "(x, y, z) = (%.3f, %.3f, %.3f)\n"
                    "(Px, Py, Pz) = (%.3f, %.3f, %.3f)\n"
                    "Charge = %.3f, P = %.3f, Chi2 = %.3f",
                    refPoint[0], refPoint[1], refPoint[2],
                    p.X(), p.Y(), p.Z(), charge, p.Mag(),
                    track->getChi2()));

            eveTrack->SetUserData(new TrackUserData(track, p.Mag()));

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

    TStyle EventObjects::createParticleStyle() {
            Int_t palette[12];
            palette[0] = TColor::GetColor("#800000");
            palette[1] = TColor::GetColor("#fffac8");
            palette[2] = TColor::GetColor("#808000");
            palette[3] = TColor::GetColor("#469990");
            palette[4] = TColor::GetColor("#000075");
            palette[5] = TColor::GetColor("#f58231");
            palette[6] = TColor::GetColor("#bfef45");
            palette[7] = TColor::GetColor("#42d4f4");
            palette[8] = TColor::GetColor("#911eb4");
            palette[9] = TColor::GetColor("#f032e6");
            palette[10] = TColor::GetColor("#ffd8b1");
            palette[11] = TColor::GetColor("#dcbeff");
            TStyle particleStyle;
            particleStyle.SetPalette(12, palette);
            return particleStyle;
        }

    void EventObjects::setMCPCut(double cut) {
        mcPCut = cut;
        log(INFO) << "Setting new MCParticle P cut: " << cut << std::endl;
        const std::vector<TEveElementList*>& particleLists = getElementsByType(std::string(LCIO::MCPARTICLE));
        if (particleLists.size() > 0) {
            for (std::vector<TEveElementList*>::const_iterator it = particleLists.begin();
                    it != particleLists.end(); it++) {
                TEveElementList* particleList = *(it);
                applyPCut(particleList, mcPCut);
            }
        }
    }

    void EventObjects::setTrackPCut(double cut) {
        trackPCut = cut;
        log(INFO) << "Setting new Track P cut: " << cut << std::endl;
        const std::vector<TEveElementList*>& trackLists = getElementsByType(std::string(LCIO::TRACK));
        if (trackLists.size() > 0) {
            for (std::vector<TEveElementList*>::const_iterator it = trackLists.begin();
                    it != trackLists.end(); it++) {
                TEveElementList* particleList = *(it);
                log(FINE) << "Applying P cut to: " << particleList->GetElementName() << std::endl;
                applyPCut(particleList, trackPCut);
            }
        }
    }

    void EventObjects::applyPCut(TEveElement* element, double& cut) {
        if (element->GetUserData() != nullptr) {
            TrackUserData* trackData = (TrackUserData*)(element->GetUserData());
            if (trackData != nullptr) {
                double p = trackData->p();
                if (p < cut) {
                    log(FINEST) << "Cutting Track with P: " << p << std::endl;
                    element->SetRnrSelf(false);
                } else {
                    element->SetRnrSelf(true);
                }
            }
        }
        for (TEveElement::List_i it = element->BeginChildren();
                it != element->EndChildren(); it++) {
            TEveTrack* track = dynamic_cast<TEveTrack*>(*(it));
            applyPCut(track, cut);
        }
    }

    void EventObjects::setChi2Cut(double cut) {
        chi2Cut_ = cut;
        log(INFO) << "Setting new Track chi2 cut: " << cut << std::endl;
        const std::vector<TEveElementList*>& trackLists = getElementsByType(std::string(LCIO::TRACK));
        if (trackLists.size() > 0) {
            for (std::vector<TEveElementList*>::const_iterator it = trackLists.begin();
                    it != trackLists.end(); it++) {
                TEveElementList* trackList = *(it);
                applyChi2Cut(trackList);
            }
        }
    }

    void EventObjects::applyChi2Cut(TEveElementList* trackList) {
        for (TEveElementList::List_i it = trackList->BeginChildren();
                it != trackList->EndChildren();
                it++ ) {
            TEveElement* element = *it;
            if (element->GetUserData() != nullptr) {
                LCObjectUserData* userData = (LCObjectUserData*) element->GetUserData();
                EVENT::Track* track = (EVENT::Track*) userData->getLCObject();
                if (track->getChi2() > chi2Cut_) {
                    log(FINEST) << "Cutting Track with chi2: " << track->getChi2() << std::endl;
                    element->SetRnrSelf(false);
                } else {
                    element->SetRnrSelf(true);
                }
            }
        }
    }

    TEveElementList* EventObjects::createReconstructedParticles(EVENT::LCCollection* coll) {

        TEveTrackPropagator *propsetCharged = new TEveTrackPropagator();
        propsetCharged->SetMagFieldObj(new TEveMagFieldConst(0.0, app_->getMagFieldY(), 0.0));
        propsetCharged->SetDelta(0.01);
        propsetCharged->SetMaxR(150);
        propsetCharged->SetMaxZ(200);
        propsetCharged->SetMaxOrbs(2.0);
        propsetCharged->SetFitDecay(true);

        TEveTrackPropagator *propsetNeutral = new TEveTrackPropagator();
        propsetNeutral->SetMagFieldObj(new TEveMagFieldConst(0.0, app_->getMagFieldY(), 0.0));
        propsetNeutral->SetDelta(0.01);
        propsetNeutral->SetMaxR(150);
        propsetNeutral->SetMaxZ(200);
        propsetNeutral->SetMaxOrbs(1.0);
        propsetNeutral->SetFitDecay(true);

        TStyle clusStyle = createParticleStyle();
        int nColors = clusStyle.GetNumberOfColors();
        int currColor = 0;
        TEveElementList* elements = new TEveElementList();
        //std::map<EVENT::ReconstructedParticle*, TEveCompound*> particleMap;
        for (int i = 0; i < coll->getNumberOfElements(); i++) {

            log(FINEST) << "Creating recon particle..." << std::endl;

            if (currColor > (nColors - 1)) {
                currColor = 0;
            }
            int color = clusStyle.GetColorPalette(currColor);

            EVENT::ReconstructedParticle* particle =
                    dynamic_cast<EVENT::ReconstructedParticle*>(coll->getElementAt(i));

            TEveCompound* compound = new TEveCompound("ReconstructedParticle");
            compound->OpenCompound();

            auto charge = particle->getCharge();
            auto endVertex = particle->getEndVertex();
            auto energy = particle->getEnergy();
            auto mass = particle->getMass();
            auto momentum = particle->getMomentum();
            auto px = momentum[0];
            auto py = momentum[1];
            auto pz = momentum[2];
            auto pid = particle->getParticleIDUsed();
            auto refPoint = particle->getReferencePoint();
            //auto startVertex = particle->getStartVertex();
            //auto vertexPosition = startVertex->getPosition();

            TVector3 p(px, py, pz);

            auto title = Form("Reconstructed Particle\n"
                    "(x, y, z) = (%.3f, %.3f, %.3f)\n"
                    "(Px, Py, Pz) = (%.3f, %.3f, %.3f)\n"
                    "Charge = %.3f, Energy = %.3f, PID = %d\n"
                    "P = %.3f",
                    refPoint[0], refPoint[1], refPoint[2],
                    px, py, pz,
                    charge, energy, pid->getPDG(),
                    p.Mag());

            // Create a track for the particle itself.
            TEveRecTrack *recTrack = new TEveRecTrack();
            recTrack->fV.Set(TEveVector(refPoint[0], refPoint[1], refPoint[2]));
            recTrack->fP.Set(p);
            recTrack->fSign = charge;
            TEveTrack *eveTrack = new TEveTrack(recTrack, nullptr);
            eveTrack->SetElementName("Track");
            if (charge != 0) {
                eveTrack->SetPropagator(propsetCharged);
            } else {
                eveTrack->SetPropagator(propsetNeutral);
            }
            eveTrack->SetMainColor(color);
            eveTrack->MakeTrack(false);
            eveTrack->SetElementTitle(title);
            eveTrack->SetElementName("Particle");
            compound->AddElement(eveTrack);

            // Create tracks and set their color.
            auto tracks = particle->getTracks();
            log(FINEST) << "Creating n tracks: " << tracks.size() << std::endl;
            IMPL::LCCollectionVec trackVec(LCIO::TRACK);
            trackVec.setSubset(true);
            for (EVENT::TrackVec::const_iterator it = tracks.begin();
                    it != tracks.end(); it++) {
                trackVec.push_back(*it);
            }
            TEveElementList* trackList = createReconTracks(&trackVec);
            trackList->SetElementName("Tracks");
            for (TEveElementList::List_i it = trackList->BeginChildren();
                    it != trackList->EndChildren(); it++) {
                (*it)->SetMainColor(color);
                (*it)->SetElementTitle(title);
                (*it)->SetRnrSelf(false);
            }
            compound->AddElement(trackList);

            // Build clusters and set their color.
            auto clusters = particle->getClusters();
            log(FINEST) << "Creating n clusters: " << tracks.size() << std::endl;
            IMPL::LCCollectionVec clusterVec(LCIO::CLUSTER);
            clusterVec.setSubset(true);
            for (EVENT::ClusterVec::const_iterator it = clusters.begin();
                    it != clusters.end(); it++) {
                EVENT::Cluster* clus = *it;
                const float* pos = clus->getPosition();
                log(FINEST) << "Adding cluster at: ("
                        << pos[0] << ", " << pos[1] << ", " << pos[2]
                        << ")" << std::endl;
                clusterVec.push_back(*it);
            }
            TEveElementList* clusterList = createCalClusters(&clusterVec);
            clusterList->SetElementName("Clusters");
            for (TEveElementList::List_i it = clusterList->BeginChildren();
                    it != clusterList->EndChildren(); it++) {
                TEveElement* clusterElement = (*it);
                clusterElement->SetMainColor(color);
                clusterElement->SetElementTitle(title);
                for (TEveElement::List_i it2 = clusterElement->BeginChildren();
                        it2 != clusterElement->EndChildren();
                        it2++) {
                    TEveElement* hitElement = (*it2);
                    hitElement->SetMainColor(color);
                    hitElement->SetElementTitle(title);
                }
            }
            compound->AddElement(clusterList);

            compound->CloseCompound();
            compound->SetRnrSelfChildren(true, true);
            elements->AddElement(compound);
            //particleMap[particle] = compound;

            ++currColor;

            log(FINEST) << "Done creating recon particle!" << std::endl;
        }

        /*
        for (std::map<EVENT::ReconstructedParticle*, TEveCompound*>::iterator it = particleMap.begin();
                it != particleMap.end(); it++) {
            EVENT::ReconstructedParticle* particle = it->first;
            TEveCompound* compound = it->second;
            compound->OpenCompound();
            TEveElementList* particles = new TEveElementList("Particles");
        }
        */

        return elements;
    }

    const std::vector<TEveElementList*> EventObjects::getElementsByType(const std::string& typeName) {
        return typeMap_[typeName];
    }

} /* namespace hps */
