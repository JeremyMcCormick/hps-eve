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

    class EventManager : public TEveEventManager, public Verbosity {

        public:

            EventManager(TEveManager* eve,
                         DetectorGeometry* det,
                         EventDisplay* app,
                         std::vector<std::string> fileNames,
                         std::set<std::string> excludeColls,
                         double bY);

            virtual ~EventManager();

            void Open();

            void GotoEvent(Int_t i);
            void NextEvent();
            void PrevEvent();

            /**
             * Set event number from EventDisplay GUI.
             */
            void SetEventNumber();

            void setVerbosity(int verbosity);

            /*
            void Close();
            void AfterNewEventLoaded();
            void AddNewEventCommand(const TString& cmd);
            void RemoveNewEventCommand(const TString& cmd);
            void ClearNewEventCommands();
            */

        private:

            void loadEvent(EVENT::LCEvent* event);

            TEveManager* eve_;
            //TGeoManager* geo_;
            DetectorGeometry* det_;
            EventDisplay* app_;

            IO::LCReader* reader_;
            std::vector<std::string> fileNames_;
            std::set<std::string> excludeColls_;

            int runNumber_{-1};

            EventObjects* event_;

            int eventNum_{-1};
            //int maxEvents_{999999};

            ClassDef(EventManager, 1);
    };
}

#endif
