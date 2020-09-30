#ifndef HPS_FILECACHE_H_
#define HPS_FILECACHE_H_ 1

// HPS
#include "Logger.h"

// C++ standard library
#include <string>

#ifdef HAVE_CURL

static size_t _write_data(void *ptr, size_t size, size_t nmemb, void *stream);

static void _download(const char* url, const char* filename);

#endif

namespace hps {

    class FileCache : public Logger {

        public:

            FileCache(std::string cacheDir);

            virtual ~FileCache();

            void createCacheDir();

            bool isCached(const std::string& fileName);

            std::string getCachedPath(const std::string& fileName);

            void cache(const char* url, const char* outfile);

        private:

            std::string cacheDir_;

    };
}

#endif
