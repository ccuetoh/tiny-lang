#include <fstream>
#include <string>

#include "logger.h"
#include "compiler.h"
#include "config.h"
#include "errors.h"

/*
 * Important: This is the WIP main, and it's here just for testing.
 */

int main(int argc, char *argv[]) {
    try {
        tiny::Configuration::get().parseArguments(argc, argv);
    } catch(const tiny::CLIError &e) {
        tiny::fatal(e.msg);
        return 1;
    }

    // Set the logging level
    auto lv = tiny::LogLevel(std::get<std::int32_t>(tiny::getSetting(tiny::Option::Log).param));
    tiny::Logger::get().setLevel(lv);

    try {
        std::locale::global(std::locale("en_US.UTF8"));
    } catch (std::runtime_error const &) {
        tiny::warn("Unable to use 'en_US.UTF8' locale. Using the system's default ('" + std::locale().name() + "').");
        tiny::warn("Non-ASCII characters might be unrecognized.");
    }

    if (tiny::getSetting(tiny::Option::PrintVersion).isEnabled) {
        std::cout << TINY_NAME << " " << TINY_VERSION << " (" << TINY_VERSION_NICKNAME << "). "
                  << TINY_COPYRIGHT << " " << TINY_LICENCE << std::endl;
        return 0;
    }

    tiny::Compiler compiler;
    compiler.compile();
}