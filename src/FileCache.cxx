#include "FileCache.h"

#include <sys/stat.h>
#include <iostream>
#include <fstream>

#ifdef HAVE_CURL

#include <curl/curl.h>

size_t _write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

void _download(const char* url, const char* outfile)
{
    CURL *curl;
    CURLcode res;
    FILE *pagefile;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    /* send all data to this function  */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_data);

    /* open the file */
    pagefile = fopen(outfile, "wb");
    if(pagefile) {

        /* write the page body to this file handle */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, pagefile);

        /* get it! */
        curl_easy_perform(curl);

        /* close the header file */
        fclose(pagefile);
     }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl);

    curl_global_cleanup();
}

#endif

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

    void FileCache::cache(const char* url, const char* outfile) {
        log(INFO) << "Downloading: " << url << " -> " << outfile << std::endl;
#ifdef HAVE_CURL
        _download(url, getCachedPath(outfile).c_str());
#else
        // Should never get here but raise an error anyways.
        throw std::runtime_error("curl is not enabled!");
#endif
    }

}
