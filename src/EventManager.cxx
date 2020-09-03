#include "EventManager.h"

#include "IOIMPL/LCFactory.h"

ClassImp(hps::EventManager);

namespace hps {

    EventManager::EventManager(TEveManager* eve, std::vector<std::string> fileNames) :
            TEveEventManager("HPS Event Manager", ""),
            eve_(eve),
            fileNames_(fileNames),
            reader_(nullptr) {
    }

    EventManager::~EventManager() {
    }

    void EventManager::Open() {
        std::cout << "Opening reader... " << std::endl;
        reader_ = IOIMPL::LCFactory::getInstance()->createLCReader();
        reader_->open(this->fileNames_);
        std::cout << "Done opening reader!" << std::endl;
    }

    void EventManager::NextEvent() {
        std::cout << "Reading next LCIO event..." << std::endl;

        // Destroy the Eve elements from prior event.
        eve_->GetCurrentEvent()->DestroyElements();

        EVENT::LCEvent* event = reader_->readNextEvent();
        event_.setEvent(event);
        event_.build();
        auto elements = event_.getElementList();
        for (std::vector<TEveElement*>::const_iterator it = elements.begin();
                it != elements.end();
                it++) {
            std::cout << "Adding element to event: " << (*it)->GetElementName() << std::endl;
            eve_->AddElement(*it);
        }

        std::cout << "Read LCIO event: " << event->getEventNumber() << std::endl;
    }

    void EventManager::GotoEvent(Int_t i) {
        std::cout << "EventManager::GotoEvent - " << i << std::endl;
    }

    void EventManager::PrevEvent() {
        std::cout << "EventManager::PrevEvent" << std::endl;
    }

    void EventManager::Close() {
        std::cout << "EventManager::Close" << std::endl;
    }

    void EventManager::AfterNewEventLoaded() {
        std::cout << "EventManager::AfterNewEventLoaded" << std::endl;
    }

    void EventManager::AddNewEventCommand(const TString& cmd) {
        std::cout << "EventManager::AddNewEventCommand - " << cmd << std::endl;
    }

    void EventManager::RemoveNewEventCommand(const TString& cmd) {
        std::cout << "EventManager::RemoveNewEventCommand - " << cmd << std::endl;
    }

    void EventManager::ClearNewEventCommands() {
        std::cout << "EventManager::ClearNewEventCommands" << std::endl;
    }
}
