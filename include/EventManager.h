#ifndef HPS_EVENTMANAGER_H_
#define HPS_EVENTMANAGER_H_ 1

// HPS
#include "EventObjects.h"

// C++ standard library
#include <iostream>

// ROOT
#include "TEveEventManager.h"

// LCIO
#include "EVENT/LCIO.h"
#include "IO/LCReader.h"

namespace hps {

    class EventManager : public TEveEventManager {

        public:

            EventManager(TEveManager* eve, TGeoManager* geo, std::vector<std::string> fileNames);

            ~EventManager();

            void Open();

            void GotoEvent(Int_t i);
            void NextEvent();
            void PrevEvent();
            void Close();

            void AfterNewEventLoaded();

            void AddNewEventCommand(const TString& cmd);
            void RemoveNewEventCommand(const TString& cmd);
            void ClearNewEventCommands();

        private:

            TEveManager* eve_;
            TGeoManager* geo_;
            IO::LCReader* reader_;
            std::vector<std::string> fileNames_;

            EventObjects* event_;

            ClassDef(EventManager, 1);
    };
}

#endif
