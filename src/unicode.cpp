#include "unicode.h"

tiny::UnicodeCodepoints tiny::UnicodeParser::fromStream(std::istream &stream) {
    std::istreambuf_iterator<char> it(stream.rdbuf());
    std::istreambuf_iterator<char> eos;

    std::vector<std::uint32_t> codepoints;

    std::uint32_t val;
    try {
        for (val = utf8::next(it, eos); val; val = utf8::next(it, eos)) {
            codepoints.push_back(val);
        }
    }
    catch (utf8::not_enough_room &e) {}

    return codepoints;
}

std::string tiny::UnicodeParser::toString(std::uint32_t codepoint) {
    std::string out;
    if (codepoint <= 0x7f)
        out.append(1, static_cast<char>(codepoint));
    else if (codepoint <= 0x7ff) {
        out.append(1, static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
        out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
    } else if (codepoint <= 0xffff) {
        out.append(1, static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
        out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
        out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
    } else {
        out.append(1, static_cast<char>(0xf0 | ((codepoint >> 18) & 0x07)));
        out.append(1, static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
        out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
        out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
    }

    return out;
}

std::string tiny::UnicodeParser::toString(const UnicodeCodepoints &codepoints) {
    std::string out;
    for (auto const &codepoint: codepoints) {
        out += tiny::UnicodeParser::toString(codepoint);
    }

    return out;
}

tiny::UnicodeCodepoints tiny::UnicodeParser::fromString(std::string_view str) {
    std::vector<std::uint32_t> codepoints;
    auto it = str.begin();

    std::uint32_t val;
    try {
        for (val = utf8::next(it, str.end()); val; val = utf8::next(it, str.end())) {
            codepoints.push_back(val);
        }
    }
    catch (utf8::not_enough_room &e) {}

    return codepoints;
}
