#include "unicode.h"

tiny::String::String(const std::istream &stream) {
    std::istreambuf_iterator<char> it(stream.rdbuf());
    std::istreambuf_iterator<char> eos;

    try {
        for (std::uint32_t val = utf8::next(it, eos); val; val = utf8::next(it, eos)) {
            codepoints.push_back(val);
        }
    }
    catch (const utf8::not_enough_room &) {
        // Sometimes this exception is thrown on EOF. Seems to be a bug on the library. Ignore for now.
    }
}

tiny::String::String(std::string_view str) {
    auto it = str.begin();

    try {
        for (std::uint32_t val = utf8::next(it, str.end()); val; val = utf8::next(it, str.end())) {
            codepoints.push_back(val);
        }
    }
    catch (const utf8::not_enough_room &) {
        // Sometimes this exception is thrown on EOF. Seems to be a bug on the library. Ignore for now.
    }
}

tiny::String::String(std::uint32_t c)
{
    codepoints.push_back(c);
}

std::string tiny::String::toString() const {
    std::string str;
    for (auto const &codepoint: codepoints) {
        if (codepoint <= 0x7f)
            str.append(1, static_cast<char>(codepoint));
        else if (codepoint <= 0x7ff) {
            str.append(1, static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
            str.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
        } else if (codepoint <= 0xffff) {
            str.append(1, static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
            str.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
            str.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
        } else {
            str.append(1, static_cast<char>(0xf0 | ((codepoint >> 18) & 0x07)));
            str.append(1, static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
            str.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
            str.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
        }
    }

    return str;
}

std::vector<std::uint32_t> tiny::String::data() const
{
    return codepoints;
}
