#ifndef TINY_UNICODE_H
#define TINY_UNICODE_H

#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <cstdint>

#include "utf8.h"

namespace tiny {
    class String {
    public:
        std::vector<std::uint32_t> codepoints = {};

        explicit String() = default;

        /*!
         * \brief Transforms a char stream into a unicode string
         * \param stream An UTF-8 encoded char stream
         */
        explicit String(const std::istream &stream);

        /*!
         * \brief Transforms a std::string into a unicode string
         * \param str A UTF-8 encoded std::string
         */
        explicit String(std::string_view str);

        /*!
         * \brief Transforms a std::uint32_t representing a wide char into a unicode string
         * \param str A UTF-8 encoded std::string
         */
        explicit String(std::uint32_t c);

        /*!
         * \brief Transforms a C-string into a unicode string
         * \param str A UTF-8 encoded C-string
         */
        String(const char str[]):String(std::string_view(str)) {};

        /*!
         * \brief Transforms a UnicodeString vector into a std::string
         * \return A std::string
         */
        [[nodiscard]] std::string toString() const;

        /*!
         * \brief Fetches a copy of the internal data
         * \return A std::vector<std::uint32_t> containing the UTF-8 codepoints
         */
        [[nodiscard]] std::vector<std::uint32_t> data() const;

        /*!
         * \brief Less-than operator for UnicodeStrings. Uses the std::string implementation
         */
        [[nodiscard]] bool operator < (const tiny::String &str2) const {
             return toString() < str2.toString();
         }

        /*!
        * \brief Adds a char to the string
        */
        void operator += (const std::uint32_t &c) {
            codepoints.push_back(c);
        }

        /*!
        * \brief Adds a char to the string
        */
        tiny::String operator + (const char cStr[]) const {
            std::vector<std::uint32_t> codepointsCopy(codepoints);
            for(char const* it = cStr; *it; ++it) {
                codepointsCopy.push_back(*it);
            }

            tiny::String newString;
            newString.codepoints = codepointsCopy;

            return newString;
        }

        /*!
        * \brief Appends a string
        */
        void operator += (const tiny::String &str) {
            codepoints.insert(codepoints.end(), str.codepoints.begin(), str.codepoints.end());
        }

        /*!
        * \brief Appends a string
        */
        tiny::String operator + (const tiny::String &str) const {
            std::vector<std::uint32_t> codepointsCopy(codepoints);
            codepointsCopy.insert(codepointsCopy.end(), str.codepoints.begin(), str.codepoints.end());

            tiny::String newString;
            newString.codepoints = codepointsCopy;

            return newString;
        }

        /*!
        * \brief Compares two strings
        */
        [[nodiscard]] bool operator == (const tiny::String &str) const {
            return codepoints == str.codepoints;
        }

        /*!
        * \brief Asserts the difference between two strings
        */
        [[nodiscard]] bool operator != (const tiny::String &str) const {
            return codepoints != str.codepoints;
        }

        /*!
        * \brief Checks whether a string contains anything
        */
        [[nodiscard]] explicit operator bool() const {
            return codepoints.empty();
        }

        /*!
        * \brief Returns the i-th codepoint in the string
        */
        [[nodiscard]] std::uint32_t  operator [](const int &i) const {
            return codepoints[i];
        }
    };
}

#endif //TINY_UNICODE_H
