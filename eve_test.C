#include <iostream>

#include "TGeoManager.h"
#include "TEveGeoNode.h"
#include "TEveManager.h"
#include "TRint.h"
#include "TEveBrowser.h"
#include "TEveElement.h"
#include "TGeoNode.h"
#include "TGeoNavigator.h"

TEveElementList* build_tracker(TGeoManager* geo) {
    auto trackerElements = new TEveElementList("Tracker");
    TGeoNode* trackingVol = geo->GetTopVolume()->FindNode("tracking_volume_0");
    TGeoNode* baseVol = trackingVol->GetVolume()->FindNode("base_volume_0");
    auto nav = geo->GetCurrentNavigator();
    for (int i=0; i<baseVol->GetNdaughters(); i++) {
        TGeoNode* node = baseVol->GetDaughter(i);
        if (std::string(node->GetName()).find("module_L") != std::string::npos) {
            std::cout << "Processing node: " << node->GetName() << std::endl;
            nav->CdDown(node);
            auto matrix = nav->GetCurrentMatrix();
            matrix->Print();
            node->GetVolume()->GetShape()->GetName();

            TGeoVolume* vol = gGeoManager->GetCurrentVolume();
            TEveGeoShape* shape = new TEveGeoShape(node->GetName(), vol->GetMaterial()->GetName());
            shape->SetMainColor(vol->GetLineColor());
            shape->SetMainTransparency(vol->GetTransparency());
            shape->RefMainTrans().SetFrom(*nav->GetCurrentMatrix());
            trackerElements->AddElement(shape);

            std::cout << "Added tracker element: " << shape->GetName() << std::endl;

            std::cout << std::endl;
        }
    }
    return trackerElements;
}

void eve_test() {
    std::cout << "HPS Eve Test" << std::endl;

    // Create ROOT intepreter app
    TRint *app = 0;
    app = new TRint("XXX", 0, 0);

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

