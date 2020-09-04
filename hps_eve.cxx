#include <iostream>

#include "EventDisplay.h"

// C++ standard library
#include <string>
#include <vector>
#include <set>

// ROOT
#include "TRint.h"
#include "TEveBrowser.h"

using hps::EventDisplay;

int main (int argc, char **argv) {

    std::string geometryFile;
    std::vector<std::string> lcioFileList;
    std::set<std::string> excludeColls;

    int c = 0;
    while ((c = getopt (argc, argv, "e:g:")) != -1) {
        switch (c) {
            case 'g':
                geometryFile = std::string(optarg);
                break;
            case 'e':
                excludeColls.insert(std::string(optarg));
                break;
            case '?':
                std::cout << optopt << std::endl;
                break;
        }
    }

    for (int index = optind; index < argc; index++) {
        lcioFileList.push_back (std::string (argv[index]));
    }

    if (geometryFile.length () == 0) {
        throw std::runtime_error ("Missing name of geometry file (provide with '-g' switch)");
    }

    if (lcioFileList.size () == 0) {
        throw std::runtime_error("Missing one or more LCIO files (provide as extra arguments)");
    }

    // Create ROOT intepreter app
    TRint *app = 0;
    app = new TRint("XXX", 0, 0);

    // Create Eve manager
    TEveManager *manager = TEveManager::Create(kTRUE, "FV");

    // Get Eve browser and embed it
    TEveBrowser *browser = manager->GetBrowser ();
    browser->StartEmbedding(TRootBrowser::kLeft);

    // Create the main event display class
    EventDisplay display(manager, geometryFile, lcioFileList, excludeColls);

    browser->SetTabTitle("Event Control", 0);
    browser->StopEmbedding();

    manager->FullRedraw3D();

    app->Run(kFALSE);

    return 0;
}
