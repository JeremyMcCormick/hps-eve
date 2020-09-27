#include "Logger.h"

ClassImp(hps::Logger);

namespace hps {

    Logger::LoggerMap Logger::LOGGERS_ = Logger::LoggerMap();

    Logger::Logger(std::string name,
                   int level,
                   std::ostream& logOut,
                   std::ostream& logErr) :
            name_(name),
            level_(level),
            logOut_(logOut),
            logErr_(logErr) {
        LOGGERS_[name] = this;
    }

    Logger::~Logger() {
    }

    int Logger::getLevel() {
        return level_;
    }

    void Logger::setLevel(int verbosity) {
        level_ = verbosity;
    }
}
