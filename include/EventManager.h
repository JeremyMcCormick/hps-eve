
#include "TEveEventManager.h"

#include <iostream>

#include "EVENT/LCIO.h"
#include "IO/LCReader.h"

namespace hps {

    class EventManager : public TEveEventManager {

        public:
            EventManager(std::vector<std::string> fileNames);

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

            IO::LCReader* reader_;
            std::vector<std::string> fileNames_;
    };
}
