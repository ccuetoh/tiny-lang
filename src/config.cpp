#include "config.h"
#include "stream.h"
#include "comparator.h"
#include "logger.h"
#include "errors.h"

tiny::Setting tiny::Configuration::getSetting(tiny::Option opt)
{
    for (auto const &s: settings) {
        if (s.option == opt) {
            return s;
        }
    }

    return {}; // Should be unreachable
}

void tiny::Configuration::setSetting(const tiny::Setting &stng)
{
    for (auto &setting : settings) {
        if (setting.option == stng.option) {
            setting = stng;
            return;
        }
    }
}


void tiny::Configuration::parseArguments(int argc, char *argv[])
{
    if (argc < 2) {
        return; // No arguments
    }

    std::vector<tiny::String> vec;
    vec.reserve(argc-1);

    for (std::int32_t i = 1; i < argc; i++) {
        vec.emplace_back(argv[i]);
    }

    tiny::Stream<tiny::String> s(vec);
    tiny::StreamComparator comparator(s);
    while (s) {
        switch(comparator.match(CMD_TABLE)) {
        case Option::Invalid:
            throw tiny::CLIError("Invalid setting '" + s.peek().toString() + "'");
        case Option::PrintVersion: {
            setSetting(tiny::Setting{Option::PrintVersion, true});
            return; // Since this is an operating mode whe can return immediately because no other configs matter
        }
        case Option::Log: {
            auto levelStr = s.get();
            static std::map<tiny::String, std::int32_t> levelTable {
                    {"debug", std::int32_t(tiny::LogLevel::Debug)},
                    {"info", std::int32_t(tiny::LogLevel::Info)},
                    {"warn", std::int32_t(tiny::LogLevel::Warning)},
                    {"warning", std::int32_t(tiny::LogLevel::Warning)},
                    {"warnings", std::int32_t(tiny::LogLevel::Warning)},
                    {"error", std::int32_t(tiny::LogLevel::Error)},
                    {"errors", std::int32_t(tiny::LogLevel::Error)},
                    {"fatal", std::int32_t(tiny::LogLevel::Fatal)},
                    {"disable", std::int32_t(tiny::LogLevel::Disable)},
                    {"disabled", std::int32_t(tiny::LogLevel::Disable)},
            };

            if (levelTable.count(levelStr) < 1) {
                throw tiny::CLIError("Invalid argument ('" + levelStr.toString() + "') for the '--log' setting");
            }

            setSetting(tiny::Setting{Option::Log, true, levelTable[levelStr]});
            break;
        }

        case Option::OutputASTJSON:
            setSetting(tiny::Setting{Option::OutputASTJSON, true});
            break;
        }
    }
}
