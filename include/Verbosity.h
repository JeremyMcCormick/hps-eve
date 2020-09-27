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

    class Verbosity {

        public:

            typedef std::map<std::string, Verbosity*> LoggerMap;
            typedef LoggerMap::iterator LoggerMapIter;

        public:

            Verbosity(std::string name,
                      int verbose = INFO,
                      std::ostream& logOut = std::cout,
                      std::ostream& logErr = std::cerr);

            virtual ~Verbosity();

            int getVerbosity();

            virtual void setVerbosity(int verbosity);

            inline void log(std::string what, int level = INFO) {
                log(level) << what << std::endl;
            }

            inline std::ostream& log(int level = INFO) {
                std::ostream* logMsg = &logOut_;
                if (level < INFO) {
                    logMsg = &logErr_;
                }
                if (checkVerbosity(level)) {
                    logMsg->clear();
                    (*logMsg) << "[ " << name_ << " ] [ " << levelName(level) << "] ";
                } else {
                    logMsg->setstate(std::ios::failbit);

                }
                return *logMsg;
            }

            void flush() {
                logOut_.flush();
                logErr_.flush();
            }

            static Verbosity* getLogger(std::string& name) {
                return LOGGERS_[name];
            }

            static void flushLoggers() {
                for (LoggerMapIter it = LOGGERS_.begin(); it != LOGGERS_.end(); it++) {
                    it->second->flush();
                }
            }

        private:

            inline bool checkVerbosity(int level) {
                return verbosity_ >= level;
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

            int verbosity_{0};

            ClassDef(Verbosity, 1);
    };
}

#endif
