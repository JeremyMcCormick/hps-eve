#include <vector>
#include <string>

namespace HPS {

    class EventDisplay {

        public:

            void parseArgs (int argc, char **argv);

        private:

            std::vector<std::string> lcioFileList_;
            std::string geometryFile;
    };
}
