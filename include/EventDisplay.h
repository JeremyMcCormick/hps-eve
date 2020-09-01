#include <vector>
#include <string>

// ROOT
#include "TGFrame.h"
#include "TEveManager.h"

namespace hps {

    class EventDisplay : public TGMainFrame {

        public:

            EventDisplay(TEveManager* manager,
                         std::string geometryFile,
                         std::vector<std::string> lcioFileList);

            ~EventDisplay();

            void NextEvent();

        private:

            TEveManager* manager_;
            TEveEventManager* eventManager_;

            std::vector<std::string> lcioFileList_;
            std::string geometryFile_;

            ClassDef(EventDisplay, 1);
    };
}
