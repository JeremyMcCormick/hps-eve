#include "Verbosity.h"

ClassImp(hps::Verbosity);

namespace hps {

    Verbosity::LoggerMap Verbosity::LOGGERS_ = Verbosity::LoggerMap();

    Verbosity::Verbosity(std::string name,
                         int verbose,
                         std::ostream& logOut,
                         std::ostream& logErr) :
            name_(name),
            verbosity_(verbose),
            logOut_(logOut),
            logErr_(logErr) {
        LOGGERS_[name] = this;
    }

    Verbosity::~Verbosity() {
    }

    int Verbosity::getVerbosity() {
        return verbosity_;
    }

    void Verbosity::setVerbosity(int verbosity) {
        verbosity_ = verbosity;
    }
}
