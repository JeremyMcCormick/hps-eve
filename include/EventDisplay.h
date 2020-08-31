#include <vector>
#include <string>

#include "TGFrame.h"
#include "TEveManager.h"

namespace HPS {

    class EventDisplay : public TGMainFrame {

        public:

            EventDisplay(TEveManager* manager,
                         std::string geometryFile,
                         std::vector<std::string> lcioFileList);

        private:

            TEveManager* manager_;

            std::vector<std::string> lcioFileList_;
            std::string geometryFile_;
    };
}
