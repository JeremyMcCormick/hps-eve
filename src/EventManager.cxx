#include "EventManager.h"

#include "IOIMPL/LCFactory.h"

#include "EventDisplay.h"

ClassImp(hps::EventManager);

namespace hps {

    EventManager::EventManager(TEveManager* eve,
                               TGeoManager* geo,
                               EventDisplay* app,
                               std::vector<std::string> fileNames) :
            TEveEventManager("HPS Event Manager", ""),
            eve_(eve),
            geo_(geo),
            fileNames_(fileNames),
            reader_(nullptr),
            event_(new EventObjects(geo)),
            app_(app) {
    }

    EventManager::~EventManager() {
        delete event_;
    }

    void EventManager::Open() {
        std::cout << "[ EventManager ]: Opening reader... " << std::endl;
        reader_ = IOIMPL::LCFactory::getInstance()->createLCReader();
        reader_->open(this->fileNames_);
        std::cout << "[ EventManager ]: Done opening reader!" << std::endl;
    }

    void EventManager::NextEvent() {
        std::cout << "[ EventManager ]: Reading next LCIO event..." << std::endl;

        // Destroy the Eve elements from the prior event.
        eve_->GetCurrentEvent()->DestroyElements();

        EVENT::LCEvent* event = reader_->readNextEvent();
        std::cout << "[ EventManager ] : Read LCIO event: " << event->getEventNumber() << std::endl;

        std::cout << "[ EventManager ] : Converting event to Eve..." << std::endl;
        event_->build(eve_, event);
        std::cout << "[ EventManager ] : Done converting event!" << std::endl;
        std::cout << std::endl;
    }

    void EventManager::GotoEvent(Int_t i) {
        std::cout << "[ EventManager ] : GotoEvent: " << i << std::endl;
    }

    void EventManager::PrevEvent() {
        std::cout << "[ EventManager ] : PrevEvent" << std::endl;
    }

    void EventManager::SetEventNumber() {
        std::cout << "[ EventManager ] : Set event number from GUI: " << app_->getCurrentEventNumber() << std::endl;
    }

    /*
    void EventManager::Close() {
        std::cout << "[ EventManager ] : Close" << std::endl;
    }

    void EventManager::AfterNewEventLoaded() {
        std::cout << "[ EventManager ] : AfterNewEventLoaded" << std::endl;
    }

    void EventManager::AddNewEventCommand(const TString& cmd) {
        std::cout << "[ EventManager ] : AddNewEventCommand - " << cmd << std::endl;
    }

    void EventManager::RemoveNewEventCommand(const TString& cmd) {
        std::cout << "[ EventManager ] : RemoveNewEventCommand - " << cmd << std::endl;
    }

    void EventManager::ClearNewEventCommands() {
        std::cout << "[ EventManager ] : ClearNewEventCommands" << std::endl;
    }
    */
}
