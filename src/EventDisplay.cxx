#include "EventDisplay.h"

// HPS
#include "DetectorGeometry.h"
#include "EventManager.h"
#include "FileCache.h"

// ROOT
#include "TEveManager.h"
#include "TEveBrowser.h"
#include "TEveViewer.h"
#include "TGFrame.h"
#include "TGButton.h"
#include "TSystem.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TGNumberEntry.h"

// C++ standard library
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include <fstream>
#include <limits>

ClassImp(hps::EventDisplay);

namespace hps {

    EventDisplay* EventDisplay::instance_ = nullptr;

    EventDisplay::EventDisplay() :
            Logger("EventDisplay"),
            TGMainFrame (gClient->GetRoot(), 320, 320) {
    }

    EventDisplay::~EventDisplay() {
        delete cache_;
    }

    void EventDisplay::initialize() {

        // This pointer needs to be set externally for now before initialization.
        if (eveManager_ == nullptr) {
            throw std::runtime_error("The Eve manager was not set!");
        }

        // Create the file cache.
        cache_ = new FileCache(cacheDir_);
        cache_->setLogLevel(getLogLevel());
        cache_->createCacheDir();

        // Initialize the geometry and load detector if GDML was provided.
        det_ = new DetectorGeometry(this, cache_);
        det_->setLogLevel(getLogLevel());
        if (geometryFile_.size() > 0) {
            log("Opening geometry file: " + geometryFile_, INFO);
            det_->loadDetectorFile(geometryFile_);
            log("Done opening geometry!");
        }

        // Create the event manager.
        eventManager_ = new EventManager(this);
        eveManager_->AddEvent(eventManager_);
        eventManager_->Open();

        // Build the GUI.
        buildGUI();
    }

    void EventDisplay::buildGUI() {

        SetWindowName("HPS Event Display");

        TGGroupFrame *frmEvent = new TGGroupFrame(this, "Event Navigation", kHorizontalFrame);
        TGHorizontalFrame *hf = new TGHorizontalFrame(this);
        frmEvent->AddFrame(hf);

        auto homedir = getenv("HPS_EVE_DIR");
        if (!homedir) {
            throw std::runtime_error("HPS_EVE_DIR is not set!");
        }
        TString icondir(Form("%s/share/icons/", homedir));

        // Event Control
        {
            TGPictureButton* b = 0;

            // Next event
            b = new TGPictureButton (hf, gClient->GetPicture (icondir + "GoBack.gif"));
            hf->AddFrame(b);
            b->Connect ("Clicked()", "hps::EventManager", eventManager_, "PrevEvent()");

            // Previous event
            b = new TGPictureButton (hf, gClient->GetPicture (icondir + "GoForward.gif"));
            hf->AddFrame(b);
            b->Connect("Clicked()", "hps::EventManager", eventManager_, "NextEvent()");

            // Go to event
            TGHorizontalFrame* eventNrFrame = new TGHorizontalFrame(frmEvent);
            TGLabel* eventNrLabel = new TGLabel(eventNrFrame, "  Go to \n  Event");
            eventNumberEntry_ = new TGNumberEntry(eventNrFrame, 0, 5, -1,
                                                  TGNumberFormat::kNESInteger,
                                                  TGNumberFormat::kNEAAnyNumber,
                                                  TGNumberFormat::kNELLimitMinMax,
                                                  0, std::numeric_limits<int>::max());
            eventNrFrame->AddFrame(eventNrLabel, new TGLayoutHints(kLHintsNormal, 5, 5, 0, 0));
            eventNrFrame->AddFrame(eventNumberEntry_);
            eventNumberEntry_->GetNumberEntry()->Connect(
                    "ReturnPressed()", "hps::EventManager", eventManager_, "SetEventNumber()");
            frmEvent->AddFrame(eventNrFrame);

            // Add event frame
            AddFrame(frmEvent, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        }

        // Display cuts
        {
            TGGroupFrame *frmCuts = new TGGroupFrame(this, "Cuts", kHorizontalFrame);
            TGVerticalFrame* vf = new TGVerticalFrame(frmCuts);
            frmCuts->AddFrame(vf);

            // MCParticle P cut
            TGGroupFrame *frmMCParticlePCut = new TGGroupFrame(vf, "MCParticle P Cut: ", kHorizontalFrame);
            vf->AddFrame(frmMCParticlePCut, new TGLayoutHints (kLHintsLeft, 2, 2, 2, 2));
            TGHorizontalFrame *cellSize = new TGHorizontalFrame(frmMCParticlePCut);
            TGLabel *cellLabel = new TGLabel(frmMCParticlePCut, "GeV");
            cellSize->AddFrame(cellLabel, new TGLayoutHints(kLHintsLeft));
            MCParticlePCutEntry_ = new TGNumberEntry(frmMCParticlePCut, 0.0, 5, -1,
                                             TGNumberFormat::kNESRealThree,
                                             TGNumberFormat::kNEANonNegative,
                                             TGNumberFormat::kNELNoLimits,
                                             0.000, 10.0);
            frmMCParticlePCut->AddFrame(MCParticlePCutEntry_);
            frmMCParticlePCut->AddFrame(cellLabel, new TGLayoutHints(kLHintsBottom, 2, 0, 0, 0));
            MCParticlePCutEntry_->Connect ("ValueSet(Long_t)", "hps::EventManager", eventManager_, "modifyMCPCut()");

            // Track P cut
            TGGroupFrame *frmTrackPCut = new TGGroupFrame(vf, "Track P Cut: ", kHorizontalFrame);
            vf->AddFrame(frmTrackPCut, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
            cellSize = new TGHorizontalFrame(frmTrackPCut);
            cellLabel = new TGLabel(frmTrackPCut, "GeV");
            cellSize->AddFrame(cellLabel, new TGLayoutHints(kLHintsLeft));
            trackPCutEntry_ = new TGNumberEntry(frmTrackPCut, 0.0, 5, -1,
                                                TGNumberFormat::kNESRealThree,
                                                TGNumberFormat::kNEANonNegative,
                                                TGNumberFormat::kNELNoLimits,
                                                0.000, 10.0);
            frmTrackPCut->AddFrame(trackPCutEntry_);
            frmTrackPCut->AddFrame(cellLabel, new TGLayoutHints(kLHintsBottom, 2, 0, 0, 0));
            trackPCutEntry_->Connect ("ValueSet(Long_t)", "hps::EventManager", eventManager_, "modifyTrackPCut()");

            // Track chi2 cut
            TGGroupFrame* frmChi2Cut = new TGGroupFrame(vf, "Track chi2 cut:", kHorizontalFrame);
            vf->AddFrame(frmChi2Cut, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
            cellSize->AddFrame(cellLabel, new TGLayoutHints(kLHintsLeft));
            chi2CutEntry_ = new TGNumberEntry(frmChi2Cut, 40.0, 5, -1,
                                              TGNumberFormat::kNESRealTwo,
                                              TGNumberFormat::kNEAPositive,
                                              TGNumberFormat::kNELNoLimits,
                                              0.01, 999.0);
            frmChi2Cut->AddFrame(chi2CutEntry_);
            chi2CutEntry_->Connect ("ValueSet(Long_t)", "hps::EventManager", eventManager_, "modifyChi2Cut()");

            AddFrame(frmCuts, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY | kLHintsTop));
        }

        MapSubwindows();
        Resize(GetDefaultSize());
        MapWindow();
    }

    int EventDisplay::getCurrentEventNumber() {
        return eventNumberEntry_->GetIntNumber();
    }

    EventManager* EventDisplay::getEventManager() {
        return eventManager_;
    }

    TEveManager* EventDisplay::getEveManager() {
        return eveManager_;
    }

    DetectorGeometry* EventDisplay::getDetectorGeometry() {
        return det_;
    }

    double EventDisplay::getMagFieldY() {
        return bY_;
    }

    const std::vector<std::string>& EventDisplay::getLcioFiles() {
        return lcioFileList_;
    }

    bool EventDisplay::excludeCollection(const std::string& collName) {
        return excludeColls_.find(collName) != excludeColls_.end();
    }

    EventDisplay* EventDisplay::getInstance() {
        if (instance_ == nullptr) {
            instance_ = new EventDisplay();
        }
        return instance_;
    }

    double EventDisplay::getMCPCut() {
        return MCParticlePCutEntry_->GetNumber();
    }

    double EventDisplay::getTrackPCut() {
        return trackPCutEntry_->GetNumber();
    }

    double EventDisplay::getChi2Cut() {
        return chi2CutEntry_->GetNumber();
    }

    void EventDisplay::setEveManager(TEveManager* eveManager) {
        eveManager_ = eveManager;
    }

    void EventDisplay::setGeometryFile(std::string geometryFile) {
        geometryFile_ = geometryFile;
    }

    void EventDisplay::setCacheDir(std::string cacheDir) {
        cacheDir_ = cacheDir;
    }

    void EventDisplay::addLcioFiles(std::vector<std::string> lcioFileList) {
        lcioFileList_.insert(lcioFileList_.end(), lcioFileList.begin(), lcioFileList.end());
    }

    void EventDisplay::addExcludeCollections(std::set<std::string> excludeColls) {
        excludeColls_.insert(excludeColls.begin(), excludeColls.end());
    }

    void EventDisplay::setMagFieldY(double bY) {
        bY_ = bY;
    }

} /* namespace hps */
