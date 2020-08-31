#include <vector>
#include <string>

namespace HPS {

    class EventDisplay {

        public:

            /** Parse command line arguments. */
            void parseArgs (int argc, char **argv);

            /** Run and return the return code. */
            int run();

        private:

            std::vector<std::string> lcioFileList_;
            std::string geometryFile;
    };
}
