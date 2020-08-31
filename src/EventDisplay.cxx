#include "EventDisplay.h"

// C++ standard library
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <stdlib.h>

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

namespace HPS {

    EventDisplay::EventDisplay(TEveManager* manager,
                               std::string geometryFile,
                               std::vector<std::string> lcioFileList)
            : TGMainFrame(gClient->GetRoot(), 320, 320),
              geometryFile_(geometryFile),
              lcioFileList_(lcioFileList),
              manager_(manager) {

        TGeoManager* gGeoManager = manager_->GetGeometry(this->geometryFile_.c_str());
        TGeoNode* top = gGeoManager->GetTopNode();
        TEveGeoTopNode* world = new TEveGeoTopNode(gGeoManager, top);
        manager_->AddGlobalElement(world);
        manager_->FullRedraw3D(kTRUE);
    }

} /* namespace HPS */
