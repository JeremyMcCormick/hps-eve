#ifndef HPS_VERBOSITY_H_
#define HPS_VERBOSITY_H_ 1

namespace hps {

    class Verbosity {

        public:

            Verbosity(int verbose = 0);

            virtual ~Verbosity();

            int getVerbosity();

            virtual void setVerbosity(int verbosity);

            inline bool checkVerbosity(int level) {
                return verbosity_ >= level;
            }

            inline bool checkVerbosity() {
                return verbosity_;
            }

        private:

            int verbosity_{0};
    };
}

#endif
