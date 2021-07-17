#ifndef TINY_UNICODE_H
#define TINY_UNICODE_H

#include <string>
#include <iostream>
#include <vector>
#include <sstream>

#include "utf8.h"

namespace tiny {
    //! Aliases a uint32_t vector
    using UnicodeCodepoints = std::vector<uint32_t>;

    //! Helper class for unicode transformations
    class UnicodeParser {
    public:
        //! Transforms a char stream into UnicodeCodepoints
        static UnicodeCodepoints FromStream(std::istream &stream);

        //! Transforms uint32_t into a std::string
        static std::string ToString(uint32_t codepoint);

        //! Transforms UnicodeCodepoints into a std::string
        static std::string ToString(const UnicodeCodepoints &);

        //! Transforms UnicodeCodepoints into a std::string
        static UnicodeCodepoints FromString(std::string_view str);
    };
}

#endif //TINY_UNICODE_H
