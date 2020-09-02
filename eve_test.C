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

TEveElementList* build_tracker(TGeoManager* geo) {
    auto trackerElements = new TEveElementList("Tracker");
    //TGeoNode* trackingVol = geo->GetTopVolume()->FindNode("tracking_volume_0");
    //TGeoNode* baseVol = trackingVol->GetVolume()->FindNode("base_volume_0");
    geo->cd("/world_volume_1/tracking_volume_0/base_volume_0");
    auto ndau = geo->GetCurrentNode()->GetNdaughters();
    for (int i=0; i<ndau; i++) {
        geo->CdDown(i);
        //TGeoNode* node = baseVol->GetDaughter(i);
        TGeoNode* node = geo->GetCurrentNode();
        std::cout << "cd into: " << geo->GetCurrentNode() << std::endl;
        if (std::string(node->GetName()).find("module_L") != std::string::npos) {
            std::cout << "Adding module: " << node->GetName() << std::endl;
            auto nodeName = node->GetName();
            std::string path = std::string("/world_volume_1/tracking_volume_0/base_volume_0/") + std::string(nodeName);

            geo->cd(path.c_str());

            TGeoVolume* vol = gGeoManager->GetCurrentVolume();
            TEveGeoShape* shape = new TEveGeoShape(node->GetName(), vol->GetMaterial()->GetName());
            shape->SetShape((TGeoShape*) vol->GetShape()->Clone());
            shape->SetMainColor(vol->GetLineColor());
            shape->SetFillColor(vol->GetFillColor());
            shape->SetMainTransparency(vol->GetTransparency());
            shape->RefMainTrans().SetFrom(*geo->GetCurrentMatrix());
            trackerElements->AddElement(shape);
        }
        geo->CdUp();
    }
    return trackerElements;
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

    //auto eveNode = new TEveGeoTopNode(geo, topNode);
    //eveNode->ExpandIntoListTreesRecursively();
    //manager->AddGlobalElement(eveNode);

    TEveElement* tracker = build_tracker(geo);
    manager->AddGlobalElement(tracker);

    browser->SetTabTitle("Event Control", 0);
    browser->StopEmbedding();

    manager->FullRedraw3D();

    app->Run(kFALSE);
}

