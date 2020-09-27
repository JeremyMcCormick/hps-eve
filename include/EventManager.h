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

    class EventDisplay;
    class EventObjects;

    class EventManager : public TEveEventManager, public Logger {

        public:

            EventManager(EventDisplay* app);

            virtual ~EventManager();

            void Open();

            void GotoEvent(Int_t i);
            void NextEvent();
            void PrevEvent();

            /**
             * Set event number from EventDisplay GUI.
             */
            void SetEventNumber();

            void setLogLevel(int verbosity);

            /**
             * Modify the MCParticle P cut according to GUI setting.
             */
            void modifyPCut();

            /*
            void Close();
            void AfterNewEventLoaded();
            void AddNewEventCommand(const TString& cmd);
            void RemoveNewEventCommand(const TString& cmd);
            void ClearNewEventCommands();
            */

        private:

            void loadEvent(EVENT::LCEvent* event);

            /*bool getTreeSelections(std::set<std::string>& selections);*/

        private:

            IO::LCReader* reader_;

            EventDisplay* app_;
            EventObjects* event_;

            int runNumber_{-1};
            int eventNum_{-1};
            //int maxEvents_{999999};

            ClassDef(EventManager, 1);
    };
}

#endif
