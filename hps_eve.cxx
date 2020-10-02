// HPS
#include "EventDisplay.h"

// ROOT
#include "TRint.h"
#include "TEveBrowser.h"

// C++ standard library
#include <string>
#include <vector>
#include <set>
#include <iostream>

using hps::EventDisplay;

void print_usage(const char* msg = 0, bool doExit = true, int returnCode = 1) {
    std::cout << "Usage: hps-eve [args] [LCIO files]" << std::endl;
    std::cout << "    -g [gdml] : Path to GDML file" << std::endl;
    std::cout << "    -b [bY] : Fixed BY value" << std::endl;
    std::cout << "    -l [level] : Log level (0-6)" << std::endl;
    std::cout << "    -e [collection] : LCIO collection to exclude by name" << std::endl;
    std::cout << "    -c [directory] : Path to cache directory which will be created" << std::endl;
    std::cout << "GDML file is required if curl and libxml2 were not enabled." << std::endl;
    std::cout << "One or more LCIO files are required." << std::endl;
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
    std::set<std::string> excludeCollectionNames;
    std::set<std::string> excludeCollectionTypes;
    std::string cacheDir(".cache");
    int logLevel = hps::ERROR;
    double bY = 0.0;

    int c = 0;
    while ((c = getopt (argc, argv, "b:e:g:l:c:")) != -1) {
        switch (c) {
            case 'g':
                geometryFile = std::string(optarg);
                break;
            case 'e':
                excludeCollectionNames.insert(std::string(optarg));
                break;
            case 't':
                excludeCollectionTypes.insert(std::string(optarg));
            case 'b':
                bY = std::stod(optarg);
                break;
            case 'l':
                logLevel = atoi(optarg);
                break;
            case 'c':
                cacheDir = std::string(optarg);
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

    // If curl and libxml2 were not both enabled, a GDML file must be provided on the command line.
#if !defined(HAVE_CURL) || !defined(HAVE_LIBXML2)
    if (geometryFile.length () == 0) {
        print_usage("ERROR: Missing path to geometry file (provide with '-g' switch or enable curl and libxml2)");
    }
#endif

    if (lcioFileList.size () == 0) {
        print_usage("ERROR: Missing one or more LCIO files (provide as extra arguments)");
    }

    // Create ROOT interpreter.
    TRint *app = 0;
    app = new TRint("XXX", 0, 0);

    // Create Eve manager.
    TEveManager* manager = TEveManager::Create(kTRUE, "FV");

    // Get Eve browser and start embedding.
    TEveBrowser *browser = manager->GetBrowser();
    browser->StartEmbedding(TRootBrowser::kLeft);

    // Create the application and configure from command line arguments.
    EventDisplay* ed = EventDisplay::getInstance();
    ed->setLogLevel(logLevel);
    ed->setEveManager(manager);
    ed->setGeometryFile(geometryFile);
    ed->setCacheDir(cacheDir);
    ed->addLcioFiles(lcioFileList);
    ed->addExcludeCollectionNames(excludeCollectionNames);
    ed->addExcludeCollectionTypes(excludeCollectionTypes);
    ed->setMagFieldY(bY);
    ed->initialize();

    // Post-initialization of the Eve components.
    browser->SetTabTitle("Event Control", 0);
    browser->StopEmbedding();
    ed->getEveManager()->FullRedraw3D();

    app->Run(kFALSE);
    delete ed;
    return 0;
}
