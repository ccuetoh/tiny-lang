#include "gtest/gtest.h"

#include "config.h"

TEST(Configuration, GetSetting) {
    auto config = tiny::getSetting(tiny::Option::PrintVersion);

    // Default settings
    ASSERT_EQ(config.option, tiny::Option::PrintVersion);
    ASSERT_EQ(config.isEnabled, false);
    ASSERT_EQ(std::get<tiny::String>(config.param), tiny::String(""));
}

TEST(Configuration, SetSetting) {
    tiny::Setting stng{tiny::Option::Log, true, std::int32_t(tiny::LogLevel::Debug)};
    tiny::Configuration::get().setSetting(stng);

    auto config = tiny::getSetting(tiny::Option::Log);

    ASSERT_EQ(config.option, tiny::Option::Log);
    ASSERT_EQ(config.isEnabled, true);
    ASSERT_EQ(std::get<std::int32_t>(config.param), std::int32_t(tiny::LogLevel::Debug));
}
