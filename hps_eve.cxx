#include <iostream>
#include "EventDisplay.h"

using HPS::EventDisplay;

int main(int argc, char** argv) {
    std::cout << "hello hps eve" << std::endl;
    EventDisplay* app = new EventDisplay();
    app->parseArgs(argc, argv);

}
