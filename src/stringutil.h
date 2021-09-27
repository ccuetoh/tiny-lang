//
// Created by Camilo Hernández on 16-07-2021.
//

#ifndef TINY_STRINGUTIL_H
#define TINY_STRINGUTIL_H

namespace tiny {
    /*!
     * \brief Trims a string from the left
     * \param s The string
     * \param t The value to trim, defaults to blanks (" \t\n\r\f\v")
     * \return The string with the trimmed characters
     *
     * Trims a string from the left by removing all matching characters repeatedly from the left side of the string.
     */
    inline std::string &ltrim(std::string &s, const char *t = " \t\n\r\f\v") {
        s.erase(0, s.find_first_not_of(t));
        return s;
    }

    /*!
     * \brief Trims a string from the right
     * \param s The string
     * \param t The value to trim, defaults to blanks (" \t\n\r\f\v")
     * \return The string with the trimmed characters
     *
     * Trims a string from the right by removing all matching characters repeatedly from the right side of the string.
     */
    inline std::string &rtrim(std::string &s, const char *t = " \t\n\r\f\v") {
        s.erase(s.find_last_not_of(t) + 1);
        return s;
    }

    /*!
     * \brief Trims a string from the left and right
     * \param s The string
     * \param t The value to trim, defaults to blanks (" \t\n\r\f\v")
     * \return The string with the trimmed characters
     *
     * Trims a string from both sides by removing all matching characters repeatedly from either side of the string.
     */
    inline std::string &trim(std::string &s, const char *t = " \t\n\r\f\v") {
        return ltrim(rtrim(s, t), t);
    }
}

#endif //TINY_STRINGUTIL_H
