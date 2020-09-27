#ifndef HPS_EVENTDISPLAY_H_
#define HPS_EVENTDISPLAY_H_ 1

#include <vector>
#include <string>

// HPS
#include "TGFrame.h"
#include "TEveManager.h"
#include "TGNumberEntry.h"
#include "Logger.h"

namespace hps {

    class EventManager;
    class DetectorGeometry;

    class EventDisplay : public TGMainFrame, public Logger {

        public:

            virtual ~EventDisplay();

            /**
             * Get current event number from GUI component.
             */
            int getCurrentEventNumber();

            EventManager* getEventManager();

            TEveManager* getEveManager();

            DetectorGeometry* getDetectorGeometry();

            void setLogLevel(int verbosity);

            double getMagFieldY();

            const std::vector<std::string>& getLcioFiles();

            bool excludeCollection(const std::string& collName);

            static EventDisplay* createEventDisplay(TEveManager* manager,
                                                    std::string geometryFile,
                                                    std::string cacheDir,
                                                    std::vector<std::string> lcioFileList,
                                                    std::set<std::string> excludeColls,
                                                    double bY,
                                                    int logLevel);

            static EventDisplay* getInstance();

            double getPCut();

        private:

            EventDisplay(TEveManager* manager,
                         std::string geometryFile,
                         std::string cacheDir,
                         std::vector<std::string> lcioFileList,
                         std::set<std::string> excludeColls,
                         double bY,
                         int logLevel);

        private:

            TEveManager* eveManager_;
            EventManager* eventManager_;
            DetectorGeometry* det_;

            std::vector<std::string> lcioFileList_;
            std::set<std::string> excludeColls_;

            double bY_;

            TGNumberEntry* eventNumberEntry_;

            TGNumberEntry* PTCutEntry_;

            static EventDisplay* instance_;

            ClassDef(EventDisplay, 1);
    };
}

#endif
