#include <filesystem>

#include "pipeline.h"
#include "errors.h"
#include "parser.h"

template<typename Output>
std::string tiny::PipelineStage<Output>::getStepName() const {
    switch (step) {
        case tiny::CompilationStep::FileSelection:
            return "FileSelection";
        case tiny::CompilationStep::Lexer:
            return "Lexer";
        case tiny::CompilationStep::Parser:
            return "Parser";

        case tiny::CompilationStep::None:
        default:
            return "None";
    }
}

void tiny::Pipeline::addFileSelectionStage(const tiny::PipelineStage<std::vector<tiny::File>> &s) {
    fileSelectionStages.push_back(s);
}

void tiny::Pipeline::addLexStage(const tiny::PipelineStage<std::vector<tiny::Lexeme>> &s) {
    lexStages.push_back(s);
}

void tiny::Pipeline::addParseStage(const tiny::PipelineStage<tiny::ASTFile> &s) {
    parseStages.push_back(s);
}

std::uint64_t tiny::Pipeline::getPipeLength(tiny::CompilationStep step) const {
    switch (step) {
        case tiny::CompilationStep::FileSelection:
            return fileSelectionStages.size();
        case CompilationStep::Lexer:
            return lexStages.size();
        case CompilationStep::Parser:
            return parseStages.size();

        case CompilationStep::None:
        default:
            return -1;
    }
}


std::vector<tiny::File> tiny::Pipeline::runFileSelectionPipe(std::vector<tiny::File> &files) const {
    for (const auto &s: fileSelectionStages) {
        auto res = s.task(files);
        files = res.output;

        switch (res.action) {
            case tiny::StageAction::Continue:
                break;

            case tiny::StageAction::Reject:
                throw tiny::PipelineRejectError(s.name, s.getStepName(), res.msg);

            default:
                throw tiny::PipelineError(s.name, s.getStepName(), "Invalid action value");
        }
    }

    return files;
}

std::vector<tiny::Lexeme> tiny::Pipeline::runLexPipe(std::vector<tiny::Lexeme> &lexemes) const {
    for (const auto &s: lexStages) {
        auto res = s.task(lexemes);
        lexemes = res.output;

        switch (res.action) {
            case tiny::StageAction::Continue:
                break;

            case tiny::StageAction::Reject:
                throw tiny::PipelineRejectError(s.name, s.getStepName(), res.msg);

            default:
                throw tiny::PipelineError(s.name, s.getStepName(), "Invalid action value");
        }
    }

    return lexemes;
}

tiny::ASTFile tiny::Pipeline::runParsePipe(tiny::ASTFile &files) const {
    for (const auto &s: parseStages) {
        auto res = s.task(files);
        files = res.output;

        switch (res.action) {
            case tiny::StageAction::Continue:
                break;

            case tiny::StageAction::Reject:
                throw tiny::PipelineRejectError(s.name, s.getStepName(), res.msg);

            default:
                throw tiny::PipelineError(s.name, s.getStepName(), "Invalid action value");
        }
    }

    return files;
}
