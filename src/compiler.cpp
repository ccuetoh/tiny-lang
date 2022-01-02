#include "compiler.h"

void tiny::Compiler::log(tiny::LogLv lv, const std::string &msg) {
    if (logger != nullptr) {
        logger->log(lv, msg);
    }
}

void tiny::Compiler::setLogger(tiny::Logger *l) {
    logger = l;
}

std::string tiny::Compiler::getSignature() {
    return TINY_NAME + " "  + TINY_VERSION + " (" + TINY_VERSION_NICKNAME + ")";
}

tiny::CompilationResult tiny::Compiler::compile() {
    // Run the compilation steps in sequence, and then apply the pipeline to the stage

    log(tiny::LogLv::Debug, getSignature());

    log(tiny::LogLv::Debug, "File selection stage");

    tiny::File meta;

    /*
     * File-selection step
     *
     * The file selection stage uses the FileSelector to pick the files targeted by the compiler, and fetches the metadata
     * file. If no metadata or/and source files are found error out.
     */

    try {
        meta = fileSelector.getMetaFile();
    } catch (const tiny::MetaNotFoundError &e) {
        log(tiny::LogLv::Fatal, e.what());
        return {tiny::CompilationStatus::Error, {tiny::CompilationStep::FileSelection, e.what()}};
    }

    std::vector<tiny::File> sources;
    try {
        sources = fileSelector.getLocalSourceFiles();
    } catch (const tiny::SourcesNotFoundError &e) {
        log(tiny::LogLv::Fatal, e.what());
        return {tiny::CompilationStatus::Error, {tiny::CompilationStep::FileSelection, e.what()}};
    }

    std::vector<tiny::File> files(sources.begin(), sources.end());
    files.push_back(meta);

    log(tiny::LogLv::Debug, "Running file selection pipe with length " + std::to_string(pl.getPipeLength(tiny::CompilationStep::FileSelection)));
    files = pl.runFileSelectionPipe(files);

    log(tiny::LogLv::Debug, "Selected " + std::to_string(files.size()) + " files: ");
    for (auto const &f: files) {
        log(tiny::LogLv::Debug, "\t" + f.path.string());
    }

    // With the sources selected we run each one of them in the compiler
    std::vector<tiny::ASTFile> astFiles;
    for (auto const &f: files) {
        if (f.type == tiny::FileType::Meta) {
            // TODO Parse metafile
            continue;
        }

        log(tiny::LogLv::Info, "Compiling " + f.path.filename().string());

        std::ifstream filestream(f.path);
        tiny::WalkableStream<std::uint32_t> charStream(filestream);

        tiny::Lexer lexer(charStream);
        std::vector<tiny::Lexeme> lexemes;

        log(tiny::LogLv::Debug, "Lexing..");

        /*
         * Lexing stage
         *
         * Separate every file in lexemes that can be used to build the parse tree
         */

        try {
            lexemes = lexer.lexAll();
        } catch (tiny::LexError &e) {
            if (logger != nullptr) {
                tiny::ErrorBuilder builder(e, charStream);

                log(tiny::LogLv::Error,e.what());
                builder.log(*logger);
                log(tiny::LogLv::Fatal,"Invalid program");
            }

            return {tiny::CompilationStatus::Error, {tiny::CompilationStep::Lexer, e.what()}};
        }

        log(tiny::LogLv::Debug, "Running lex pipe with length " + std::to_string(pl.getPipeLength(tiny::CompilationStep::Lexer)));
        lexemes = pl.runLexPipe(lexemes);

        tiny::WalkableStream<tiny::Lexeme> lexemeStream(lexemes, tiny::Lexeme(tiny::Token::None));
        tiny::Parser parser(lexemeStream);

        log(tiny::LogLv::Debug,"Parsing..");

        /*
         * Parse stage
         *
         * Use the lexemes to build an AST (Parse tree) that can represent the relationship between the lexemes
         */

        tiny::ASTFile astFile;
        try {
            astFile = parser.file(f.path.filename().string());
        } catch (tiny::ParseError &e) {
            if (logger != nullptr) {
                tiny::ErrorBuilder builder(e, charStream);

                logger->error(e.what());
                builder.log(*logger);
                logger->fatal("Invalid program");
            }

            return {tiny::CompilationStatus::Error, {tiny::CompilationStep::Parser, e.what()}};

        } catch (const std::exception &e) {
            log(tiny::LogLv::Error,"Exception encountered while parsing");
            log(tiny::LogLv::Error,e.what());
            log(tiny::LogLv::Fatal,"Invalid program");

            return {tiny::CompilationStatus::Error, {tiny::CompilationStep::Parser, e.what()}};
        }

        log(tiny::LogLv::Debug, "Running parse pipe with length " + std::to_string(pl.getPipeLength(tiny::CompilationStep::Parser)));
        astFile = pl.runParsePipe(astFile);

        /*
         * For now we just output the AST to a file as a JSON document
         */

        // TODO Move to argument option
        std::ofstream jsonOut;
        jsonOut.open("" + f.path.filename().string() + ".ast.json");
        jsonOut << astFile.toJson().dump(4);
        jsonOut.close();

        astFiles.push_back(astFile);
    }

    log(tiny::LogLv::Info,"Done");

    return {tiny::CompilationStatus::Ok};
}
