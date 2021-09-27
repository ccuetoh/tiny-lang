#ifndef TINY_ERRORS_H
#define TINY_ERRORS_H

#include <utility>

#include "lexer.h"
#include "metadata.h"
#include "logger.h"

namespace tiny {
    //! Base class for errors thrown by the Tiny compiler. The inherited errors should be preferred when throwing.
    struct CompilerError : public std::exception {
        //! Explanation of the error
        std::string msg;
        //! Information about the error, that might include the file and fragment that triggered it
        tiny::Metadata meta;

        /*!
         * \brief Creates a new CompilerError error exception
         * \param msg Explanation of the error
         * \param md Information about the error
         */
        explicit CompilerError(std::string msg, tiny::Metadata md) : msg(std::move(msg)), meta(std::move(md)) {};

        /*!
         * \brief Returns a C-string detailing the error
         * \return A C-string with an explanation of the error
         */
        virtual const char *what() {
            return msg.c_str();
        }
    };

    //! Gets thrown by the lexer when a program can't be tokenized.
    struct LexError : tiny::CompilerError {
        using CompilerError::CompilerError; // Inherit the constructor
    };

    //! Gets thrown by the parser when a given program's tokens are syntactically invalid.
    struct ParseError : tiny::CompilerError {
        using CompilerError::CompilerError; // Inherit the constructor
    };

    //! The ErrorBuilder applies a stream over the exception's metadata to generate a formatted exception message for logging
    class ErrorBuilder {
        //! The exception thrown
        tiny::CompilerError &exception;

        //! The stream representation of the file that threw the error
        tiny::WalkableStream<std::uint32_t> &stream;

    public:
        //! Constructor over the exception and stream
        explicit ErrorBuilder(tiny::CompilerError &e, tiny::WalkableStream<std::uint32_t> &s) : exception(e), stream(s) {};

        /*!
         * \brief Logs the formatted exception to the provider logger with level Error
         * \param l Logger in which to log the error
         */
        void log(tiny::Logger &l) const;
    };
}

#endif //TINY_ERRORS_H
