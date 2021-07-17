#include "logger.h"

void tiny::Logger::setLevel(tiny::Logger::Level lv) {
    level = lv;
}

int tiny::Logger::getLevel() const {
    return level;
}

void tiny::Logger::log(tiny::Logger::Level lv, const std::string &msg) {
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
    log(Level::Debug, msg);
}

void tiny::Logger::info(const std::string &msg) {
    log(Level::Info, msg);
}

void tiny::Logger::warning(const std::string &msg) {
    log(Level::Warning, msg);
}

void tiny::Logger::error(const std::string &msg) {
    log(Level::Error, msg);
}

void tiny::Logger::fatal(const std::string &msg) {
    log(Level::Fatal, msg);
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

int tiny::LogMsg::levelColour(tiny::Logger::Level lv) {
#if defined(_WIN32)
    switch (lv) {
        case Logger::Debug:
            return 15;
        case Logger::Info:
            return 9;
        case Logger::Warning:
            return 14;
        case Logger::Error:
        case Logger::Fatal:
            return 12;
    }
#else
    switch (lv) {
        case Logger::Debug:
            return 37;
        case Logger::Info:
            return 34;
        case Logger::Warning:
            return 33;
        case Logger::Error:
        case Logger::Fatal:
            return 31;
    }
#endif

    return 0;
}
