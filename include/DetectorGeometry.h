#ifndef HPS_DETECTORGEOMETRY_H_
#define HPS_DETECTORGEOMETRY_H_ 1

// HPS
#include "Logger.h"

// ROOT
#include "TGeoManager.h"
#include "TEveManager.h"
#include "TEveGeoNode.h"
#include "TEveElement.h"

// C++ standard library
#include <map>

#ifdef HAVE_LIBXML2

static void extractGdmlFile(const char* lcddName, const char* gdmlName);

#endif

namespace hps {

    class EventDisplay;
    class FileCache;

    class DetectorGeometry : public Logger {

        public:

            DetectorGeometry(EventDisplay* app, FileCache* cache);

            ~DetectorGeometry();

            TGeoManager* getGeoManager();

            /**
             * Utility function to convert a single TGeoNode into an Eve element.
             */
            static TEveElement* toEveElement(TGeoManager* mgr, TGeoNode* node);

            void loadDetector(const std::string& detName);

            void loadDetectorFile(const std::string& gdmlName);

            bool isInitialized();

        private:

            void buildDetector();

            /**
             * Create a list of Eve geometry elements from the children of a
             * single volume specified by a path.
             */
            static TEveElementList* createGeoElements(TGeoManager*,
                                                      const char* name,
                                                      const char* path,
                                                      const char* patt,
                                                      Char_t transparency = 100);

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

            FileCache* fileCache_;
    };
}

#endif

