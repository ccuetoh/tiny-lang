#include "logger.h"

void tiny::Logger::setLevel(tiny::LogLevel lv) {
    level = lv;
}

tiny::LogLevel tiny::Logger::getLevel() const {
    return level;
}

void tiny::Logger::log(LogLevel lv, const std::string &msg) {
    LogMsg logMsg(lv, msg);
    log(logMsg);
}

void tiny::Logger::log(const tiny::LogMsg &msg) {
    if (msg.level > getLevel()) {
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

    stream << "[";

    SetConsoleTextAttribute(console, tiny::LogMsg::levelColour(msg.level));
    stream << lv;

    SetConsoleTextAttribute(console, consoleAttr); // Reset the console
    stream << "] " + msg.content + "\n";
#else
    stream << "[" << "\u001b[" << tiny::LogMsg::levelColour(msg.level) << ";1m" << lv;
    stream << "\u001b[0m] " + msg.content + "\n";
#endif
}

void tiny::Logger::debug(const std::string &msg) {
    log(LogLevel::Debug, msg);
}

void tiny::Logger::info(const std::string &msg) {
    log(LogLevel::Info, msg);
}

void tiny::Logger::warning(const std::string &msg) {
    log(LogLevel::Warning, msg);
}

void tiny::Logger::error(const std::string &msg) {
    log(LogLevel::Error, msg);
}

void tiny::Logger::fatal(const std::string &msg) {
    log(LogLevel::Fatal, msg);
}

std::string tiny::LogMsg::levelToString(LogLevel lv) {
    switch (lv) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warning:
            return "WARNING";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Fatal:
            return "FATAL";
        default:
            return "?";
    }
}

std::int32_t tiny::LogMsg::levelColour(tiny::LogLevel lv) {
    switch (lv) {
#if defined(_WIN32)
        case LogLevel::Debug:
            return 15;
        case LogLevel::Info:
            return 9;
        case LogLevel::Warning:
            return 14;
        case LogLevel::Error:
        case LogLevel::Fatal:
            return 12;
#else
        case LogLevel::Debug:
            return 37;
        case LogLevel::Info:
            return 34;
        case LogLevel::Warning:
            return 33;
        case LogLevel::Error:
        case LogLevel::Fatal:
            return 31;
#endif
        default:
            return 0;
    }
}
