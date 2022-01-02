#include "logger.h"

void tiny::Logger::setLevel(tiny::LogLv lv) {
    level = lv;
}

tiny::LogLv tiny::Logger::getLevel() const {
    return level;
}

void tiny::Logger::log(LogLv lv, const std::string &msg) {
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
    log(LogLv::Debug, msg);
}

void tiny::Logger::info(const std::string &msg) {
    log(LogLv::Info, msg);
}

void tiny::Logger::warning(const std::string &msg) {
    log(LogLv::Warning, msg);
}

void tiny::Logger::error(const std::string &msg) {
    log(LogLv::Error, msg);
}

void tiny::Logger::fatal(const std::string &msg) {
    log(LogLv::Fatal, msg);
}

std::string tiny::LogMsg::levelToString(LogLv lv) {
    switch (lv) {
        case LogLv::Debug:
            return "DEBUG";
        case LogLv::Info:
            return "INFO";
        case LogLv::Warning:
            return "WARNING";
        case LogLv::Error:
            return "ERROR";
        case LogLv::Fatal:
            return "FATAL";
        default:
            return "?";
    }
}

std::int32_t tiny::LogMsg::levelColour(tiny::LogLv lv) {
    switch (lv) {
#if defined(_WIN32)
        case LogLv::Debug:
            return 15;
        case LogLv::Info:
            return 9;
        case LogLv::Warning:
            return 14;
        case LogLv::Error:
        case LogLv::Fatal:
            return 12;
#else
            case LogLv::Debug:
                return 37;
            case LogLv::Info:
                return 34;
            case LogLv::Warning:
                return 33;
            case LogLv::Error:
            case LogLv::Fatal:
                return 31;
#endif
        default:
            return 0;
    }
}
