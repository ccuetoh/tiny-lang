//
// Created by Camilo Hernández on 16-07-2021.
//

#ifndef TINY_STRINGUTIL_H
#define TINY_STRINGUTIL_H

namespace tiny::util {
    //! Trim a string from left
    inline std::string &ltrim(std::string &s, const char *t = " \t\n\r\f\v") {
        s.erase(0, s.find_first_not_of(t));
        return s;
    }

    //! Trim a string from the right
    inline std::string &rtrim(std::string &s, const char *t = " \t\n\r\f\v") {
        s.erase(s.find_last_not_of(t) + 1);
        return s;
    }

    //! Trim a string from the left and right
    inline std::string &trim(std::string &s, const char *t = " \t\n\r\f\v") {
        return ltrim(rtrim(s, t), t);
    }
}

#endif //TINY_STRINGUTIL_H
