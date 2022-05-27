#include "compiler.h"
#include "config.h"
#include "symtab.h"
#include "errors.h"

std::string tiny::Compiler::getSignature() {
    return TINY_NAME + " " + TINY_VERSION + " (" + TINY_VERSION_NICKNAME + ")";
}

tiny::CompilationResult tiny::Compiler::compile() const {
    // Run the compilation steps in sequence, and then apply the pipeline to the stage

    tiny::debug(getSignature());
    tiny::debug("Selecting files..");

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
        tiny::fatal(e.what());
        return {tiny::CompilationStatus::Error, {tiny::CompilationStep::FileSelection, e.what()}};
    }

    std::vector<tiny::File> sources;
    try {
        sources = fileSelector.getLocalSourceFiles();
    } catch (const tiny::SourcesNotFoundError &e) {
        tiny::fatal( e.what());
        return {tiny::CompilationStatus::Error, {tiny::CompilationStep::FileSelection, e.what()}};
    }

    std::vector<tiny::File> files(sources.begin(), sources.end());
    files.push_back(meta);

    /*
    tiny::debug("Running file selection pipe with length " +
                            std::to_string(pl.getPipeLength(tiny::CompilationStep::FileSelection)));
    files = pl.runFileSelectionPipe(files);
     */

    tiny::debug("Got " + std::to_string(files.size()) + " files: ");
    for (auto const &f: files) {
        tiny::debug("  " + f.path.string());
    }

    // With the sources selected we run each one of them in the compiler
    std::vector<tiny::ASTFile> astFiles;
    for (auto const &f: files) {
        if (f.type == tiny::FileType::Meta) {
            // TODO Parse metafile
            continue;
        }

        tiny::debug(f, "Running compiler..");

        std::ifstream filestream(f.path);
        tiny::Stream charStream(filestream);

        tiny::Lexer lexer(charStream);
        lexer.setMetadataFile(f);
        std::vector<tiny::Lexeme> lexemes;

        tiny::debug(f, "Lexing..");

        /*
         * Lexing stage
         *
         * Separate every file in lexemes that can be used to build the parse tree
         */

        try {
            lexemes = lexer.lexAll();
        } catch (const tiny::LexError &e) {
            tiny::error(e.what());
            e.log(charStream);
            tiny::fatal("Invalid program");

            return {tiny::CompilationStatus::Error, {tiny::CompilationStep::Lexer, e.what()}};
        }

        /*
        tiny::debug("Running lex pipe with length " + std::to_string(pl.getPipeLength(tiny::CompilationStep::Lexer)));
        lexemes = pl.runLexPipe(lexemes);
         */

        tiny::Stream<tiny::Lexeme> lexemeStream(lexemes);
        tiny::Parser parser(lexemeStream);

        tiny::debug(f, "Parsing..");

        /*
         * Parse stage
         *
         * Use the lexemes to build an AST (Parse tree) that can represent the relationship between the lexemes
         */

        tiny::ASTFile astFile;
        try {
            astFile = parser.file(f);
        } catch (const tiny::ParseError &e) {
            tiny::error(e.what());
            e.log(charStream);
            tiny::fatal("Invalid program");

            return {tiny::CompilationStatus::Error, {tiny::CompilationStep::Parser, e.what()}};

        } catch (const std::exception &e) {
            tiny::error("Exception encountered while parsing");
            tiny::error(e.what());
            tiny::fatal("Invalid program");

            return {tiny::CompilationStatus::Error, {tiny::CompilationStep::Parser, e.what()}};
        }

        /*
        tiny::debug("Running parse pipe with length " + std::to_string(pl.getPipeLength(tiny::CompilationStep::Parser)));
        astFile = pl.runParsePipe(astFile);
         */

        if (tiny::getSetting(tiny::Option::OutputASTJSON).isEnabled) {
            astFile.dumpJson(f.path.filename().string() + ".ast.json");
        }

        astFiles.push_back(astFile);

        tiny::debug(f, "Building symbol table..");

        tiny::SymbolTable symtab(astFile);
        symtab.build();
    }

    tiny::info("Done");

    return {tiny::CompilationStatus::Ok};
}
