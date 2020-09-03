#include "EventDisplay.h"

// HPS
#include "EventManager.h"

// C++ standard library
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include <fstream>

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

ClassImp(hps::EventDisplay);

namespace hps {

    EventDisplay::EventDisplay(TEveManager* manager,
                               std::string geometryFile,
                               std::vector<std::string> lcioFileList)
            : TGMainFrame(gClient->GetRoot(), 320, 320),
              geometryFile_(geometryFile),
              lcioFileList_(lcioFileList),
              manager_(manager),
              eventManager_(nullptr) {

        SetWindowName("HPS Event Display");

        TGeoManager* gGeoManager = manager_->GetGeometry(this->geometryFile_.c_str());

        geo_ = new DetectorGeometry(gGeoManager, manager);

        eventManager_ = new EventManager(lcioFileList);
        manager_->AddEvent(eventManager_);
        eventManager_->Open();

        TGVerticalFrame* contents = new TGVerticalFrame(this, 1000, 1200);
        TGHorizontalFrame* commandFrameNextEvent = new TGHorizontalFrame(contents, 100,0);

        TGButton* buttonNext = new TGTextButton(commandFrameNextEvent, "Next Event >>>");
        commandFrameNextEvent->AddFrame(buttonNext, new TGLayoutHints(kLHintsExpandX));
        buttonNext->Connect("Pressed()", "hps::EventDisplay", this, "NextEvent()");

        contents->AddFrame(commandFrameNextEvent, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        AddFrame(contents, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        MapSubwindows();
        Resize(GetDefaultSize());
        MapWindow();
    }

    EventDisplay::~EventDisplay() {
    }

    void EventDisplay::NextEvent() {
        std::cout << "<<<< nextEvent" << std::endl;
        eventManager_->NextEvent();
    }

} /* namespace HPS */
