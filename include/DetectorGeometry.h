#ifndef HPS_DETECTORGEOMETRY_H_
#define HPS_DETECTORGEOMETRY_H_ 1

// C++ standard library
#include <map>

// ROOT
#include "TGeoManager.h"
#include "TEveManager.h"
#include "TEveGeoNode.h"
#include "TEveElement.h"

namespace hps {

    class DetectorGeometry {

        public:

            DetectorGeometry(TGeoManager* geo, TEveManager* eve, int verbose = 0);

            /**
             * Utility function to convert a single TGeoNode into an Eve element.
             */
            static TEveElement* toEveElement(TGeoManager* mgr, TGeoNode* node);

        private:

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

            int verbose_;
    };
}

#endif

