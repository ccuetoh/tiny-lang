#include <fstream>
#include <string>

#include "logger.h"
#include "compiler.h"

/*
 * Important: This is the WIP main, and it's here just for testing.
 */

int main(int argc, char **argv) {
    tiny::Logger::get().setLevel(tiny::LogLv::Debug);

    try {
        std::locale::global(std::locale("en_US.UTF8"));
    } catch (std::runtime_error const &) {
        tiny::warn("Unable to use 'en_US.UTF8' locale. Using the system's default ('" + std::locale().name() + "').");
        tiny::warn("Non-ASCII characters might be unrecognized.");
    }

    if (argc == 2 && (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "version") == 0)) {
        std::cout << TINY_NAME << " " << TINY_VERSION << " (" << TINY_VERSION_NICKNAME << "). "
                  << TINY_COPYRIGHT << " " << TINY_LICENCE << std::endl;
        return 0;
    }

    tiny::Compiler compiler;
    compiler.compile();
}