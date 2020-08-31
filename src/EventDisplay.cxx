#include "EventDisplay.h"

// C++ standard library
#include <unistd.h>
#include <iostream>
#include <stdexcept>

// ROOT
#include "TGeoManager.h"
#include "TRint.h"
#include "TEveManager.h"
#include "TEveBrowser.h"
#include "TGeoNode.h"
#include "TEveGeoNode.h"

namespace HPS {

    void EventDisplay::parseArgs (int argc, char **argv) {
        std::cout << "Parsing args ..." << std::endl;
        int c = 0;
        while ((c = getopt (argc, argv, "g:")) != -1) {
            switch (c) {
                case 'g':
                    std::cout << "GDML file: " << optarg << std::endl;
                    this->geometryFile = std::string(optarg);
                    break;
                case '?':
                    std::cout << optopt << std::endl;
                    break;
            }
        }

        for (int index = optind; index < argc; index++) {
            std::cout << "LCIO file: " << argv[index] << std::endl;
            this->lcioFileList_.push_back(std::string(argv[index]));
        }

        if (this->geometryFile.length() == 0) {
            throw std::runtime_error("Missing name of geometry file (provide with '-g' switch).");
        }

        std::cout << "Done parsing args!" << std::endl;
    }

    int EventDisplay::run() {
        TRint *app = 0;
        app = new TRint("ROOT App", 0, 0);
        //app->SetPrompt("");

        TEveManager* gEve = TEveManager::Create(kTRUE , "FV");

        TEveBrowser* browser = gEve->GetBrowser();
        browser->StartEmbedding(TRootBrowser::kLeft);

        TGeoManager* gGeoManager = gEve->GetGeometry(this->geometryFile.c_str());
        TGeoNode* top = gGeoManager->GetTopNode();
        TEveGeoTopNode* world = new TEveGeoTopNode(gGeoManager, top);
        gEve->AddGlobalElement(world);
        gEve->FullRedraw3D(kTRUE);

        //TGLViewer* v = gEve->GetDefaultGLViewer();
        //v->DoDraw();

        browser->StartEmbedding(TRootBrowser::kLeft);
        browser->SetTabTitle("Event Control", 0);
        browser->StopEmbedding();

        app->Run(kFALSE);

        return 0;
    }
} /* namespace HPS */
