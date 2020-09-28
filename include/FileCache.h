#ifndef HPS_FILECACHE_H_
#define HPS_FILECACHE_H_ 1

// HPS
#include "Logger.h"

// C++ standard library
#include <string>

namespace hps {

    class FileCache : public Logger {

        public:

            FileCache(std::string cacheDir);

            virtual ~FileCache();

            void createCacheDir();

            bool isCached(const std::string& fileName);

            std::string getCachedPath(const std::string& fileName);

        private:

            std::string cacheDir_;

    };
}

#endif
