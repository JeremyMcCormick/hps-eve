#include "DetectorGeometry.h"

// C++ standard library
#include <iostream>
#include <stdexcept>

// ROOT
#include "TEveElement.h"
#include "TEveGeoNode.h"
#include "TEveGeoShape.h"
#include "TEveTrans.h"
#include "TEveEventManager.h"
#include "TEveScene.h"

namespace hps {

    DetectorGeometry::DetectorGeometry(TGeoManager* geo, TEveManager* eve)
        : geo_(geo), eve_(eve) {
        addTracker();
        addEcal();
    }

    TEveElementList* DetectorGeometry::createGeoElements(TGeoManager* geo,
                                                         const char* name,
                                                         const char* path,
                                                         const char* patt) {
        auto elements = new TEveElementList(name);
        geo->cd(path);
        auto ndau = geo->GetCurrentNode()->GetNdaughters();
        for (int i=0; i<ndau; i++) {
            geo->CdDown(i);
            TGeoNode* node = geo->GetCurrentNode();
            if (std::string(node->GetName()).find(patt) != std::string::npos) {
                auto nodeName = node->GetName();
                TGeoVolume* vol = gGeoManager->GetCurrentVolume();
                TEveGeoShape* shape = new TEveGeoShape(node->GetName(), vol->GetMaterial()->GetName());
                shape->SetShape((TGeoShape*) vol->GetShape()->Clone());
                shape->SetMainColor(vol->GetLineColor());
                shape->SetFillColor(vol->GetFillColor());
                shape->SetMainTransparency(vol->GetTransparency());
                shape->RefMainTrans().SetFrom(*geo->GetCurrentMatrix());
                elements->AddElement(shape);
            }
            geo->CdUp();
        }
        return elements;
    }

    void DetectorGeometry::addTracker() {
        auto tracker = createGeoElements(geo_,
                                         "SVT",
                                         "/world_volume_1/tracking_volume_0/base_volume_0",
                                         "module_L");
        eve_->AddGlobalElement(tracker);
    }

    void DetectorGeometry::addEcal() {
        auto cal = createGeoElements(geo_,
                                     "ECAL",
                                     "/world_volume_1",
                                     "crystal_volume");
        eve_->AddGlobalElement(cal);
    }
}
