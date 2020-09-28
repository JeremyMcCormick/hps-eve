#include "EventManager.h"

// HPS
#include "DetectorGeometry.h"
#include "EventDisplay.h"

// LCIO
#include "IOIMPL/LCFactory.h"

ClassImp(hps::EventManager);

namespace hps {

    EventManager::EventManager(EventDisplay* app) :
            Logger("EventManager"),
            TEveEventManager("HPS Event Manager", ""),
            reader_(nullptr),
            event_(new EventObjects(app)),
            app_(app) {

        // Set log level from main application.
        setLogLevel(app_->getLogLevel());
        event_->setLogLevel(getLogLevel());
    }

    EventManager::~EventManager() {
        delete event_;
    }

    void EventManager::Open() {

        // Open the LCIO reader.

        log("Opening reader... ", INFO);

        reader_ = IOIMPL::LCFactory::getInstance()->createLCReader(IO::LCReader::directAccess);
        reader_->open(app_->getLcioFiles());
        auto runHeader = reader_->readNextRunHeader();
        std::string detName;

        if (runHeader != nullptr) {
            // Get run number and detector name from run header that was found.
            // We are assuming run header was first record and reader does not need to be reset.
            runNumber_ = runHeader->getRunNumber();
            detName = runHeader->getDetectorName();
        } else if (runHeader == nullptr) {
            // Get run number and detector name from first event in file, as no run header was found.
            // Then reset the reader by closing and reopening it.
            log("Setting run number from first event ...");

            auto event = reader_->readNextEvent();
            runNumber_ = event->getRunNumber();
            detName = event->getDetectorName();
            reader_->close();
            reader_->open(app_->getLcioFiles());
            log("Done setting run number from first event!");

        }
        if (runNumber_ < 0) {
            // Run number was not found or it is invalid.
            // This could break random IO with the reader but continue anyways.
            log("Run number was not set!", ERROR);
        } else {
            log(INFO) << "Run number set to: " << runNumber_ << std::endl;
        }
        log("Done opening reader!", INFO);

        // Initialize the detector geometry if this has not been done already.
        if (!app_->getDetectorGeometry()->isInitialized()) {
            if (detName.size()) {
                app_->getDetectorGeometry()->loadDetector(detName);
            } else {
                // No detector name was found to load geometry so crash the application.
                log("Failed to get detector name from LCIO file!", ERROR);
                throw std::runtime_error("Failed to get detector name from LCIO file!");
            }
        }
    }

    void EventManager::NextEvent() {
        GotoEvent(eventNum_ + 1);
    }

    void EventManager::loadEvent(EVENT::LCEvent* event) {

        // Destroy previous event and load the next one.
        app_->getEveManager()->GetCurrentEvent()->DestroyElements();
        log() << "Loading LCIO event: " << event->getEventNumber() << std::endl;
        event_->build(app_->getEveManager(), event);
        log("Done loading event!");
    }

    void EventManager::GotoEvent(Int_t i) {

        Logger::flushLoggers();

        log(INFO) << "GotoEvent: " << i << std::endl;

        if (i < 0) {
            log(ERROR) << "Event number is not valid: " << i << std::endl;
            return;
        } else if (i == eventNum_) {
            log(ERROR) << "Event is already loaded: " << i << std::endl;
            return;
        }
        EVENT::LCEvent* event = nullptr;
        if (i == (eventNum_ + 1)) {

            log(FINE) << "Reading next event" << std::endl;

            try {
                event = reader_->readNextEvent();
            } catch (IO::IOException& ioe) {
                log(ERROR) << ioe.what() << std::endl;
            } catch (std::exception& e) {
                log(ERROR) << e.what() << std::endl;
            }
        } else {
            log(FINER) << "Seeking event " << i
                    << " with run number " << runNumber_ << std::endl;
            try {
                event = reader_->readEvent(runNumber_, i);
                if (event == nullptr) {
                    log(ERROR) << "Seeking failed!" << std::endl;
                }
            } catch (IO::IOException& ioe) {
                log(ERROR) << ioe.what() << std::endl;
            } catch (std::exception& e) {
                log(ERROR) << e.what() << std::endl;
            }
        }
        if (event != nullptr) {
            loadEvent(event);
            eventNum_ = i;
        } else {
            log(ERROR) << "Failed to read next event!" << std::endl;
        }
        app_->getEveManager()->FullRedraw3D(false);
    }

    void EventManager::PrevEvent() {
        log(FINE) << "PrevEvent" << std::endl;
        if (eventNum_ > 0) {
            GotoEvent(eventNum_ - 1);
        } else {
            log(WARNING) << "Already at first event!" << std::endl;
        }
    }

    void EventManager::SetEventNumber() {
        log(FINE) << "Set event number: " << app_->getCurrentEventNumber() << std::endl;
        if (app_->getCurrentEventNumber() > -1) {
            GotoEvent(app_->getCurrentEventNumber());
        } else {
            log(ERROR) << "Event number is not valid: "
                    << app_->getCurrentEventNumber() << std::endl;

        }
    }

    void EventManager::setLogLevel(int verbosity) {
        Logger::setLogLevel(verbosity);
        event_->setLogLevel(verbosity);
    }

    void EventManager::modifyPCut() {
        // Forward P cut to EventObjects.
        event_->setPCut(app_->getPCut()); // @suppress("Ambiguous problem")

        // Redraw the scene.
        app_->getEveManager()->FullRedraw3D(false);
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
