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

    EventDisplay::EventDisplay(TEveManager *manager,
                               std::string geometryFile,
                               std::string cacheDir,
                               std::vector<std::string> lcioFileList,
                               std::set<std::string> excludeColls,
                               double bY) :
            Verbosity("EventDisplay"),
            TGMainFrame (gClient->GetRoot(), 320, 320),
            lcioFileList_(lcioFileList),
            excludeColls_(excludeColls),
            eveManager_(manager),
            eventManager_(nullptr),
            eventNumberEntry_(nullptr),
            bY_(bY) {

        SetWindowName("HPS Event Display");

        det_ = new DetectorGeometry(manager, cacheDir);
        if (geometryFile.size() > 0) {
            log("Opening geometry file: " + geometryFile, INFO);
            det_->loadDetectorFile(geometryFile);
            log("Done opening geometry!");
        }

        if (bY == 0.0) {
            log("The fixed B-field value is zero!", WARNING);
        }

        eventManager_ = new EventManager(this);
        eveManager_->AddEvent(eventManager_);
        eventManager_->Open();

        ///////////////////////////
        // Start build GUI
        ///////////////////////////

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
                                             0.001, 10.0);
            frmPTCut->AddFrame(PTCutEntry_);
            frmPTCut->AddFrame(cellLabel, new TGLayoutHints(kLHintsBottom, 2, 0, 0, 0));

            // TODO: connect this to EventObjects
            PTCutEntry_->Connect ("ValueSet(Long_t)", "hps::EventManager", eventManager_, "modifyPCut()");

            AddFrame(frmCuts, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY | kLHintsTop));
        }

        MapSubwindows();
        Resize(GetDefaultSize());
        MapWindow();

        ///////////////////
        // End build GUI
        ///////////////////
    }

    EventDisplay::~EventDisplay() {
    }

    int EventDisplay::getCurrentEventNumber() {
        return eventNumberEntry_->GetIntNumber();
    }

    void EventDisplay::setVerbosity(int verbosity) {
        Verbosity::setVerbosity(verbosity);
        det_->setVerbosity(verbosity);
        eventManager_->setVerbosity(verbosity);
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

    EventDisplay* EventDisplay::createEventDisplay(TEveManager* manager,
                                                        std::string geometryFile,
                                                        std::string cacheDir,
                                                        std::vector<std::string> lcioFileList,
                                                        std::set<std::string> excludeColls,
                                                        double bY) {
        if (instance_ != nullptr) {
            throw std::runtime_error("The EventDisplay should only be created once!");
        }
        instance_ = new EventDisplay(manager, geometryFile, cacheDir, lcioFileList, excludeColls, bY);
        return instance_;
    }

    EventDisplay* EventDisplay::getInstance() {
        return instance_;
    }

    double EventDisplay::getPCut() {
        return  PTCutEntry_->GetNumber();
    }

} /* namespace hps */
