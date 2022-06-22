#ifndef TINY_ERRORS_H
#define TINY_ERRORS_H

#include <utility>

#include "metadata.h"

namespace tiny {
    /*!
     * \brief Base class for errors thrown by the main Tiny compiler processes.
     * Base class for errors thrown by the main Tiny compiler processes. The inherited errors should be preferred when
     * throwing.
     */
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
        [[nodiscard]] const char *what() const noexcept override {
            return msg.c_str();
        }

        /*!
         * \brief Uses the error metadata to build a context around a stream and logs it to the Logger
         * \param s The stream over which the error was found
         */
        void log(tiny::Stream<std::uint32_t> &s) const;
    };

    //! Gets thrown by the lexer when a program can't be tokenized.
    struct LexError : tiny::CompilerError {
        using CompilerError::CompilerError; // Inherit the constructor
    };

    //! Gets thrown by the parser when a given program's tokens are syntactically invalid.
    struct ParseError : tiny::CompilerError {
        using CompilerError::CompilerError; // Inherit the constructor
    };

    //! Gets thrown by the semantic analyzer when the code is syntactically valid, yet makes no semantic sense
    struct SemanticError : tiny::CompilerError {
        using CompilerError::CompilerError; // Inherit the constructor
    };

    //! Gets thrown by the semantic analyzer when an operation is done between operators of incompatible types
    struct IncompatibleTypesError : tiny::SemanticError {
        using SemanticError::SemanticError; // Inherit the constructor
    };

    //! Gets thrown by the semantic analyzer when an operation requires a type type to be defined, but is yet unknown
    struct UnknownTypeError : tiny::SemanticError {
        using SemanticError::SemanticError; // Inherit the constructor
    };

    //! Gets thrown by the semantic analyzer when an operation requires a type type to be defined, but is yet unknown
    struct InvalidTypeError : tiny::SemanticError {
        using SemanticError::SemanticError; // Inherit the constructor
    };

    //! Base error for failed fetch operations over an AST. Narrower errors should be preferred over this generic one
    struct BadASTError : tiny::CompilerError {
        using CompilerError::CompilerError; // Inherit the constructor
    };

    //! Gets thrown when a fetch over an AST node yielded no results, but one was explicitly expected
    struct NoSuchChild : tiny::BadASTError {
        using BadASTError::BadASTError; // Inherit the constructor
    };

    //! Gets thrown when a fetch over the string value of an AST node yielded no results, but one was explicitly expected
    struct NoSuchValue : tiny::BadASTError {
        using BadASTError::BadASTError; // Inherit the constructor
    };

    //! Gets thrown when a fetch over a parameter inside a AST node yielded no results, but one was explicitly expected
    struct NoSuchParameter : tiny::BadASTError {
        using BadASTError::BadASTError; // Inherit the constructor
    };


    //! Gets thrown when a script in the pipeline reports an error
    struct PipelineError : public std::exception {
        //! Name of the stage
        const std::string &name;

        //! Name of the step
        const std::string &step_name;

        //! An optional message provided by the script
        const std::string &msg;

        //! Description of the error
        std::string detail;

        /*!
         * \brief Creates a new PipelineError error exception
         * \param s Stage that threw the error
         * \param msg An optional message provided from the script that explains the error
         */
        explicit PipelineError(const std::string &name, const std::string &step_name, const std::string &msg) : name(
                name), step_name(step_name), msg(msg) {
            detail =
                    "The pipeline failed for script '" + name + "' in step " + step_name + " with the message '" + msg +
                    "'";
        };

        /*!
         * \brief Returns a C-string detailing the error
         * \return A C-string with an explanation of the error
         */
        [[nodiscard]] const char *what() const noexcept override {
            return detail.c_str();
        }
    };

    //! Gets thrown when a script in the pipeline rejects the input
    struct PipelineRejectError : tiny::PipelineError {
        explicit PipelineRejectError(const std::string &name, const std::string &step_name, const std::string &msg)
                : tiny::PipelineError(name, step_name, msg) {
            detail =
                    "The pipeline failed for script '" + name + "' in step " + step_name + " with the message '" + msg +
                    "'";
        };

        /*!
         * \brief Returns a C-string detailing the error
         * \return A C-string with an explanation of the error
         */
        [[nodiscard]] const char *what() const noexcept override {
            return detail.c_str();
        }
    };

    //! Gets thrown when a file error is reported (such as not found or unable to open)
    struct FileError : public std::exception {
        //! An optional message describing the error
        std::string msg = "File error";

        /*!
         * \brief Creates a new FileError with no details message
         */
        explicit FileError() = default;

        /*!
         * \brief Creates a new FileError with a describing error
         * \param msg An optional message that describes the error
         */
        explicit FileError(std::string msg) : msg(std::move(msg)) {};

        /*!
         * \brief Returns a C-string detailing the error
         * \return A C-string with an explanation of the error
         */
        [[nodiscard]] const char *what() const noexcept override {
            return msg.c_str();
        }
    };

    //! Gets thrown when the metafile is not found
    struct MetaNotFoundError : tiny::FileError {
        using FileError::FileError; // Inherit the constructor
    };

    //! Gets thrown when the more than one metafiles are found
    struct TooManyMetaFiles : tiny::FileError {
        using FileError::FileError; // Inherit the constructor
    };

    //! Gets thrown when no Tiny source files are found
    struct SourcesNotFoundError : tiny::FileError {
        using FileError::FileError; // Inherit the constructor
    };

    struct CLIError : public std::exception {
        //! A message describing the error
        std::string msg = "Command error";

        /*!
         * \brief Creates a new CLIError
         */
        explicit CLIError(std::string msg): msg(std::move(msg)) {};
    };
}

#endif //TINY_ERRORS_H
