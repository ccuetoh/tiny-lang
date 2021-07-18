#ifndef TINY_LOGGER_H
#define TINY_LOGGER_H

#include <string>
#include <iostream>
#include <mutex>

#if defined(_WIN32)

#include <windows.h>

#endif

namespace tiny {
    // Forward declare helper struct
    struct LogMsg;

    /*!
     * The Logger class handles logging and printing of messages through
     * the compilation process. It can be configured to any of several logging
     * levels. By default the logger prints its output to std-io for non-error
     * messages, and to std-err for error messages.
     */
    class Logger {
    public:
        /*!
         * Fetches the Logger's singleton instance. Logger implements Meyers' singleton pattern.
         */
        static Logger &getInstance() {
            static Logger instance;
            return instance;
        }

        // Level

        /*!
         * Level represents the available logging levels. When a level is selected the messages
         * with a higher logging-level will not be logged.
         */
        enum Level {
            Debug,
            Info,
            Warning,
            Error,
            Fatal,
        };

        //! Sets the current logging level.
        void setLevel(Logger::Level lv);

        //! Gets the current logging level.
        [[nodiscard]] int getLevel() const;

        // Logging

        //! Logs the message.
        void log(const LogMsg &msg);

        //! Logs a message with the given logging-level.
        void log(Logger::Level lv, const std::string &msg);

        //! Shorthand for log with Debug level.
        void debug(const std::string &msg);

        //! Shorthand for log with Info level.
        void info(const std::string &msg);

        //! Shorthand for log with Warning level.
        void warning(const std::string &msg);

        //! Shorthand for log with Error level.
        void error(const std::string &msg);

        //! Shorthand for log with Fatal level.
        void fatal(const std::string &msg);

    private:
        Logger() = default;;

        Logger(Logger const &);              // Meyers' singleton pattern. Don't Implement
        void operator=(Logger const &);      // Meyers' singleton pattern. Don't Implement

        //! Lock over the logger
        std::mutex mutex;

        //! Current logging-level. Defaults to "Info".
        int level = Logger::Level::Info;
    };

    //! Defines a message for the Logger.
    struct LogMsg {
        //! Constructor using a logging-level and a string message.
        LogMsg(Logger::Level level, std::string msg) : level(level), content(std::move(msg)) {}

        //! Message to get logged.
        std::string content;

        //! Level of the error.
        Logger::Level level;

        //! Fetches the level as a string.
        static std::string levelToString(Logger::Level lv);

        //! Fetches the output colour for the level.
        static int levelColour(Logger::Level level);
    };
}


#endif //TINY_LOGGER_H
