#ifndef HPS_LCOBJECTUSERDATA_H_
#define HPS_LCOBJECTUSERDATA_H_ 1

#include "EVENT/LCObject.h"
#include "EVENT/MCParticle.h"

using EVENT::LCObject;
using EVENT::MCParticle;

namespace hps {

    class LCObjectUserData {

        public:

            LCObjectUserData(LCObject* object) : object_(object) {
            }

            ~LCObjectUserData() {
            }

            inline LCObject* getLCObject() {
                return object_;
            }

        protected:

            LCObject* object_;

    };


    class MCParticleUserData : public LCObjectUserData {

        public:

            MCParticleUserData(LCObject* object, double p) :
                LCObjectUserData(object), p_(p) {
            }


            inline MCParticle* getMCParticle() {
                return dynamic_cast<MCParticle*>(object_);
            }

            double p() {
                return p_;
            }

        private:

            double p_{0.};

    };

}

#endif
