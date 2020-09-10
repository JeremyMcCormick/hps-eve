#include "Verbosity.h"

namespace hps {

    Verbosity::Verbosity(int verbosity) : verbosity_(verbosity) {
    }

    Verbosity::~Verbosity() {
    }

    int Verbosity::getVerbosity() {
        return verbosity_;
    }

    void Verbosity::setVerbosity(int verbosity) {
        verbosity_ = verbosity;
    }

    bool Verbosity::checkVerbosity(int level) {
            return verbosity_ >= level;
    }

    bool Verbosity::checkVerbosity() {
        return verbosity_ > 0;
    }

}
