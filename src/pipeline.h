#ifndef TINY_PIPELINE_H
#define TINY_PIPELINE_H

#include <vector>
#include <fstream>
#include <functional>

#include "lexer.h"
#include "file.h"
#include "ast.h"

namespace tiny {
    enum class CompilationStep {
        None,
        FileSelection,
        Lexer,
        Parser,
    };

    enum class StageAction {
        None,
        Continue,
        Reject,
    };

    template<typename Output>
    struct StageResult {
        explicit StageResult(tiny::StageAction a): action(a) {};
        explicit StageResult(Output o): output(o) {};
        StageResult(tiny::StageAction a, std::string_view msg): action(a), msg(msg){};
        StageResult(tiny::StageAction a, Output o): action(a), output(o) {};
        StageResult(tiny::StageAction a, Output o, std::string_view msg): action(a), output(o), msg(msg) {};

        tiny::StageAction action = tiny::StageAction::Continue;

        Output output;
        std::string msg;
    };

    template<typename Output>
    struct Stage {
        Stage(std::string_view name, std::function<StageResult<Output>(Output)> task): name(name), task(task) {};

        std::string name;
        tiny::CompilationStep step;

        std::function<StageResult<Output>(Output)> task;

        [[nodiscard]] std::string getStepName() const;
    };

    class Pipeline {
        std::vector<tiny::Stage<std::vector<tiny::File>>> fileSelectionStages;
        std::vector<tiny::Stage<std::vector<tiny::Lexeme>>> lexStages;
        std::vector<tiny::Stage<tiny::ASTFile>> parseStages;

    public:
        Pipeline() = default;

        void addFileSelectionStage(const tiny::Stage<std::vector<tiny::File>> &s);
        void addLexStage(const tiny::Stage<std::vector<tiny::Lexeme>> &s);
        void addParseStage(const tiny::Stage<tiny::ASTFile> &s);

        std::vector<tiny::File> runFileSelectionPipe(std::vector<tiny::File>& files);
        std::vector<tiny::Lexeme> runLexPipe(std::vector<tiny::Lexeme> &lexemes);
        tiny::ASTFile runParsePipe(tiny::ASTFile &file);

        std::uint64_t getPipeLength(tiny::CompilationStep step) const;
    };
}



#endif //TINY_PIPELINE_H
