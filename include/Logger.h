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

    class Logger {

        public:

            typedef std::map<std::string, Logger*> LoggerMap;
            typedef LoggerMap::iterator LoggerMapIter;

        public:

            Logger(std::string name,
                      int level = INFO,
                      std::ostream& logOut = std::cout,
                      std::ostream& logErr = std::cerr);

            virtual ~Logger();

            int getLogLevel();

            virtual void setLogLevel(int verbosity);

            inline void log(std::string what, int level = INFO) {
                log(level) << what << std::endl;
            }

            inline std::ostream& log(int level = INFO) {
                std::ostream* logMsg = &logOut_;
                if (level < INFO) {
                    logMsg = &logErr_;
                }
                logMsg->flush();
                logMsg->clear();
                if (checkLevel(level)) {
                    //std::cout << ">>>> good level: " << level << std::endl;
                    (*logMsg) << "[ " << name_ << " ] [ " << levelName(level) << " ] ";
                } else {
                    //std::cout << ">>>> setting failbit" << std::endl;
                    logMsg->setstate(std::ios::failbit);
                }
                return *logMsg;
            }

            void flush() {
                logOut_.flush();
                logErr_.flush();
            }

            static Logger* getLogger(std::string& name) {
                return LOGGERS_[name];
            }

            static void flushLoggers() {
                for (LoggerMapIter it = LOGGERS_.begin(); it != LOGGERS_.end(); it++) {
                    it->second->flush();
                }
            }

        private:

            inline bool checkLevel(int level) {
                //std::cout << ">>>> checkLevel name, logger, msg: " << name_ << ", " << level_ << ", " << level << std::endl;
                return level_ >= level;
            }

            static const std::string& levelName(int level) {
                static std::string error = "ERROR";
                static std::string warning = "WARNING";
                static std::string info = "INFO";
                static std::string fine = "FINE";
                static std::string finer = "FINER";
                static std::string finest = "FINEST";
                static std::string bad = "";
                switch (level) {
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

            std::ostream& logOut_;
            std::ostream& logErr_;

            static LoggerMap LOGGERS_;

            int level_{0};

            ClassDef(Logger, 1);
    };
}

#endif
