#include "lexer.h"
#include "errors.h"

std::vector<tiny::Lexeme> tiny::Lexer::lexAll() {
    std::vector<tiny::Lexeme> lexemes;
    while (operator bool()) {
        tiny::Lexeme lexeme = lex();
        if (lexeme.isNone()) {
            continue;
        }

        lexemes.push_back(lexeme);
    }

    return lexemes;
}

tiny::Lexeme tiny::Lexer::lex() {
    // Skip over blank chars
    std::uint32_t input;
    do {
        input = s.get();
    } while ((isblank(char(input)) || input == '\r') && s);
    // Blanks and carriage returns get discarded

    if (input == '\xff') {
        // EOF
        return tiny::Lexeme(tiny::Token::None, getMetadata());
    }

    // Newlines are common, so we check here before doing an (expensive) token-table search
    if (input == '\n') {
        return tiny::Lexeme(tiny::Token::NewLine, getMetadata());
    }

    // This all important backup makes sure that we don't just skip over the input
    s.backup();

    // "Checkpoint" this state as the start of the metadata for this token
    auto meta = getMetadata();

    // We have non-empty char. Try to match it.
    tiny::StreamComparator comparator(s);
    auto match = comparator.match(TOKEN_TABLE, Token::None);

    switch (match) {
        case Token::SinglelineComment: {
            meta.end += 2; // Account for the double-slash

            UnicodeCodepoints comment;
            for (std::uint32_t peek = s.peek(); peek != '\n' && s; peek = s.peek()) {
                comment.push_back(s.get());

            }

            meta.end = s.getIndex();
            return Lexeme(tiny::Token::SinglelineComment, tiny::UnicodeParser::toString(comment), meta);
        }
        case Token::MultilineComment: {
            meta.end += 2; // Account for the comment start

            UnicodeCodepoints comment;
            while (true) {
                if (!s) {
                    meta.end = s.getIndex();
                    throw tiny::LexError("Unclosed multiline comment", getMetadata());
                }

                std::uint32_t got = s.get();
                if (got == '*' && s.peek() == '/') {
                    s.skip();

                    meta.end = s.getIndex();
                    return Lexeme{tiny::Token::MultilineComment, tiny::UnicodeParser::toString(comment), meta};
                }

                comment.push_back(got);
            }
        }
        case Token::None:
            break; // Nothing matched should try the strategies below
        default:
            meta.end = s.getIndex();
            return Lexeme(match, meta); // Some simple token (no value) matched. No further processing needed.
    }

    // Identifier or keyword
    if (iswalpha(std::int32_t(input)) || input == '_') {
        return lexId();
    }

    // Numeric literal
    if (isdigit(char(input)) || (input == '0' && s.peek() == 'x')) {
        return lexNumericLiteral();
    }

    // String literal
    if (input == '"') {
        return lexStrLiteral();
    }

    // Char literal
    if (input == '\'') {
        return lexCharLiteral();
    }

    meta.end = s.getIndex();
    throw tiny::LexError("Unknown symbol '" + tiny::UnicodeParser::toString(input) + "'", meta);
}

tiny::Lexeme tiny::Lexer::lexId() {
    auto meta = getMetadata();

    std::uint32_t input = s.get();
    if (input == StreamTerminator) {
        meta.end = s.getIndex();
        throw tiny::LexError("End-of-file while parsing ID", meta);
    }

    UnicodeCodepoints id(1, input);

    for (auto peek = s.peek();
         iswalpha(std::int32_t(peek)) || isdigit(char(peek)) || peek == '_'; peek = s.peek()) {
        id.push_back(s.get());
    }

    auto match = KEYWORD_TABLE.find(id);
    if (match != KEYWORD_TABLE.end()) {
        return Lexeme(match->second, meta);
    }

    return Lexeme(tiny::Token::Id, tiny::UnicodeParser::toString(id), meta);
}

tiny::Lexeme tiny::Lexer::lexNumericLiteral() {
    auto meta = getMetadata();

    // The narrowing conversion is fine since all the numbers are at the very beginning of the Unicode table
    char input = char(s.get());

    if (input == StreamTerminator) {
        meta.end = s.getIndex();
        throw tiny::LexError("End-of-file while parsing numeric literal", meta);
    }

    std::string number(1, input);
    bool isHex = false;

    if (input == '0') {
        if (isdigit(char(s.peek()))) {
            // Disallow literals like 00.1 or 001 but allow 0.1 or 0
            meta.end = s.getIndex();
            throw tiny::LexError("Numeric literals can't have a leading zero", meta);
        }

        if (s.peek() == 'x') {
            // Prefixed hexadecimal literal (0xFFFF)
            number.push_back(char(s.get()));
            isHex = true;
        }
    }

    auto isContinuation = [](char c, bool hex) {
        return (isdigit(c) || c == '.') || (hex && ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')));
    };

    for (char peek = char(s.peek()); isContinuation(peek, isHex); peek = char(s.peek())) {
        auto got = s.get();
        if (got == '.' && s.peek() == '.') {
            // We encountered a range (..) operator. Revert and return.

            s.backup();
            break;
        }

        number.push_back(char(got));
    }

    // Make sure we reject malformed decimal numbers like 3.12.14
    if (std::count(number.begin(), number.end(), '.') > 1) {
        meta.end = s.getIndex();
        throw tiny::LexError("Numeric literal has two decimal points", meta);
    }

    return Lexeme(tiny::Token::LiteralNum, number, meta);
}

tiny::Lexeme tiny::Lexer::lexStrLiteral() {
    auto meta = getMetadata();

    if (!s) {
        meta.end = s.getIndex();
        throw tiny::LexError("End-of-file while parsing string literal", meta);
    }

    s.skip(); // StageStep-over the first "

    tiny::UnicodeCodepoints str;
    for (std::uint32_t peek = s.peek(); peek != '"'; peek = s.peek()) {
        if (peek == StreamTerminator) {
            meta.end = s.getIndex();
            throw tiny::LexError("End-of-file while parsing string literal", meta);
        }

        str.push_back(s.get());
    }

    s.skip(); // StageStep-over the second "

    return Lexeme(tiny::Token::LiteralStr, tiny::UnicodeParser::toString(str), meta);
}

tiny::Lexeme tiny::Lexer::lexCharLiteral() {
    auto meta = getMetadata();

    if (!s) {
        meta.end = s.getIndex();
        throw tiny::LexError("End-of-file while parsing char literal", meta);
    }

    s.skip(); // StageStep-over the first '

    std::uint32_t next = s.get();
    if (s.get() != '\'') {
        meta.end = s.getIndex();
        throw tiny::LexError("Invalid char definition", meta);
    }

    return Lexeme(tiny::Token::LiteralChar, tiny::UnicodeParser::toString(next), meta);
}

void tiny::Lexer::setMetadataFilename(std::string_view fn) {
    filename = fn;
}

tiny::Metadata tiny::Lexer::getMetadata() const {
    return tiny::Metadata(filename, s.getIndex(), s.getIndex() + 1);
}

std::string tiny::Lexeme::string() const {
    switch (token) {
        // Zero-value
        case Token::None:
            return "<None, " + value + ">";

            // Identifier
        case Token::Id:
            return "<Identifier, " + value + ">";

            // Keywords
        case Token::KwModule:
            return "<Keyword Module>";
        case Token::KwConst:
            return "<Keyword Const>";
        case Token::KwImport:
            return "<Keyword Import>";
        case Token::KwStruct:
            return "<Keyword Struct>";
        case Token::KwTrait:
            return "<Keyword Trait>";
        case Token::KwFunc:
            return "<Keyword Func>";
        case Token::KwAs:
            return "<Keyword As>";
        case Token::KwIf:
            return "<Keyword If>";
        case Token::KwFor:
            return "<Keyword For>";
        case Token::KwIn:
            return "<Keyword In>";
        case Token::KwReturn:
            return "<Keyword Return>";

            // Logical operators
        case Token::KwAnd:
            return "<Keyword And>";
        case Token::KwOr:
            return "<Keyword Or>";

            // Types

        case Token::TypeAny:
            return "<Type any>";

            // Integers
        case Token::TypeFixed8:
            return "<Type fixed8>";
        case Token::TypeFixed16:
            return "<Type fixed16>";
        case Token::TypeFixed32:
            return "<Type fixed32>";
        case Token::TypeFixed64:
            return "<Type fixed64>";

            // Fixed-point
        case Token::TypeInt8:
            return "<Type int8>";
        case Token::TypeInt16:
            return "<Type int16>";
        case Token::TypeInt32:
            return "<Type int32>";
        case Token::TypeInt64:
            return "<Type int64>";

            // Floating-point
        case Token::TypeFloat8:
            return "<Type float8>";
        case Token::TypeFloat16:
            return "<Type float16>";
        case Token::TypeFloat32:
            return "<Type float32>";
        case Token::TypeFloat64:
            return "<Type float64>";

        case Token::TypeBool:
            return "<Type bool>";
        case Token::TypeChar:
            return "<Type char>";
        case Token::TypeString:
            return "<Type string>";

        case Token::TypeList:
            return "<Type list>";
        case Token::TypeDict:
            return "<Type dict>";

        case Token::LiteralNone:
            return "<None Literal>";
        case Token::LiteralTrue:
            return "<True Literal>";
        case Token::LiteralFalse:
            return "<False Literal>";

            // Operands
        case Token::Sum:
            return "<Sum>";
        case Token::Sub:
            return "<Subtraction>";
        case Token::Multi:
            return "<Multiplication>";
        case Token::Div:
            return "<Division>";

        case Token::Range:
            return "<Range>";
        case Token::Step:
            return "<StageStep>";

            // Comparators
        case Token::Eq:
            return "<Equality>";
        case Token::Neq:
            return "<Inequality>";
        case Token::Gt:
            return "<Greater-than>";
        case Token::Gteq:
            return "<Greater-than-equals>";
        case Token::Lt:
            return "<Less-than>";
        case Token::Lteq:
            return "<Less-than-equals>";

            // Assigns
        case Token::Assign:
            return "<Assignment>";
        case Token::AssignSum:
            return "<Assignment-sum>";
        case Token::AssignSub:
            return "<Assignment-subtraction>";
        case Token::AssignDiv:
            return "<Assignment-division>";
        case Token::AssignMulti:
            return "<Assignment-multiplication>";

            // Symbols
        case Token::Comma:
            return "<Comma>";
        case Token::NewLine:
            return "<New Line>";
        case Token::Init:
            return "<Initialize>";
        case Token::MemberAccess:
            return "<MemberA Access>";
        case Token::Doublebang:
            return "<Doublebang>";

            // Parenthesis
        case Token::OParenthesis:
            return "<Open Parenthesis>";
        case Token::CParenthesis:
            return "<Close Parenthesis>";
        case Token::OBraces:
            return "<Open Braces>";
        case Token::CBraces:
            return "<Close Braces>";
        case Token::OBrackets:
            return "<Open Brackets>";
        case Token::CBrackets:
            return "<Close Brackets>";

            // Literals
        case Token::LiteralNum:
            return "<Numeric Literal, " + value + ">";
        case Token::LiteralStr:
            return "<String Literal, " + value + ">";
        case Token::Negation:
            return "<Negation>";

            // Memory
        case Token::Dereference:
            return "<Dereference>";
        case Token::ValueAt:
            return "<ValueAt>";

            // Comments
        case Token::SinglelineComment:
            return "<Singleline Comment, \"" + value + "\">";
        case Token::MultilineComment:
            return "<Multiline Comment, \"" + value + "\">";

        default:
            return "<Unknown>";
    }
}

tiny::UnicodeCodepoints tiny::getTypeName(tiny::Token t) {
    switch (t) {
        case tiny::Token::TypeAny:
            return tiny::UnicodeParser::fromString("any");

            // Integers
        case tiny::Token::TypeFixed8:
            return tiny::UnicodeParser::fromString("fixed8");
        case tiny::Token::TypeFixed16:
            return tiny::UnicodeParser::fromString("fixed16");
        case tiny::Token::TypeFixed32:
            return tiny::UnicodeParser::fromString("fixed32");
        case tiny::Token::TypeFixed64:
            return tiny::UnicodeParser::fromString("fixed64");

            // Fixed-point
        case tiny::Token::TypeInt8:
            return tiny::UnicodeParser::fromString("int8");
        case tiny::Token::TypeInt16:
            return tiny::UnicodeParser::fromString("int16");
        case tiny::Token::TypeInt32:
            return tiny::UnicodeParser::fromString("int32");
        case tiny::Token::TypeInt64:
            return tiny::UnicodeParser::fromString("int64");

            // Floating-point
        case tiny::Token::TypeFloat8:
            return tiny::UnicodeParser::fromString("float8");
        case tiny::Token::TypeFloat16:
            return tiny::UnicodeParser::fromString("float16");
        case tiny::Token::TypeFloat32:
            return tiny::UnicodeParser::fromString("float32");
        case tiny::Token::TypeFloat64:
            return tiny::UnicodeParser::fromString("float64");

        case tiny::Token::TypeBool:
            return tiny::UnicodeParser::fromString("bool");
        case tiny::Token::TypeChar:
            return tiny::UnicodeParser::fromString("char");
        case tiny::Token::TypeString:
            return tiny::UnicodeParser::fromString("string");

        case tiny::Token::TypeList:
            return tiny::UnicodeParser::fromString("list");
        case tiny::Token::TypeDict:
            return tiny::UnicodeParser::fromString("dict");

        default:
            return {};
    }
}
