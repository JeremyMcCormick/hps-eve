#include "FileCache.h"

#include <sys/stat.h>
#include <iostream>
#include <fstream>

namespace hps {

    FileCache::FileCache(std::string cacheDir) : cacheDir_(cacheDir) {
        setVerbosity(1); // Hard-coded to print all its log messages for now.
        createCacheDir();
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
        if (checkVerbosity()) {
            std::cout << "[ FileCache ] Creating cache dir: " << cacheDir_ << std::endl;
        }
        int check = mkdir(cacheDir_.c_str(), 0755);
        if (!check) {
            if (checkVerbosity()) {
                std::cout << "[ FileCache ] Created cache dir!" << std::endl;
            }
        } else {
            if (errno == EEXIST) {
                if (checkVerbosity()) {
                    std::cout << "[ FileCache ] Cache dir already exists!" << std::endl;
                }
            } else {
                throw std::runtime_error("Failed to create cache dir.");
            }
        }
    }
}
