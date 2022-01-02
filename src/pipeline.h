#ifndef TINY_PIPELINE_H
#define TINY_PIPELINE_H

#include <vector>
#include <fstream>
#include <functional>

#include "lexer.h"
#include "file.h"
#include "ast.h"

namespace tiny {
    //! Represents the step inside the compilation process
    enum class CompilationStep {
        None,
        FileSelection,
        Lexer,
        Parser,
    };

    //! The action taken by a stage. The stage can either Reject the code or Continue the pipeline
    enum class StageAction {
        None,
        Continue,
        Reject,
    };

    //! The expected output of the stage
    template<typename Output>
    //! The result produced by a stage. Contains the output and the action taken by the stage, with an optional message
    struct StageResult {
        //! Constructs a result with an action and no further output or message, should only be used on rejection without detail
        explicit StageResult(tiny::StageAction a): action(a) {};
        //! Constructs a result with only the output and defaults the action to Continue
        explicit StageResult(Output o): output(o) {};
        //! Constructs a result without output
        StageResult(tiny::StageAction a, std::string_view msg): action(a), msg(msg){};
        //! Constructs a result without detail message
        StageResult(tiny::StageAction a, Output o): action(a), output(o) {};
        //! Full constructor for the result
        StageResult(tiny::StageAction a, Output o, std::string_view msg): action(a), output(o), msg(msg) {};

        //! The action which the stage decided to take
        tiny::StageAction action = tiny::StageAction::Continue;

        //! The output of the stage
        Output output;
        //! An optional message that explains the result. Should always be set when the action is Reject
        std::string msg;
    };

    template<typename Output>
    /*!
     * \brief A stage is a script that gets injected inside the compilation Pipeline
     * \tparam Output The expected output of the stage
     *
     * A stage is a script that gets injected inside the compilation Pipeline. It is essentially a function that runs
     * after a step in the compiler, and receives as input the steps' or other stage's output and can modify the values
     * that get passed to the next step of compilation.
     */
    struct PipelineStage {
        //! Full constructor
        PipelineStage(std::string_view name, std::function<StageResult<Output>(Output)> task): name(name), task(task) {};

        //! Arbitrary name for this stage
        std::string name;
        //! The step which this stage targets
        tiny::CompilationStep step = tiny::CompilationStep::None;

        //! The task (function) that this stages executes
        std::function<StageResult<Output>(Output)> task;

        /*!
         * \brief Gets a string with the name of the step
         * \return A string with the name of the step
         */

        [[nodiscard]] std::string getStepName() const;
    };

    /*!
     * \brief The pipeline simplifies the execution of scripts (stages) between steps in the compilation process
     *
     * The pipeline simplifies the execution of scripts (stages) between steps in the compilation process. It holds
     * all the loaded stages and runs them back-to-back for a given stage until a final output is reached. This output
     * is then run through the next compilation step.
     */
    class Pipeline {
        //! The stages to run after file selection
        std::vector<tiny::PipelineStage<std::vector<tiny::File>>> fileSelectionStages;
        //! The stages to run after lexing the files
        std::vector<tiny::PipelineStage<std::vector<tiny::Lexeme>>> lexStages;
        //! The stages to run after parsing the files
        std::vector<tiny::PipelineStage<tiny::ASTFile>> parseStages;

    public:
        //! Default constructor
        Pipeline() = default;

        /*!
         * \brief Adds a stage to the file selection pipe
         * \param s A stage
         */
        void addFileSelectionStage(const tiny::PipelineStage<std::vector<tiny::File>> &s);
        /*!
         * \brief Adds a stage to the lexer pipe
         * \param s A stage
         */
        void addLexStage(const tiny::PipelineStage<std::vector<tiny::Lexeme>> &s);
        /*!
         * \brief Adds a stage to the parser pipe
         * \param s A stage
         */
        void addParseStage(const tiny::PipelineStage<tiny::ASTFile> &s);

        /*!
         * \brief Runs back-to-back all the stages inside the file selection pipe
         * \param files The output of the file selector
         * \return The output of the last stage, or the unmodified input if no stages are set
         */
        std::vector<tiny::File> runFileSelectionPipe(std::vector<tiny::File>& files) const;
        /*!
         * \brief Runs back-to-back all the stages inside the lexer pipe
         * \param lexemes The output of the lexer
         * \return The output of the last stage, or the unmodified input if no stages are set
         */
        std::vector<tiny::Lexeme> runLexPipe(std::vector<tiny::Lexeme> &lexemes) const;
        /*!
         * \brief Runs back-to-back all the stages inside the parser pipe
         * \param files The output of the parser
         * \return The output of the last stage, or the unmodified input if no stages are set
         */
        tiny::ASTFile runParsePipe(tiny::ASTFile &file) const;

        /*!
         * \brief Gets the length of a pipeline
         * \param step The pipeline of which to calculate the length
         * \return The number of stages in the selected pipeline
         */
        [[nodiscard]] std::uint64_t getPipeLength(tiny::CompilationStep step) const;
    };
}



#endif //TINY_PIPELINE_H
