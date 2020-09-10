#ifndef HPS_VERBOSITY_H_
#define HPS_VERBOSITY_H_ 1

namespace hps {

    class Verbosity {

        public:

            Verbosity(int verbose = 0);

            virtual ~Verbosity();

            int getVerbosity();

            virtual void setVerbosity(int verbosity);

            bool checkVerbosity(int level);

            bool checkVerbosity();

        private:

            int verbosity_{0};
    };
}

#endif
