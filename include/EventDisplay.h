#ifndef HPS_EVENTDISPLAY_H_
#define HPS_EVENTDISPLAY_H_ 1

#include <vector>
#include <string>

// ROOT
#include "TGFrame.h"
#include "TEveManager.h"
#include "TGNumberEntry.h"

// HPS
#include "DetectorGeometry.h"

namespace hps {

    class EventManager;

    class EventDisplay : public TGMainFrame, public Verbosity {

        public:

            EventDisplay(TEveManager* manager,
                         std::string geometryFile,
                         std::string cacheDir,
                         std::vector<std::string> lcioFileList,
                         std::set<std::string> excludeColls,
                         double bY);

            virtual ~EventDisplay();

            /**
             * Get current event number from GUI component.
             */
            int getCurrentEventNumber();

            EventManager* getEventManager();

            TEveManager* getEveManager();

            DetectorGeometry* getDetectorGeometry();

            void setVerbosity(int verbosity);

            double getMagFieldY();

            const std::vector<std::string>& getLcioFiles();

            bool excludeCollection(const std::string& collName);

        private:

            TEveManager* eveManager_;
            EventManager* eventManager_;
            DetectorGeometry* det_;

            std::vector<std::string> lcioFileList_;
            std::set<std::string> excludeColls_;

            double bY_;

            TGNumberEntry* eventNumberEntry_;

            ClassDef(EventDisplay, 1);
    };
}

#endif
