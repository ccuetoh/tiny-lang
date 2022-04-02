#include "compiler.h"

std::string tiny::Compiler::getSignature() {
    return TINY_NAME + " " + TINY_VERSION + " (" + TINY_VERSION_NICKNAME + ")";
}

tiny::CompilationResult tiny::Compiler::compile() {
    // Run the compilation steps in sequence, and then apply the pipeline to the stage

    tiny::debug(getSignature());
    tiny::debug("File selection stage");

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

    tiny::debug("Running file selection pipe with length " +
                            std::to_string(pl.getPipeLength(tiny::CompilationStep::FileSelection)));
    files = pl.runFileSelectionPipe(files);

    tiny::debug("Selected " + std::to_string(files.size()) + " files: ");
    for (auto const &f: files) {
        tiny::debug("\t" + f.path.string());
    }

    // With the sources selected we run each one of them in the compiler
    std::vector<tiny::ASTFile> astFiles;
    for (auto const &f: files) {
        if (f.type == tiny::FileType::Meta) {
            // TODO Parse metafile
            continue;
        }

        tiny::info("Compiling " + f.path.filename().string());

        std::ifstream filestream(f.path);
        tiny::Stream charStream(filestream);

        tiny::Lexer lexer(charStream);
        lexer.setMetadataFilename(f.path.filename().string());
        std::vector<tiny::Lexeme> lexemes;

        tiny::debug("Lexing..");

        /*
         * Lexing stage
         *
         * Separate every file in lexemes that can be used to build the parse tree
         */

        try {
            lexemes = lexer.lexAll();
        } catch (tiny::LexError &e) {
            tiny::error(e.what());
            e.log(charStream);
            tiny::fatal("Invalid program");

            return {tiny::CompilationStatus::Error, {tiny::CompilationStep::Lexer, e.what()}};
        }

        tiny::debug("Running lex pipe with length " + std::to_string(pl.getPipeLength(tiny::CompilationStep::Lexer)));
        lexemes = pl.runLexPipe(lexemes);

        tiny::Stream<tiny::Lexeme> lexemeStream(lexemes, tiny::Lexeme(tiny::Token::None));
        tiny::Parser parser(lexemeStream);

        tiny::debug("Parsing..");

        /*
         * Parse stage
         *
         * Use the lexemes to build an AST (Parse tree) that can represent the relationship between the lexemes
         */

        tiny::ASTFile astFile;
        try {
            astFile = parser.file(f.path.filename().string());
        } catch (tiny::ParseError &e) {
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

        tiny::debug("Running parse pipe with length " + std::to_string(pl.getPipeLength(tiny::CompilationStep::Parser)));
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

    tiny::info("Done");

    return {tiny::CompilationStatus::Ok};
}
