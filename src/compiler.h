#ifndef TINY_COMPILER_H
#define TINY_COMPILER_H

#include "logger.h"
#include "pipeline.h"
#include "parser.h"
#include "file.h"

const std::string TINY_NAME("Tiny Compiler");
const std::string TINY_VERSION("v0.1");
const std::string TINY_VERSION_NICKNAME("Alpaca");
const std::string TINY_LICENCE("UIUC License");
const std::string TINY_COPYRIGHT("(c) Camilo Hernández 2021-");

namespace tiny {
    //! The status of the compilation. Either Error or Ok
    enum class CompilationStatus {
        //! Error during compilation
        Error,
        //! Successful compilation
        Ok,
    };

    //! Contains the detail of the error if encountered. It consists of the step in which the error occurred and a message
    struct CompilationErrorDetail {
        //! The step in which an error occurred
        tiny::CompilationStep step;
        //! A message describing the error
        std::string msg;
    };

    //! Contains the status of the compilation, and an erro detail if one was encountered
    struct CompilationResult {
        //! Status of the compilation
        tiny::CompilationStatus status = tiny::CompilationStatus::Ok;

        //! If an error was encountered, a description of the error
        tiny::CompilationErrorDetail error{};
    };

    // TODO Compilation settings

    /*!
     * \brief A wrapper for all the compilation steps plus settings, logging and pipeline management
     */
    class Compiler {
    public:
        //! Default builder, no parameters
        explicit Compiler() = default;

        /*!
         * \brief Gets a brief signature of the compiler instance, including name and version data
         * \return A string containing the signature
         */
        [[nodiscard]] static std::string getSignature();

        /*!
         * \brief Starts the compilation process
         * \return A compilation result struct, which indicates the result of the compilation
         *
         * The compiler will call the compilation steps in sequence, and run the corresponding pipeline in between steps.
         * If an error is encountered (such as an invalid program or missing files) the exception is caught inside the
         * compiler and returned as a CompilationResult with status Error and containing a non-empty CompilationErrorDetail.
         *
         * If the logger is set the compilation process will log information and errors to it. If the logger is null, logging
         * understood as disabled.
         */
        tiny::CompilationResult compile();

    private:
        //! The compilation Pipeline to support scripting
        tiny::Pipeline pl = tiny::Pipeline();
        //! The file selector to choose which files should be targeted by the compiler
        tiny::FileSelector fileSelector{};
    };
}

#endif //TINY_COMPILER_H
