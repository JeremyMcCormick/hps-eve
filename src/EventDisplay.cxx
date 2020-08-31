#include "EventDisplay.h"

#include <unistd.h>
#include <iostream>
#include <stdexcept>

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
}
