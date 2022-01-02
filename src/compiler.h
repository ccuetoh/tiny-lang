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
    enum class CompilationStatus {
        Error,
        Ok,
    };

    struct CompilationErrorDetail {
        tiny::CompilationStep step;
        std::string msg;
    };

    struct CompilationResult {
        CompilationStatus status;
        CompilationErrorDetail error{};
    };

    // TODO Compilation settings

    class Compiler {
    public:
        explicit Compiler() = default;

        [[nodiscard]] static std::string getSignature();
        void setLogger(tiny::Logger *logger);

        tiny::CompilationResult compile();

    private:
        tiny::Logger *logger = nullptr;

        tiny::Pipeline pl = tiny::Pipeline();
        tiny::FileSelector fileSelector{};

        void log(tiny::LogLv lv, const std::string &msg);
    };
}

#endif //TINY_COMPILER_H
