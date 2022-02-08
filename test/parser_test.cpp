#include "gtest/gtest.h"

#include <parser.h>

TEST(Parser, Module) {
    return;

    /*
    TODO
    std::stringstream data;
    data << "module example\n"
            "\n"
            "func main() {\n"
            "    var := -1 + (-1 + 1)\n"
            "}\n";

    tiny::Lexer lexer(data);
    tiny::Stream<tiny::Lexeme> lexemes(lexer.lexAll());

    tiny::Parser p(lexemes);

    const auto path = "path/to/file";
    auto ast = p.file(path);

    tiny::ASTFile::fromJson();

    return;
    */
}

