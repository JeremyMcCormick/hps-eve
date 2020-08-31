#include <iostream>
#include "EventDisplay.h"

using HPS::EventDisplay;

int main(int argc, char** argv) {
    EventDisplay* eventDisplay = new EventDisplay();
    eventDisplay->parseArgs(argc, argv);
    return eventDisplay->run();
}
