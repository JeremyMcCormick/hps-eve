#include "Logger.h"

ClassImp(hps::Logger);

namespace hps {

    Logger::LoggerMap Logger::LOGGERS_ = Logger::LoggerMap();
    LogHandler::LogHandlerMap LogHandler::HANDLERS_ = LogHandler::LogHandlerMap();

    std::string LogHandler::DEFAULT = std::string("DEFAULT");

    Logger::Logger(std::string name,
                   int level,
                   LogHandler* handler) :
            name_(name),
            level_(level) {
        LOGGERS_[name] = this;
        if (handler == nullptr) {
            handler_ = LogHandler::getDefault();
        }
    }

    Logger::~Logger() {
    }

    int Logger::getLogLevel() {
        return level_;
    }

    void Logger::setLogLevel(int level) {
        level_ = level;
    }
}
