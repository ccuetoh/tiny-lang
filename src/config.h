#ifndef TINY_CONFIG_H
#define TINY_CONFIG_H

#include <utility>
#include <variant>
#include <map>

#include "unicode.h"
#include "logger.h"

namespace tiny{
    //! The different configurable settings for the compiler
    enum class Option {
        Invalid,
        PrintVersion,
        Log,
        OutputASTJSON,
    };

    //! Holds the current state of a setting
    struct Setting {
        //! The option this state corresponds to
        tiny::Option option = Option::Invalid;

        //! Whether the setting is enabled or disabled. Behavior depends on which option this setting corresponds to
        bool isEnabled = false;

        //! A variant containing an optional parameter for the setting
        std::variant<tiny::String, std::int32_t> param = "";
    };

    //! A singleton that contains the current settings for the runtime
    class Configuration {
    public:
        Configuration(Configuration const &) = delete;       // Meyers' singleton pattern. Don't Implement
        void operator=(Configuration const &) = delete;      // Ibidem

        //! Get the singleton instance
        static Configuration &get() {
            static Configuration instance;
            return instance;
        }

        //! Gets a setting by its option
        tiny::Setting getSetting(tiny::Option opt);
        //! Sets a setting by replacing the state of the setting with the matching
        void setSetting(const tiny::Setting &stng);

        //! Parses the command-line arguments
        void parseArguments(int argc, char *argv[]);

    private:
        //! Default constructor
        Configuration() = default;

        //! Current settings. Starts as the default settings
        std::vector<tiny::Setting> settings {
                {Option::PrintVersion, false},
                {Option::Log, true, std::int32_t(tiny::LogLevel::Info)},
                {Option::OutputASTJSON, false},
        };

        //! Maps parameters to their respective option for use in argument parsing
        const std::map<std::vector<tiny::String>, tiny::Option> CMD_TABLE {
                {{"version"}, Option::PrintVersion},
                {{"--log"}, Option::Log},
                {{"--ast-json"}, Option::OutputASTJSON},
        };
    };

#if !defined(TINY_DISABLE_COMPACT_CONFIG)
    inline auto getSetting(tiny::Option opt) { return tiny::Configuration::get().getSetting(opt); }
#endif
}


#endif //TINY_CONFIG_H
