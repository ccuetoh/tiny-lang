#ifndef TINY_UNICODE_H
#define TINY_UNICODE_H

#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <cstdint>

#include "utf8.h"

namespace tiny {
    //! Aliases a uint32_t type
    using UnicodeCodepoint = std::uint32_t;

    //! Aliases a UnicodeCodepoints vector
    using UnicodeCodepoints = std::vector<tiny::UnicodeCodepoint>;

    //! Helper class for unicode transformations
    class UnicodeParser {
    public:
        /*!
         * \brief Transforms a char stream into UnicodeCodepoints
         * \param stream An UTF-8 encoded char stream
         * \return A UTF-8 encoded UnicodeCodepoints vector
         */
        static UnicodeCodepoints fromStream(std::istream &stream);

        /*!
         * \brief Transforms a single UnicodeCodepoint (uint32_t) into a std::string
         * \param codepoint An UTF-8 UnicodeCodepoint
         * \return A std::string
         */
        static std::string toString(std::uint32_t codepoint);

        /*!
         * \brief Transforms a UnicodeCodepoints vector into a std::string
         * \param codepoints A UnicodeCodepoints vector
         * \return A std::string
         */
        static std::string toString(const UnicodeCodepoints &codepoints);

        /*!
         * \brief Transforms a std::string into a UnicodeCodepoints vector
         * \param str A UTF-8 encoded std::string
         * \return A UnicodeCodepoints vector with the content of the string
         */
        static UnicodeCodepoints fromString(std::string_view str);
    };
}

#endif //TINY_UNICODE_H
