#include <fstream>
#include <string>

#include "logger.h"
#include "explorer.h"
#include "lexer.h"
#include "parser.h"

const std::string TINY_NAME("Tiny Compiler");
const std::string TINY_VERSION("v0.1");
const std::string TINY_VERSION_NICKNAME("Alpaca");
const std::string TINY_LICENCE("UIUC License");
const std::string TINY_COPYRIGHT("(c) Camilo Hernández 2021-");

/*
 * This is the WIP main, and it's here just for testing.
 */

int main(int argc, char **argv) {
    tiny::Logger *logger = &tiny::Logger::getInstance();

    try {
        std::locale::global(std::locale("en_US.UTF8"));
    } catch (std::runtime_error &e) {
        logger->warning(
                "Unable to use 'en_US.UTF8' locale. Using the system's default ('" + std::locale().name() + "').");
        logger->warning("Non-ASCII characters might be unrecognized.");
    }

    if (argc == 2 && (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "version") == 0)) {
        std::cout << TINY_NAME << " " << TINY_VERSION << " (" << TINY_VERSION_NICKNAME << "). "
                  << TINY_COPYRIGHT << " " << TINY_LICENCE << std::endl;
        return 0;
    }

    // TODO Handle no such directory
    tiny::Explorer exp(R"(./sandbox)");
    exp.setSearchDepth(0);

    auto files = exp.search("main.ty");
    if (files.empty()) {
        logger->fatal("No main file found");
        return 1;
    }

    auto target = files[0];

    logger->info("Target is " + target.path().filename().string());
    logger->info("Lexing..");

    std::ifstream filestream(target.path().string());
    tiny::WalkableStream<std::uint32_t> s(filestream);

    tiny::Lexer lexer(s);
    lexer.setMetadataFilename(target.path().filename().string()); // TODO use relative or absolute path

    std::vector<tiny::Lexeme> lexemes;

    auto lexingStart = std::chrono::high_resolution_clock::now();

    try {
        lexemes = lexer.lexAll();
    } catch (tiny::LexError &e) {
        tiny::ErrorBuilder builder(e, s);

        logger->error(e.what());
        builder.log(*logger);
        logger->fatal("Invalid program");

        return 1;
    }

    auto lexingEnd = std::chrono::high_resolution_clock::now();
    auto lexingDuration = std::chrono::duration_cast<std::chrono::milliseconds>(lexingEnd - lexingStart);

    tiny::WalkableStream<tiny::Lexeme> lexemeStream(lexemes, tiny::Lexeme(tiny::Token::None));
    tiny::Parser parser(lexemeStream);

    logger->info("Parsing..");

    auto parsingStart = std::chrono::high_resolution_clock::now();

    tiny::ASTFile file;
    try {
        file = parser.file(std::filesystem::absolute(target.path()).string());
    } catch (tiny::ParseError &e) {
        tiny::ErrorBuilder builder(e, s);

        logger->error(e.what());
        builder.log(*logger);
        logger->fatal("Invalid program");

        return 1;
    } catch (std::exception &e) {
        logger->error("Exception encountered while parsing");
        logger->error(e.what());
        logger->fatal("Invalid program");

        return 1;
    }

    auto parsingEnd = std::chrono::high_resolution_clock::now();
    auto parsingDuration = std::chrono::duration_cast<std::chrono::milliseconds>(parsingEnd - parsingStart);

    logger->info("");
    logger->info("Ok");
    logger->info("Execution times:");
    logger->info("\tLexing - " + std::to_string(lexingDuration.count()) + "ms");
    logger->info("\tParsing - " + std::to_string(parsingDuration.count()) + "ms");

    // TODO Remove. Add as cmd option
    std::ofstream jsonOut;
    jsonOut.open("./sandbox/ast.json");
    jsonOut << file.toJson().dump(4);
    jsonOut.close();

    return 0;
}