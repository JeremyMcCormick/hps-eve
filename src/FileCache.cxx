#include "FileCache.h"

#include <sys/stat.h>
#include <iostream>
#include <fstream>

namespace hps {

    FileCache::FileCache(std::string cacheDir) :
            Logger("FileCache"),
            cacheDir_(cacheDir) {
    }

    FileCache::~FileCache() {
    }

    bool FileCache::isCached(const std::string& fileName) {
        std::ifstream cacheFile;
        cacheFile.open(getCachedPath(fileName));
        return cacheFile.good();
    }

    std::string FileCache::getCachedPath(const std::string& fileName) {
        return cacheDir_  + "/" + fileName;
    }

    void FileCache::createCacheDir() {
        log("Creating cache directory: " + cacheDir_, INFO);
        int check = mkdir(cacheDir_.c_str(), 0755);
        if (!check) {
            log("Created cache directory!", INFO);
        } else {
            if (errno != EEXIST) {
                throw std::runtime_error("Failed to create cache directory.");
            }
        }
    }
}
