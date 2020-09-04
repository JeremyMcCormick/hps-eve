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

    class EventDisplay : public TGMainFrame {

        public:

            EventDisplay(TEveManager* manager,
                         std::string geometryFile,
                         std::vector<std::string> lcioFileList,
                         std::set<std::string> excludeColls,
                         int verbose = 0);

            ~EventDisplay();

            void GotoEvent(Int_t i);

            /**
             * Get current event number from GUI component.
             */
            int getCurrentEventNumber();

        private:

            TEveManager* manager_;
            TEveEventManager* eventManager_;
            DetectorGeometry* geo_;

            std::vector<std::string> lcioFileList_;
            std::string geometryFile_;

            TGNumberEntry* eventNumberEntry_;

            int verbose_;

            ClassDef(EventDisplay, 1);
    };
}

#endif
