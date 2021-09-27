#include "logger.h"

void tiny::Logger::setLevel(tiny::Logger::Level lv) {
    level = lv;
}

tiny::Logger::Level tiny::Logger::getLevel() const {
    return level;
}

void tiny::Logger::log(const std::string &msg, Logger::Level lv) {
    LogMsg logMsg(lv, msg);
    log(logMsg);
}

void tiny::Logger::log(const tiny::LogMsg &msg) {
    if (msg.level < getLevel()) {
        return;
    }

    const std::lock_guard<std::mutex> lock(mutex);

    std::string lv = tiny::LogMsg::levelToString(msg.level);

#if defined(_WIN32)
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

    // Get the current console state so we can restore it
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    GetConsoleScreenBufferInfo(console, &bufferInfo);
    WORD consoleAttr = bufferInfo.wAttributes;

    std::cout << "[";

    SetConsoleTextAttribute(console, tiny::LogMsg::levelColour(msg.level));
    std::cout << lv;

    SetConsoleTextAttribute(console, consoleAttr); // Reset the console
    std::cout << "] " + msg.content + "\n";
#else
    std::cout << "[" << "\u001b[" << tiny::LogMsg::levelColour(msg.level) << ";1m" << lv;
    std::cout << "\u001b[0m] " + msg.content + "\n";
#endif
}

void tiny::Logger::debug(const std::string &msg) {
    log(msg, Level::Debug);
}

void tiny::Logger::info(const std::string &msg) {
    log(msg, Level::Info);
}

void tiny::Logger::warning(const std::string &msg) {
    log(msg, Level::Warning);
}

void tiny::Logger::error(const std::string &msg) {
    log(msg, Level::Error);
}

void tiny::Logger::fatal(const std::string &msg) {
    log(msg, Level::Fatal);
}

std::string tiny::LogMsg::levelToString(Logger::Level lv) {
    switch (lv) {
        case Logger::Level::Debug:
            return "DEBUG";
        case Logger::Level::Info:
            return "INFO";
        case Logger::Level::Warning:
            return "WARNING";
        case Logger::Level::Error:
            return "ERROR";
        case Logger::Level::Fatal:
            return "FATAL";
        default:
            return "?";
    }
}

std::int32_t tiny::LogMsg::levelColour(tiny::Logger::Level lv) {
    switch (lv) {
#if defined(_WIN32)
        case Logger::Level::Debug:
            return 15;
        case Logger::Level::Info:
            return 9;
        case Logger::Level::Warning:
            return 14;
        case Logger::Level::Error:
        case Logger::Level::Fatal:
            return 12;
#else
        case Logger::Level::Debug:
            return 37;
        case Logger::Level::Info:
            return 34;
        case Logger::Level::Warning:
            return 33;
        case Logger::Level::Error:
        case Logger::Level::Fatal:
            return 31;
#endif
        default:
            return 0;
    }
}
