#include "EventDisplay.h"

// HPS
#include "EventManager.h"

// C++ standard library
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



    EventDisplay::EventDisplay(TEveManager* manager,
                               std::string geometryFile,
                               std::vector<std::string> lcioFileList)
            : TGMainFrame(gClient->GetRoot(), 320, 320),
              geometryFile_(geometryFile),
              lcioFileList_(lcioFileList),
              manager_(manager),
              eventManager_(nullptr),
              eventNumberEntry_(nullptr) {

        SetWindowName("HPS Event Display");

        TGeoManager* gGeoManager = manager_->GetGeometry(this->geometryFile_.c_str());

        geo_ = new DetectorGeometry(gGeoManager, manager);

        eventManager_ = new EventManager(manager,
                                         gGeoManager,
                                         this,
                                         lcioFileList,
                                         1 /* verbose lvl */);
        manager_->AddEvent(eventManager_);
        eventManager_->Open();

        ///////////////////////////
        // Start build GUI
        ///////////////////////////

        TGGroupFrame *frmEvent = new TGGroupFrame(this, "Event Navigation", kHorizontalFrame);
        TGHorizontalFrame *hf = new TGHorizontalFrame(this);
        frmEvent->AddFrame(hf);

        // TODO: get this dir from env var in a script configured by CMake
        TString icondir(Form("%s/icons/", getenv("PWD")));
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

        ///////////////////
        // End build GUI
        ///////////////////

        MapSubwindows();
        Resize(GetDefaultSize());
        MapWindow();
    }

    EventDisplay::~EventDisplay() {
    }

    int EventDisplay::getCurrentEventNumber() {
        return eventNumberEntry_->GetIntNumber();
    }
} /* namespace hps */
