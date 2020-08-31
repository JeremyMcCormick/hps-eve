
#include "TEveEventManager.h"

#include <iostream>

namespace hps {

    class EventManager : public TEveEventManager {

        public:
            EventManager() : TEveEventManager("HPS Event", "") {
            }

            void NextEvent() {
                std::cout << "EventManager::NextEvent()" << std::endl;
            }
    };
}
