#ifndef TINY_ERRORS_H
#define TINY_ERRORS_H

#include <utility>

#include "lexer.h"
#include "metadata.h"
#include "logger.h"

namespace tiny {
    //! Base class for errors thrown by the Tiny compiler. The inherited errors should be preferred when throwing.
    struct CompilerError : public std::exception {
        std::string msg;
        tiny::Metadata meta;

        explicit CompilerError(std::string msg, tiny::Metadata md) : msg(std::move(msg)), meta(std::move(md)) {};

        virtual const char *what() {
            return msg.c_str();
        }
    };

    //! Gets thrown when a given program string can't be tokenized.
    struct LexError : tiny::CompilerError {
        using CompilerError::CompilerError; // Inherit the constructor
    };

    //! The ErrorBuilder applies a stream over the exception's metadata to generate a formatted exception message for
    //! logging
    class ErrorBuilder {
        //! The exception thrown
        tiny::CompilerError &exception;

        //! The stream representation of the file that threw the error
        tiny::WalkableStream<uint32_t> &stream;

    public:
        //! Constructor over the exception and stream
        explicit ErrorBuilder(tiny::CompilerError &e, tiny::WalkableStream<uint32_t> &s) : exception(e), stream(s) {};

        //! Logs the formatted exception to the provider logger with level Error
        void log(tiny::Logger *l) const;
    };
}

#endif //TINY_ERRORS_H
