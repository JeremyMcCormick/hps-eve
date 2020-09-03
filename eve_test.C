#include <iostream>

#include "TGeoManager.h"
#include "TEveGeoNode.h"
#include "TEveManager.h"
#include "TRint.h"
#include "TEveBrowser.h"
#include "TEveElement.h"
#include "TGeoNode.h"
#include "TGeoNavigator.h"
#include "TEveTrans.h"

// Add elements which are all children of a single volume
void addGeoElements(TGeoManager* geo,
                    TEveManager* mgr,
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
    mgr->AddGlobalElement(elements);
}

void eve_test() {

    // Create ROOT intepreter app
    TRint *app = 0;
    app = new TRint("Eve App", 0, 0);

    // Create Eve manager
    TEveManager *manager = TEveManager::Create(kTRUE, "FV");

    // Get Eve browser and embed it
    TEveBrowser *browser = manager->GetBrowser();
    browser->StartEmbedding(TRootBrowser::kLeft);

    const std::string& geomFile = std::string("detector.gdml");
    auto geo = TGeoManager::Import(geomFile.c_str());

    geo->DefaultColors();

    auto topNode = gGeoManager->GetTopNode();

    addGeoElements(geo,
                   manager,
                   "SVT",
                   "/world_volume_1/tracking_volume_0/base_volume_0",
                   "module_L");

    addGeoElements(geo,
                   manager,
                   "ECAL",
                   "/world_volume_1",
                   "crystal_volume");

    browser->SetTabTitle("Event Control", 0);
    browser->StopEmbedding();

    manager->FullRedraw3D();

    app->Run(kFALSE);
}

