#ifndef HPS_DETECTORGEOMETRY_H_
#define HPS_DETECTORGEOMETRY_H_ 1

// C++ standard library
#include <map>

// ROOT
#include "TGeoManager.h"
#include "TEveManager.h"
#include "TEveGeoNode.h"
#include "TEveElement.h"

#ifdef HAVE_CURL

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);

static void download(const char* url, const char* filename);

#endif

#ifdef HAVE_LIBXML2

#include <libxml/xmlreader.h>

static void extractGdmlFile(const char* lcddName, const char* gdmlName);

#endif

namespace hps {

    class DetectorGeometry {

        public:

            DetectorGeometry(TEveManager* eve, int verbose = 0);

            /**
             * Utility function to convert a single TGeoNode into an Eve element.
             */
            static TEveElement* toEveElement(TGeoManager* mgr, TGeoNode* node);

            TGeoManager* getGeoManager();

            void loadDetector(const std::string& detName);

            void loadDetectorFile(const std::string& gdmlName);

        private:

            void buildDetector();

            /**
             * Create a list of Eve geometry elements from the children of a
             * single volume specified by a path.
             */
            static TEveElementList* createGeoElements(TGeoManager*,
                                                      const char* name,
                                                      const char* path,
                                                      const char* patt);

            /**
             * Add the SVT to Eve.
             */
            void addTracker();

            /**
             * Add the ECAL to Eve.
             */
            void addEcal();

        private:

            TGeoManager* geo_;
            TEveManager* eve_;

            std::string BASE_DETECTOR_URL{
                "https://raw.githubusercontent.com/JeffersonLab/hps-java/master/detector-data/detectors"};

            int verbose_;
    };
}

#endif

