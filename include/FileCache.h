#ifndef HPS_FILECACHE_H_
#define HPS_FILECACHE_H_ 1

// HPS
#include "Verbosity.h"

// C++ standard library
#include <string>

namespace hps {

    class FileCache : public Verbosity {

        public:

            FileCache(std::string cacheDir);

            virtual ~FileCache();

            bool isCached(const std::string& fileName);

            std::string getCachedPath(const std::string& fileName);

        private:

            void createCacheDir();

        private:

            std::string cacheDir_;

    };
}

#endif
