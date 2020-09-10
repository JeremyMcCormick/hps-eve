#include "EventManager.h"

#include "IOIMPL/LCFactory.h"

#include "EventDisplay.h"

ClassImp(hps::EventManager);

namespace hps {

    EventManager::EventManager(EventDisplay* app) :
            TEveEventManager("HPS Event Manager", ""),
            reader_(nullptr),
            event_(new EventObjects(app)),
            app_(app) {
    }

    EventManager::~EventManager() {
        delete event_;
    }

    void EventManager::Open() {
        if (checkVerbosity(1)) {
            std::cout << "[ EventManager ] Opening reader... " << std::endl;
        }
        reader_ = IOIMPL::LCFactory::getInstance()->createLCReader(IO::LCReader::directAccess);
        reader_->open(app_->getLcioFiles());
        auto runHeader = reader_->readNextRunHeader();
        auto detName = runHeader->getDetectorName();

        if (!app_->getDetectorGeometry()->isInitialized()) {
            app_->getDetectorGeometry()->loadDetector(detName);
        }

        if (runHeader != nullptr) {
            runNumber_ = runHeader->getRunNumber();
        } else if (runHeader == nullptr) {
            if (checkVerbosity(1)) {
                std::cout << "[ EventManager ] Setting run number from first event ..." << std::endl;
            }
            auto event = reader_->readNextEvent();
            runNumber_ = event->getRunNumber();
            reader_->close();
            reader_->open(app_->getLcioFiles());
            if (checkVerbosity()) {
                std::cout << "[ EventManager ] Done setting run number from first event!" << std::endl;
            }
        }
        if (checkVerbosity(1)) {
            std::cout << "[ EventManager ] Run number set to: " << runNumber_ << std::endl;
            std::cout << "[ EventManager ] Done opening reader!" << std::endl;
        }
    }

    void EventManager::NextEvent() {
        GotoEvent(eventNum_ + 1);
    }

    void EventManager::loadEvent(EVENT::LCEvent* event) {
        app_->getEveManager()->GetCurrentEvent()->DestroyElements();
        if (checkVerbosity()) {
            std::cout << "[ EventManager ] Loading LCIO event: " << event->getEventNumber() << std::endl;
        }
        event_->build(app_->getEveManager(), event);
        if (checkVerbosity()) {
            std::cout << "[ EventManager ] Done loading event!" << std::endl;
        }
    }

    void EventManager::GotoEvent(Int_t i) {
        if (checkVerbosity()) {
            std::cout << "[ EventManager ] GotoEvent: " << i << std::endl;
        }
        if (i < 0) {
            std::cerr << "[ EventManager ] Event number is not valid: " << i << std::endl;
            return;
        } else if (i == eventNum_) {
            std::cerr << "[ EventManager ] Event is already loaded: " << i << std::endl;
            return;
        }
        EVENT::LCEvent* event = nullptr;
        if (i == (eventNum_ + 1)) {
            if (checkVerbosity()) {
                std::cout << "[ EventManager ] Reading next event" << std::endl;
            }
            try {
                event = reader_->readNextEvent();
            } catch (IO::IOException& ioe) {
                std::cerr << "[ EventManager ] [ ERROR ] " << ioe.what() << std::endl;
            } catch (std::exception& e) {
                std::cerr << "[ EventManager ] [ ERROR ] " << e.what() << std::endl;
            }
        } else {
            if (checkVerbosity()) {
                std::cout << "[ EventManager ] Seeking event " << i
                        << " with run number " << runNumber_ << std::endl;
            }
            try {
                event = reader_->readEvent(runNumber_, i);
                if (event == nullptr) {
                    std::cerr << "[ EventManager ] [ ERROR ] Seeking failed!" << std::endl;
                }
            } catch (IO::IOException& ioe) {
                std::cerr << "[ EventManager ] [ ERROR ] " << ioe.what() << std::endl;
            } catch (std::exception& e) {
                std::cerr << "[ EventManager ] [ ERROR ] " << e.what() << std::endl;
            }
        }
        if (event != nullptr) {
            loadEvent(event);
            eventNum_ = i;
        } else {
            std::cerr << "[ EventManager ] [ ERROR ] Failed to read next event!" << std::endl;
        }
        app_->getEveManager()->FullRedraw3D(false);
    }

    void EventManager::PrevEvent() {
        if (checkVerbosity()) {
            std::cout << "[ EventManager ] PrevEvent" << std::endl;
        }
        if (eventNum_ > 0) {
            GotoEvent(eventNum_ - 1);
        } else {
            std::cerr << "[ EventManager ] [ ERROR ] Already at first event!" << std::endl;
        }
    }

    void EventManager::SetEventNumber() {
        if (checkVerbosity()) {
            std::cout << "[ EventManager ] Set event number: " << app_->getCurrentEventNumber() << std::endl;
        }
        if (app_->getCurrentEventNumber() > -1) {
            GotoEvent(app_->getCurrentEventNumber());
        } else {
            std::cerr << "[ EventManager ] [ ERROR ] Event number is not valid: "
                    << app_->getCurrentEventNumber() << std::endl;

        }
    }

    void EventManager::setVerbosity(int verbosity) {
        Verbosity::setVerbosity(verbosity);
        event_->setVerbosity(verbosity);
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
