#include "gtest/gtest.h"

#include <vector>
#include <random>
#include <string>
#include <iterator>
#include <chrono>
#include <fstream>

#include "stream.h"
#include "lexer.h"
#include "errors.h"

// Some helper functions for the benchmark

std::random_device rd;
std::mt19937 randomGen(rd());

tiny::UnicodeCodepoints idRandChars = tiny::UnicodeParser::fromString(
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzáÁäÄçÇúÚüÜéÉëËóÓöÖíÍïÏ_");
std::map<std::string, tiny::Lexeme> tokenRand{
        {"func",          tiny::Lexeme(tiny::Token::KwFunc)},
        {"as",            tiny::Lexeme(tiny::Token::KwAs)},
        {"for",           tiny::Lexeme(tiny::Token::KwFor)},
        {"{",             tiny::Lexeme(tiny::Token::OBraces)},
        {"}",             tiny::Lexeme(tiny::Token::CBraces)},
        {"(",             tiny::Lexeme(tiny::Token::OParenthesis)},
        {")",             tiny::Lexeme(tiny::Token::CParenthesis)},
        {"return",        tiny::Lexeme(tiny::Token::KwReturn)},
        {"struct",        tiny::Lexeme(tiny::Token::KwStruct)},
        {"in",            tiny::Lexeme(tiny::Token::KwIn)},
        {"1.0",           tiny::Lexeme(tiny::Token::LiteralNum, "1.0")},
        {"1",             tiny::Lexeme(tiny::Token::LiteralNum, "1")},
        {"\n",            tiny::Lexeme(tiny::Token::NewLine)},
        {"99999",         tiny::Lexeme(tiny::Token::LiteralNum, "99999")},
        {"\"hi\"",        tiny::Lexeme(tiny::Token::LiteralStr, "hi")},
        {"\"bye\"",       tiny::Lexeme(tiny::Token::LiteralStr, "bye")},
        {"\"None\"",      tiny::Lexeme(tiny::Token::LiteralStr, "None")},
        {"\'a\'",         tiny::Lexeme(tiny::Token::LiteralChar, "a")},
        {"None",          tiny::Lexeme(tiny::Token::LiteralNone)},
        {"True",          tiny::Lexeme(tiny::Token::LiteralTrue)},
        {"/*hi\nbye\n*/", tiny::Lexeme(tiny::Token::MultilineComment, "hi\nbye\n")},
        {"/**/",          tiny::Lexeme(tiny::Token::MultilineComment, "")},
};

tiny::Lexeme randomId() {
    std::string id;
    do {
        std::shuffle(idRandChars.begin(), idRandChars.end(), randomGen);

        std::uniform_int_distribution<std::int32_t> uni(1, std::int32_t(idRandChars.size() - 1));
        std::int32_t size = uni(randomGen);

        id = tiny::UnicodeParser::toString(tiny::UnicodeCodepoints(idRandChars.begin(), idRandChars.begin() + size));
    } while (isdigit(id[0]));

    return tiny::Lexeme(tiny::Token::Id, id);
}

std::pair<std::string, tiny::Lexeme> randomLexeme() {
    std::uniform_int_distribution<std::int32_t> zeroToOne(0, 1);
    if (zeroToOne(randomGen) < .333) { // 1/3 chance
        auto id = randomId();
        return {id.value, id};
    }

    std::uniform_int_distribution<std::int32_t> randomTokenRand(1, tokenRand.size() - 1);

    auto item = tokenRand.begin();
    std::advance(item, randomTokenRand(randomGen));

    return {item->first, item->second};
}

TEST(Lexer, LocaleSetup) {
    std::locale::global(std::locale("en_US.UTF8"));
}

TEST(Lexer, NewLine) {
    std::stringstream data;
    data << "\n";

    tiny::Lexer lexer(data);

    ASSERT_TRUE(lexer.lex() == tiny::Token::NewLine);
}

TEST(Lexer, EmptyStream) {
    std::stringstream data;
    tiny::Lexer lexer(data);

    ASSERT_FALSE(lexer);
}

TEST(Lexer, Keywords) {
    std::stringstream data;
    data << "const import module struct func as for in return and or if else trait struct";

    tiny::Lexer lexer(data);

    try {
        auto lexemes = lexer.lexAll();
        std::vector<tiny::Lexeme> expect{
                tiny::Lexeme(tiny::Token::KwConst),
                tiny::Lexeme(tiny::Token::KwImport),
                tiny::Lexeme(tiny::Token::KwModule),
                tiny::Lexeme(tiny::Token::KwStruct),
                tiny::Lexeme(tiny::Token::KwFunc),
                tiny::Lexeme(tiny::Token::KwAs),
                tiny::Lexeme(tiny::Token::KwFor),
                tiny::Lexeme(tiny::Token::KwIn),
                tiny::Lexeme(tiny::Token::KwReturn),
                tiny::Lexeme(tiny::Token::KwAnd),
                tiny::Lexeme(tiny::Token::KwOr),
                tiny::Lexeme(tiny::Token::KwIf),
                tiny::Lexeme(tiny::Token::KwElse),
                tiny::Lexeme(tiny::Token::KwTrait),
                tiny::Lexeme(tiny::Token::KwStruct),
        };

        ASSERT_EQ(lexemes, expect);
    } catch (tiny::LexError const &e) {
        std::cout << e.what() << std::endl;
        FAIL();
    }
}

TEST(Lexer, Ints) {
    std::stringstream data;
    data << "int int16 int32 int64 uint uint16 uint32 uint64";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::TypeInt32),
            tiny::Lexeme(tiny::Token::TypeInt16),
            tiny::Lexeme(tiny::Token::TypeInt32),
            tiny::Lexeme(tiny::Token::TypeInt64),
            tiny::Lexeme(tiny::Token::TypeUInt32),
            tiny::Lexeme(tiny::Token::TypeUInt16),
            tiny::Lexeme(tiny::Token::TypeUInt32),
            tiny::Lexeme(tiny::Token::TypeUInt64),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, Fixed) {
    std::stringstream data;
    data << "fixed fixed16 fixed32 fixed64 ufixed ufixed16 ufixed32 ufixed64";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::TypeFixed32),
            tiny::Lexeme(tiny::Token::TypeFixed16),
            tiny::Lexeme(tiny::Token::TypeFixed32),
            tiny::Lexeme(tiny::Token::TypeFixed64),
            tiny::Lexeme(tiny::Token::TypeUFixed32),
            tiny::Lexeme(tiny::Token::TypeUFixed16),
            tiny::Lexeme(tiny::Token::TypeUFixed32),
            tiny::Lexeme(tiny::Token::TypeUFixed64),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, Float) {
    std::stringstream data;
    data << "float float16 float32 float64";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::TypeFloat32),
            tiny::Lexeme(tiny::Token::TypeFloat16),
            tiny::Lexeme(tiny::Token::TypeFloat32),
            tiny::Lexeme(tiny::Token::TypeFloat64),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, Bools) {
    std::stringstream data;
    data << "bool True False";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::TypeBool),
            tiny::Lexeme(tiny::Token::LiteralTrue),
            tiny::Lexeme(tiny::Token::LiteralFalse),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, BoolsLowercase) {
    std::stringstream data;
    data << "true false";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();

    // Since the values are lowercase ("true" not "True") it's expected that they get interpreted as an ID and not
    // a boolean literal.
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::Id, "true"),
            tiny::Lexeme(tiny::Token::Id, "false"),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, Any) {
    std::stringstream data;
    data << "any";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::TypeAny),
    };

    ASSERT_EQ(lexemes, expect);
}


TEST(Lexer, Operands) {
    std::stringstream data;
    data << "+ - *  ** / += -= *= /= =";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::Sum),
            tiny::Lexeme(tiny::Token::Sub),
            tiny::Lexeme(tiny::Token::Multi),
            tiny::Lexeme(tiny::Token::Exp),
            tiny::Lexeme(tiny::Token::Div),
            tiny::Lexeme(tiny::Token::AssignSum),
            tiny::Lexeme(tiny::Token::AssignSub),
            tiny::Lexeme(tiny::Token::AssignMulti),
            tiny::Lexeme(tiny::Token::AssignDiv),
            tiny::Lexeme(tiny::Token::Assign),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, Comparators) {
    std::stringstream data;
    data << "== !=  > >= < <=";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::Eq),
            tiny::Lexeme(tiny::Token::Neq),
            tiny::Lexeme(tiny::Token::Gt),
            tiny::Lexeme(tiny::Token::Gteq),
            tiny::Lexeme(tiny::Token::Lt),
            tiny::Lexeme(tiny::Token::Lteq),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, Symbols) {
    std::stringstream data;
    data << ", \n  := ! !!";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::Comma),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::Init),
            tiny::Lexeme(tiny::Token::Negation),
            tiny::Lexeme(tiny::Token::Doublebang),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, NumLiterals) {
    std::stringstream data;
    data << "1.0 1 2 3 4 555555555555555 1.5555555555555555";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::LiteralNum, "1.0"),
            tiny::Lexeme(tiny::Token::LiteralNum, "1"),
            tiny::Lexeme(tiny::Token::LiteralNum, "2"),
            tiny::Lexeme(tiny::Token::LiteralNum, "3"),
            tiny::Lexeme(tiny::Token::LiteralNum, "4"),
            tiny::Lexeme(tiny::Token::LiteralNum, "555555555555555"),
            tiny::Lexeme(tiny::Token::LiteralNum, "1.5555555555555555"),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, StrLiterals) {
    std::stringstream data;
    data << "\"hi\"  \"bye\" \"foo\" \"bar\" \"hí\" \"ó!alw'q_./return     1  2\"";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::LiteralStr, "hi"),
            tiny::Lexeme(tiny::Token::LiteralStr, "bye"),
            tiny::Lexeme(tiny::Token::LiteralStr, "foo"),
            tiny::Lexeme(tiny::Token::LiteralStr, "bar"),
            tiny::Lexeme(tiny::Token::LiteralStr, "hí"),
            tiny::Lexeme(tiny::Token::LiteralStr, "ó!alw'q_./return     1  2"),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, NoSpaces) {
    std::stringstream data;
    data << ",!=!\n/";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::Comma),
            tiny::Lexeme(tiny::Token::Neq),
            tiny::Lexeme(tiny::Token::Negation),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::Div),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, SinglelineComment) {
    std::stringstream data;
    data << "//This is a comment\nvar1 := 0";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::SinglelineComment, "This is a comment"),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::Id, "var1"),
            tiny::Lexeme(tiny::Token::Init),
            tiny::Lexeme(tiny::Token::LiteralNum, "0"),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, MultilineComment) {
    std::stringstream data;
    data << "/*\nThis\nis\na\ncomment\n*/\nvar1 := 0";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::MultilineComment, "\nThis\nis\na\ncomment\n"),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::Id, "var1"),
            tiny::Lexeme(tiny::Token::Init),
            tiny::Lexeme(tiny::Token::LiteralNum, "0"),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, Unicode) {
    std::locale::global(std::locale("en_US.UTF8"));

    std::stringstream data;
    data << "func máïn(){\n//úñícÖdé\n}";

    tiny::Lexer lexer(data);

    try {
        auto lexemes = lexer.lexAll();
        std::vector<tiny::Lexeme> expect{
                tiny::Lexeme(tiny::Token::KwFunc),
                tiny::Lexeme(tiny::Token::Id, "máïn"),
                tiny::Lexeme(tiny::Token::OParenthesis),
                tiny::Lexeme(tiny::Token::CParenthesis),
                tiny::Lexeme(tiny::Token::OBraces),
                tiny::Lexeme(tiny::Token::NewLine),
                tiny::Lexeme(tiny::Token::SinglelineComment, "úñícÖdé"),
                tiny::Lexeme(tiny::Token::NewLine),
                tiny::Lexeme(tiny::Token::CBraces),
        };

        ASSERT_EQ(lexemes, expect);
    }
    catch (std::exception const &e) {
        std::cout << e.what() << std::endl;
        FAIL();
    }
}


TEST(Lexer, StructDeclaration) {
    std::stringstream data;
    data << "struct test {\n"
            "    testCompose\n"
            "    string testVariable\n"
            "}\n";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::KwStruct),
            tiny::Lexeme(tiny::Token::Id, "test"),
            tiny::Lexeme(tiny::Token::OBraces),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::Id, "testCompose"),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::TypeString),
            tiny::Lexeme(tiny::Token::Id, "testVariable"),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::CBraces),
            tiny::Lexeme(tiny::Token::NewLine)
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, TraitDeclaration) {
    std::stringstream data;
    data << "trait test1 [test2, test3] {\n"
            "    string testVariable,\n"
            "    func testFunc(int, string, customType)\n"
            "}\n";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::KwTrait),
            tiny::Lexeme(tiny::Token::Id, "test1"),
            tiny::Lexeme(tiny::Token::OBrackets),
            tiny::Lexeme(tiny::Token::Id, "test2"),
            tiny::Lexeme(tiny::Token::Comma),
            tiny::Lexeme(tiny::Token::Id, "test3"),
            tiny::Lexeme(tiny::Token::CBrackets),
            tiny::Lexeme(tiny::Token::OBraces),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::TypeString),
            tiny::Lexeme(tiny::Token::Id, "testVariable"),
            tiny::Lexeme(tiny::Token::Comma),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::KwFunc),
            tiny::Lexeme(tiny::Token::Id, "testFunc"),
            tiny::Lexeme(tiny::Token::OParenthesis),
            tiny::Lexeme(tiny::Token::TypeInt32),
            tiny::Lexeme(tiny::Token::Comma),
            tiny::Lexeme(tiny::Token::TypeString),
            tiny::Lexeme(tiny::Token::Comma),
            tiny::Lexeme(tiny::Token::Id, "customType"),
            tiny::Lexeme(tiny::Token::CParenthesis),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::CBraces),
            tiny::Lexeme(tiny::Token::NewLine)
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, TestProgram1) {
    std::stringstream data;
    data << "module example\n"
            "\n"
            "func main() {\n"
            "    var := -1 + (-1 + 1)\n"
            "}\n";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::KwModule),
            tiny::Lexeme(tiny::Token::Id, "example"),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::KwFunc),
            tiny::Lexeme(tiny::Token::Id, "main"),
            tiny::Lexeme(tiny::Token::OParenthesis),
            tiny::Lexeme(tiny::Token::CParenthesis),
            tiny::Lexeme(tiny::Token::OBraces),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::Id, "var"),
            tiny::Lexeme(tiny::Token::Init),
            tiny::Lexeme(tiny::Token::Sub),
            tiny::Lexeme(tiny::Token::LiteralNum, "1"),
            tiny::Lexeme(tiny::Token::Sum),
            tiny::Lexeme(tiny::Token::OParenthesis),
            tiny::Lexeme(tiny::Token::Sub),
            tiny::Lexeme(tiny::Token::LiteralNum, "1"),
            tiny::Lexeme(tiny::Token::Sum),
            tiny::Lexeme(tiny::Token::LiteralNum, "1"),
            tiny::Lexeme(tiny::Token::CParenthesis),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::CBraces),
            tiny::Lexeme(tiny::Token::NewLine),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, TestProgram2) {
    std::stringstream data;
    data << "// Test program with various formatting issues\n"
            "module example\n"
            "func main(){\n"
            "    var1:= -1+ (-  1+11)\n"
            "    var2:=var1+testFunc(   )\n"
            "    var1=0.\n"
            "}\n"
            "func testFunc(     ){return 1}";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::SinglelineComment, " Test program with various formatting issues"),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::KwModule),
            tiny::Lexeme(tiny::Token::Id, "example"),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::KwFunc),
            tiny::Lexeme(tiny::Token::Id, "main"),
            tiny::Lexeme(tiny::Token::OParenthesis),
            tiny::Lexeme(tiny::Token::CParenthesis),
            tiny::Lexeme(tiny::Token::OBraces),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::Id, "var1"),
            tiny::Lexeme(tiny::Token::Init),
            tiny::Lexeme(tiny::Token::Sub),
            tiny::Lexeme(tiny::Token::LiteralNum, "1"),
            tiny::Lexeme(tiny::Token::Sum),
            tiny::Lexeme(tiny::Token::OParenthesis),
            tiny::Lexeme(tiny::Token::Sub),
            tiny::Lexeme(tiny::Token::LiteralNum, "1"),
            tiny::Lexeme(tiny::Token::Sum),
            tiny::Lexeme(tiny::Token::LiteralNum, "11"),
            tiny::Lexeme(tiny::Token::CParenthesis),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::Id, "var2"),
            tiny::Lexeme(tiny::Token::Init),
            tiny::Lexeme(tiny::Token::Id, "var1"),
            tiny::Lexeme(tiny::Token::Sum),
            tiny::Lexeme(tiny::Token::Id, "testFunc"),
            tiny::Lexeme(tiny::Token::OParenthesis),
            tiny::Lexeme(tiny::Token::CParenthesis),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::Id, "var1"),
            tiny::Lexeme(tiny::Token::Assign),
            tiny::Lexeme(tiny::Token::LiteralNum, "0."),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::CBraces),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::KwFunc),
            tiny::Lexeme(tiny::Token::Id, "testFunc"),
            tiny::Lexeme(tiny::Token::OParenthesis),
            tiny::Lexeme(tiny::Token::CParenthesis),
            tiny::Lexeme(tiny::Token::OBraces),
            tiny::Lexeme(tiny::Token::KwReturn),
            tiny::Lexeme(tiny::Token::LiteralNum, "1"),
            tiny::Lexeme(tiny::Token::CBraces),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, TestProgram3) {
    std::stringstream data;
    data << "module fibonacci\n"
            "\n"
            "func main() {\n"
            "    n := 0\n"
            "\n"
            "    // Get all fibonacci number under 100\n"
            "    for n < 100 {\n"
            "        n = fibonacci(n)\n"
            "    }\n"
            "}\n"
            "\n"
            "//! Returns the next number in the Fibonacci Sequence\n"
            "int fibonacci(n int)\n"
            "{\n"
            "    if n <= 1 {\n"
            "        return n\n"
            "    }\n"
            "\n"
            "    return fibonacci(n-1) + fibonacci(n-2)\n"
            "}\n";

    tiny::Lexer lexer(data);

    auto lexemes = lexer.lexAll();
    std::vector<tiny::Lexeme> expect{
            tiny::Lexeme(tiny::Token::KwModule),
            tiny::Lexeme(tiny::Token::Id, "fibonacci"),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::KwFunc),
            tiny::Lexeme(tiny::Token::Id, "main"),
            tiny::Lexeme(tiny::Token::OParenthesis),
            tiny::Lexeme(tiny::Token::CParenthesis),
            tiny::Lexeme(tiny::Token::OBraces),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::Id, "n"),
            tiny::Lexeme(tiny::Token::Init),
            tiny::Lexeme(tiny::Token::LiteralNum, "0"),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::SinglelineComment, " Get all fibonacci number under 100"),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::KwFor),
            tiny::Lexeme(tiny::Token::Id, "n"),
            tiny::Lexeme(tiny::Token::Lt),
            tiny::Lexeme(tiny::Token::LiteralNum, "100"),
            tiny::Lexeme(tiny::Token::OBraces),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::Id, "n"),
            tiny::Lexeme(tiny::Token::Assign),
            tiny::Lexeme(tiny::Token::Id, "fibonacci"),
            tiny::Lexeme(tiny::Token::OParenthesis),
            tiny::Lexeme(tiny::Token::Id, "n"),
            tiny::Lexeme(tiny::Token::CParenthesis),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::CBraces),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::CBraces),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::SinglelineComment, "! Returns the next number in the Fibonacci Sequence"),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::TypeInt32),
            tiny::Lexeme(tiny::Token::Id, "fibonacci"),
            tiny::Lexeme(tiny::Token::OParenthesis),
            tiny::Lexeme(tiny::Token::Id, "n"),
            tiny::Lexeme(tiny::Token::TypeInt32),
            tiny::Lexeme(tiny::Token::CParenthesis),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::OBraces),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::KwIf),
            tiny::Lexeme(tiny::Token::Id, "n"),
            tiny::Lexeme(tiny::Token::Lteq),
            tiny::Lexeme(tiny::Token::LiteralNum, "1"),
            tiny::Lexeme(tiny::Token::OBraces),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::KwReturn),
            tiny::Lexeme(tiny::Token::Id, "n"),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::CBraces),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::KwReturn),
            tiny::Lexeme(tiny::Token::Id, "fibonacci"),
            tiny::Lexeme(tiny::Token::OParenthesis),
            tiny::Lexeme(tiny::Token::Id, "n"),
            tiny::Lexeme(tiny::Token::Sub),
            tiny::Lexeme(tiny::Token::LiteralNum, "1"),
            tiny::Lexeme(tiny::Token::CParenthesis),
            tiny::Lexeme(tiny::Token::Sum),
            tiny::Lexeme(tiny::Token::Id, "fibonacci"),
            tiny::Lexeme(tiny::Token::OParenthesis),
            tiny::Lexeme(tiny::Token::Id, "n"),
            tiny::Lexeme(tiny::Token::Sub),
            tiny::Lexeme(tiny::Token::LiteralNum, "2"),
            tiny::Lexeme(tiny::Token::CParenthesis),
            tiny::Lexeme(tiny::Token::NewLine),
            tiny::Lexeme(tiny::Token::CBraces),
            tiny::Lexeme(tiny::Token::NewLine),
    };

    ASSERT_EQ(lexemes, expect);
}

TEST(Lexer, Benchmark) {
    std::locale::global(std::locale("en_US.UTF8"));
    const std::int32_t benchmarkSize = 10000;

    // Generate the lexemes now, so they don't slow down the lexer when benchmarking
    std::stringstream program;
    std::vector<tiny::Lexeme> expect;

    for (std::int32_t n = 0; n < benchmarkSize; n++) {
        auto lexemePair = randomLexeme();

        program << " " + lexemePair.first;
        expect.push_back(lexemePair.second);
    }

    tiny::Lexer lexer(program);

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<tiny::Lexeme> lexemes;
    try {
        lexemes = lexer.lexAll();
    } catch (std::exception const &e) {
        std::cout << e.what() << std::endl;
        FAIL();
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    auto tokensPerSecond = std::int32_t((float(benchmarkSize) / float(duration.count())) * 1000);

    std::cout << "Lexer benchmark -> Lexed " << benchmarkSize << " random tokens in " << duration.count() << "ms ("
              << tokensPerSecond << " tokens/second)\n";

    ASSERT_EQ(lexemes, expect);
}
