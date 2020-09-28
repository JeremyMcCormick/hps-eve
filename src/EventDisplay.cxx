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
            TGMainFrame (gClient->GetRoot(), 320, 320),
            eveManager_(nullptr),
            eventManager_(nullptr),
            eventNumberEntry_(nullptr),
            det_(nullptr),
            cache_(nullptr),
            PTCutEntry_(nullptr),
            bY_(0) {
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

        // Display Cuts
        // from Druid/src/GUI.cc
        {
            TGGroupFrame *frmCuts = new TGGroupFrame(this, "Cuts", kHorizontalFrame);
            TGVerticalFrame* vf = new TGVerticalFrame(frmCuts);
            frmCuts->AddFrame(vf);

            TGGroupFrame *frmPTCut = new TGGroupFrame(vf, "MCParticle P Cut: ", kHorizontalFrame);
            vf->AddFrame(frmPTCut, new TGLayoutHints (kLHintsLeft, 2, 2, 0, 0));
            TGHorizontalFrame *cellSize = new TGHorizontalFrame(frmPTCut);
            TGLabel *cellLabel = new TGLabel(frmPTCut, "GeV");
            cellSize->AddFrame(cellLabel, new TGLayoutHints(kLHintsLeft));
            PTCutEntry_ = new TGNumberEntry (frmPTCut, 0.0, 5, -1,
                                             TGNumberFormat::kNESRealThree,
                                             TGNumberFormat::kNEAPositive,
                                             TGNumberFormat::kNELNoLimits,
                                             0.000, 10.0);
            frmPTCut->AddFrame(PTCutEntry_);
            frmPTCut->AddFrame(cellLabel, new TGLayoutHints(kLHintsBottom, 2, 0, 0, 0));

            // TODO: connect this to EventObjects
            PTCutEntry_->Connect ("ValueSet(Long_t)", "hps::EventManager", eventManager_, "modifyPCut()");

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

    double EventDisplay::getPCut() {
        return  PTCutEntry_->GetNumber();
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
