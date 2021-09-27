#ifndef TINY_LOGGER_H
#define TINY_LOGGER_H

#include <string>
#include <iostream>
#include <mutex>

#if defined(_WIN32)

#include <windows.h>

#endif

namespace tiny {
    // Forward declaration
    struct LogMsg;

    /*!
     * \brief The Logger class handles formatted and pretty logging
     *
     * The Logger class handles formatted and pretty logging. It can be configured to any of several logging
     * levels. By default the logger prints its output to std-io for non-error
     * messages, and to std-err for error messages.
     */
    class Logger {
    public:
        /*!
         * \brief Fetches the Logger's singleton instance
         * \return The Logger's singleton instance
         *
         * Fetches the Logger's singleton instance. Implements Meyers' singleton pattern.
         */
        static Logger &getInstance() {
            static Logger instance;
            return instance;
        }

        //! Logging levels. When a level is selected, messages with a higher level will not be logged.
        enum class Level {
            Debug,
            Info,
            Warning,
            Error,
            Fatal,
        };

        /*!
         * \brief Sets the current logging level
         * \param lv Logging level
         */
        void setLevel(Logger::Level lv);

        /*!
        * \brief Gets the current logging level
        * \return Current logging level
        */
        [[nodiscard]] tiny::Logger::Level getLevel() const;

        /*!
        * \brief Logs a message using its logging level
        * \param msg Message to log
        */
        void log(const LogMsg &msg);

        /*!
        * \brief Logs a message overriding its logging level to the given Level
        * \param msg Message to log
         *\param lv Level to log at
        */
        void log(const std::string &msg, Logger::Level lv);

        /*!
        * \brief Logs a message at the Debug level
        * \param msg Message to log
        */
        void debug(const std::string &msg);

        /*!
        * \brief Logs a message at the Info level
        * \param msg Message to log
        */
        void info(const std::string &msg);

        /*!
        * \brief Logs a message at the Warning level
        * \param msg Message to log
        */
        void warning(const std::string &msg);

        /*!
        * \brief Logs a message at the Error level
        * \param msg Message to log
        */
        void error(const std::string &msg);

        /*!
        * \brief Logs a message at the Fatal level
        * \param msg Message to log
        */
        void fatal(const std::string &msg);

    private:
        Logger() = default;

        Logger(Logger const &);              // Meyers' singleton pattern. Don't Implement
        void operator=(Logger const &);      // Ibidem

        //! Lock over the logger
        std::mutex mutex;

        //! Current logging-level. Defaults to Info.
        tiny::Logger::Level level = Logger::Level::Info;
    };

    //! Defines a message for the Logger.
    struct LogMsg {
        /*!
        * \brief Constructs a loggable message from a string contet and a logging level
        * \param level The logging level
        * \param msg Content of the message
        */
        LogMsg(Logger::Level level, std::string msg) : level(level), content(std::move(msg)) {}

        //! Content of the log
        std::string content;

        //! Level of the error
        Logger::Level level = Logger::Level::Info;

        /*!
        * \brief Returns a string with the logging level's name (Debug will return "DEBUG)
        * \param lv The logging level
        * \return An uppercase string with the logging level's name
        */
        static std::string levelToString(Logger::Level lv);

        /*!
        * \brief Returns the a console color based on the level provided
        * \param lv The logging level
        * \return An int with the color the console should print the message
        *
        * Returns the a console color based on the level provided. The color varies depending on the host OS. In
        * Windows systems the Windows' API coloring code is provided, otherwise the ANSI escape code color is returned.
        */
        [[nodiscard]] static std::int32_t levelColour(Logger::Level lv);
    };
}


#endif //TINY_LOGGER_H
