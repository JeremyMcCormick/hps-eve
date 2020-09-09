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

void print_usage(const char* msg = 0, bool doExit = true, int returnCode = 1) {
    std::cout << "Usage: hps-eve [args] [LCIO files]" << std::endl;
    std::cout << "    -g [gdml file]" << std::endl;
    std::cout << "    -b [bY]" << std::endl;
    std::cout << "    -v [verbose]" << std::endl;
    std::cout << "    -e [exclude coll]" << std::endl;
    if (msg) {
        std::cout << msg << std::endl;
    }
    if (doExit) {
        exit(returnCode);
    }
}

int main (int argc, char **argv) {

    std::string geometryFile;
    std::vector<std::string> lcioFileList;
    std::set<std::string> excludeColls;
    int verbose = 0;
    double bY = 0.0;

    int c = 0;
    while ((c = getopt (argc, argv, "b:e:g:v:")) != -1) {
        switch (c) {
            case 'g':
                geometryFile = std::string(optarg);
                break;
            case 'e':
                excludeColls.insert(std::string(optarg));
                break;
            case 'b':
                bY = std::stod(optarg);
                break;
            case 'v':
                verbose = atoi(optarg);
                break;
            case 'h':
                print_usage();
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
        print_usage("Missing name of geometry file (provide with '-g' switch)");
    }

    if (lcioFileList.size () == 0) {
        print_usage("Missing one or more LCIO files (provide as extra arguments)");
    }

    std::cout << std::endl;
    std::cout << "  -------- HPS Event Display -------- " << std::endl;
    std::cout << "    verbose: " << verbose << std::endl;
    std::cout << "    geometry: " << geometryFile << std::endl;
    std::cout << "    files: " << std::endl;
    for (std::vector<std::string>::iterator it = lcioFileList.begin();
            it != lcioFileList.end();
            it++) {
        std::cout << "      " << *it << std::endl;
    }
    std::cout << "    exclude: " << std::endl;
    for (std::set<std::string>::iterator it = excludeColls.begin();
            it != excludeColls.end();
            it++) {
        std::cout << "      " << *it << std::endl;
    }
    std::cout << "    bY: " << bY << std::endl;
    std::cout << "  -------------------------------- " << std::endl;
    std::cout << std::endl;

    // Create ROOT intepreter app
    TRint *app = 0;
    app = new TRint("XXX", 0, 0);

    // Create Eve manager
    TEveManager *manager = TEveManager::Create(kTRUE, "FV");

    // Get Eve browser and embed it
    TEveBrowser *browser = manager->GetBrowser ();
    browser->StartEmbedding(TRootBrowser::kLeft);

    // Create the main event display class
    EventDisplay display(manager, geometryFile, lcioFileList, excludeColls, bY, verbose);

    browser->SetTabTitle("Event Control", 0);
    browser->StopEmbedding();

    manager->FullRedraw3D();

    app->Run(kFALSE);

    return 0;
}
