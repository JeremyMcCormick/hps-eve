#ifndef HPS_VERBOSITY_H_
#define HPS_VERBOSITY_H_ 1

#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include "TObject.h"

namespace hps {

    static const int OFF = 0;
    static const int ERROR = 1;
    static const int WARNING = 2;
    static const int INFO = 3;
    static const int FINE = 4;
    static const int FINER = 5;
    static const int FINEST = 6;

    class LogHandler {

        public:

            typedef std::map<std::string, LogHandler*> Map;
            typedef Map::iterator MapIter;

        public:

            LogHandler(std::string name,
                       std::ostream* logOut = &std::cout,
                       std::ostream* logErr = &std::cerr) :
                       name_(name),
                       logOut_(logOut),
                       logErr_(logErr) {
            }

            std::ostream* getOutputStream() {
                return logOut_;
            }

            std::ostream* getErrorStream() {
                return logErr_;
            }

            static LogHandler* getDefault() {
                if (HANDLERS_.find(DEFAULT) == HANDLERS_.end()) {
                    HANDLERS_[DEFAULT] = new LogHandler(DEFAULT);
                }
                return HANDLERS_[DEFAULT];
            }

            static void flushAll() {
                for (MapIter it = HANDLERS_.begin();
                        it != HANDLERS_.end();
                        it++) {
                    it->second->getOutputStream()->flush();
                    it->second->getErrorStream()->flush();
                }
            }

        private:

            std::string name_;
            std::ostream* logOut_;
            std::ostream* logErr_;

            static std::string DEFAULT;

            static Map HANDLERS_;
    };

    class Logger {

        public:

            typedef std::map<std::string, Logger*> LoggerMap;
            typedef LoggerMap::iterator LoggerMapIter;

        public:

            Logger(std::string name,
                   int level = INFO,
                   LogHandler* handler = nullptr);

            virtual ~Logger();

            int getLogLevel();

            virtual void setLogLevel(int verbosity);

            inline void log(std::string msg, int level = INFO) {
                log(level) << msg << std::endl;
            }

            inline std::ostream& log(int level = INFO) {
                std::ostream* logMsg = handler_->getOutputStream();
                if (level < INFO) {
                    logMsg = handler_->getErrorStream();
                }
                logMsg->flush();
                logMsg->clear();
                if (checkLevel(level)) {
                    (*logMsg) << name_ << ":" << levelName(level) << " ";
                } else {
                    logMsg->setstate(std::ios::failbit);
                }
                return *logMsg;
            }

            static Logger* getLogger(std::string& name) {
                return LOGGERS_[name];
            }

        private:

            inline bool checkLevel(int level) {
                return level_ >= level;
            }

            inline static const std::string& levelName(int level) {
                static std::string off = "OFF";
                static std::string error = "ERROR";
                static std::string warning = "WARNING";
                static std::string info = "INFO";
                static std::string fine = "FINE";
                static std::string finer = "FINER";
                static std::string finest = "FINEST";
                switch (level) {
                    case OFF:
                        return off;
                    case ERROR:
                        return error;
                    case WARNING:
                        return warning;
                    case INFO:
                        return info;
                    case FINE:
                        return fine;
                    case FINER:
                        return finer;
                    case FINEST:
                        return finest;
                    default:
                        throw std::runtime_error("Bad log level: " + std::to_string(level));
                }
            }

        private:

            std::string name_;

            LogHandler* handler_;

            static LoggerMap LOGGERS_;

            int level_{0};

            ClassDef(Logger, 1);
    };
}

#endif
