#ifndef TINY_LOGGER_H
#define TINY_LOGGER_H

#include <string>
#include <iostream>
#include <mutex>

#if defined(_WIN32)
#include <Windows.h>
#endif

#include "file.h"

namespace tiny {
    // Forward declaration
    struct LogMsg;

    //! Logging levels. When a level is selected, messages with a higher level will not be logged.
    enum class LogLevel: std::int32_t {
        Debug = 4,
        Info = 3,
        Warning = 2,
        Error = 1,
        Fatal = 0,
        Disable = -1,
    };

    /*!
     * \brief The Logger class handles formatted and pretty logging
     *
     * The Logger class handles formatted and pretty logging. It can be configured to any of several logging
     * levels. By default the logger prints its output to std-io for non-error
     * messages, and to std-err for error messages.
     */
    class Logger {
    public:
        Logger(Logger const &) = delete;              // Meyers' singleton pattern. Don't Implement
        void operator=(Logger const &) = delete;      // Ibidem

        /*!
         * \brief Fetches the Logger's singleton instance
         * \return The Logger's singleton instance
         *
         * Fetches the Logger's singleton instance. Implements Meyers' singleton pattern.
         */
        static Logger &get() {
            static Logger instance;
            return instance;
        }

        /*!
         * \brief Sets the current logging level
         * \param lv Logging level
         */
        void setLevel(tiny::LogLevel lv);

        /*!
        * \brief Gets the current logging level
        * \return Current logging level
        */
        [[nodiscard]] tiny::LogLevel getLevel() const;

        /*!
        * \brief Logs a message using its logging level
        * \param msg Message to log
        */
        void log(const LogMsg &msg);

        /*!
        * \brief Logs a message overriding its logging level to the given Level
        * \param lv Level to log at
        * \param msg Message to log
        */
        void log(tiny::LogLevel lv, const std::string &msg);

        /*!
        * \brief Logs a message at the Debug level
        * \param msg Message to log
        */
        void debug(const std::string &msg);

        /*!
        * \brief Logs a message at the Debug level with it's filename
        * \param msg Message to log
        */
        void debug(const tiny::File &f, const std::string &msg);

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

        //! Stream on which to log
        std::ostream& stream = std::cout;

        //! Lock over the logger
        std::mutex mutex;

        //! Current logging-level. Defaults to Info.
        tiny::LogLevel level = LogLevel::Info;
    };

    //! Defines a message for the Logger.
    struct LogMsg {
        /*!
        * \brief Constructs a loggable message from a string contet and a logging level
        * \param level The logging level
        * \param msg Content of the message
        */
        LogMsg(tiny::LogLevel level, std::string msg) :content(std::move(msg)), level(level) {}

        //! Content of the log
        std::string content;

        //! Level of the error
        tiny::LogLevel level = tiny::LogLevel::Info;

        /*!
        * \brief Returns a string with the logging level's name (Debug will return "DEBUG)
        * \param lv The logging level
        * \return An uppercase string with the logging level's name
        */
        static std::string levelToString(LogLevel lv);

        /*!
        * \brief Returns the a console color based on the level provided
        * \param lv The logging level
        * \return An int with the color the console should print the message
        *
        * Returns the a console color based on the level provided. The color varies depending on the host OS. In
        * Windows systems the Windows' API coloring code is provided, otherwise the ANSI escape code color is returned.
        */
        [[nodiscard]] static std::int32_t levelColour(LogLevel lv);
    };

#if !defined(TINY_DISABLE_COMPACT_LOGGING)
    inline auto debug(const std::string& msg) { tiny::Logger::get().debug(msg); }
    inline auto debug(const tiny::File &f, const std::string& msg) { tiny::Logger::get().debug(f, msg); }
    inline auto info(const std::string& msg) { tiny::Logger::get().info(msg); }
    inline auto warn(const std::string& msg) { tiny::Logger::get().warning(msg); }
    inline auto error(const std::string& msg) { tiny::Logger::get().error(msg); }
    inline auto fatal(const std::string& msg) { tiny::Logger::get().fatal(msg); }
#endif

}


#endif //TINY_LOGGER_H
