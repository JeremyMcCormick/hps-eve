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
    std::cout << "    -c [cache dir]" << std::endl;
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
    std::set<std::string> excludeColls;
    std::string cacheDir(".cache");
    int verbose = 0;
    double bY = 0.0;

    int c = 0;
    while ((c = getopt (argc, argv, "b:e:g:v:c:")) != -1) {
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

    // In case curl and libxml2 were not both enabled, a GDML file must be provided on the command line.
#if !defined(HAVE_CURL) || !defined(HAVE_LIBXML2)
    if (geometryFile.length () == 0) {
        print_usage("ERROR: Missing name of geometry file (provide with '-g' switch or compile with curl/libxml2 support)");
    }
#endif

    if (lcioFileList.size () == 0) {
        print_usage("ERROR: Missing one or more LCIO files (provide as extra arguments)");
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
    std::cout << "    cache dir: " << cacheDir << std::endl;
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
    EventDisplay* ed = EventDisplay::createEventDisplay(manager,
                                                        geometryFile,
                                                        cacheDir,
                                                        lcioFileList,
                                                        excludeColls,
                                                        bY);
    ed->setLevel(verbose);

    browser->SetTabTitle("Event Control", 0);
    browser->StopEmbedding();

    manager->FullRedraw3D();

    app->Run(kFALSE);

    delete ed;

    return 0;
}
