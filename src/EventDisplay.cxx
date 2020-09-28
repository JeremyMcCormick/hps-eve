#include "EventDisplay.h"

// HPS
#include "DetectorGeometry.h"
#include "EventManager.h"
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include <fstream>
#include <limits>

// ROOT
#include "TGeoManager.h"
#include "TRint.h"
#include "TEveManager.h"
#include "TEveBrowser.h"
#include "TEveViewer.h"
#include "TGeoNode.h"
#include "TEveGeoNode.h"
#include "TGFrame.h"
#include "TGButton.h"
#include "TEveEventManager.h"
#include "TSystem.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TGNumberEntry.h"

ClassImp(hps::EventDisplay);

namespace hps {

    EventDisplay* EventDisplay::instance_ = nullptr;

    EventDisplay::EventDisplay(TEveManager* manager,
                               std::string geometryFile,
                               std::string cacheDir,
                               std::vector<std::string> lcioFileList,
                               std::set<std::string> excludeColls,
                               double bY,
                               int logLevel) :
            Logger("EventDisplay", logLevel),
            TGMainFrame (gClient->GetRoot(), 320, 320),
            eveManager_(manager),
            geometryFile_(geometryFile),
            cacheDir_(cacheDir),
            lcioFileList_(lcioFileList),
            excludeColls_(excludeColls),
            eventManager_(nullptr),
            eventNumberEntry_(nullptr),
            det_(nullptr),
            PTCutEntry_(nullptr),
            bY_(bY) {
    }

    EventDisplay::EventDisplay() :
            Logger("EventDisplay"),
            TGMainFrame (gClient->GetRoot(), 320, 320),
            eveManager_(nullptr),
            eventManager_(nullptr),
            eventNumberEntry_(nullptr),
            det_(nullptr),
            PTCutEntry_(nullptr),
            bY_(0) {
        }


    EventDisplay::~EventDisplay() {
    }

    void EventDisplay::initialize() {
        det_ = new DetectorGeometry(this, cacheDir_);
        if (geometryFile_.size() > 0) {
            log("Opening geometry file: " + geometryFile_, INFO);
            det_->loadDetectorFile(geometryFile_);
            log("Done opening geometry!");
        }

        eventManager_ = new EventManager(this);
        eveManager_->AddEvent(eventManager_);
        eventManager_->Open();

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

    /*
    void EventDisplay::setLogLevel(int verbosity) {
        Logger::setLogLevel(verbosity);
        det_->setLogLevel(verbosity);
        eventManager_->setLogLevel(verbosity);
    }
    */

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

    EventDisplay* EventDisplay::createEventDisplay(TEveManager* manager,
                                                   std::string geometryFile,
                                                   std::string cacheDir,
                                                   std::vector<std::string> lcioFileList,
                                                   std::set<std::string> excludeColls,
                                                   double bY,
                                                   int logLevel) {
        if (instance_ != nullptr) {
            throw std::runtime_error("The EventDisplay should only be created once!");
        }
        instance_ = new EventDisplay(manager,
                                     geometryFile,
                                     cacheDir,
                                     lcioFileList,
                                     excludeColls,
                                     bY,
                                     logLevel);
        return instance_;
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
