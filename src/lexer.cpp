#include "lexer.h"

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
    uint32_t input;
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


            UnicodeCodepoints comment;
            for (uint32_t peek = s.peek(); peek != '\n' && s; peek = s.peek()) {
                comment.push_back(s.get());
            }

            return Lexeme(tiny::Token::SinglelineComment, tiny::UnicodeParser::ToString(comment), meta);
        }
        case Token::MultilineComment: {
            auto startMetadata = getMetadata();

            UnicodeCodepoints comment;
            while (true) {
                if (!s) {
                    throw tiny::LexError("Unclosed multiline comment", getMetadata());
                }

                uint32_t got = s.get();
                if (got == '*' && s.peek() == '/') {
                    s.skip();
                    return Lexeme{tiny::Token::MultilineComment, tiny::UnicodeParser::ToString(comment), meta};
                }

                comment.push_back(got);
            }
        }
        case Token::None:
            break; // Nothing matched should try the strategies below
        default:
            return Lexeme(match, meta); // Some simple token (no value) matched. No further processing needed.
    }

    // Identifier or keyword
    if (std::isalpha(wchar_t(input), locale) || input == '_') {
        return lexId();
    }

    // Numeric literal
    if (isdigit(char(input))) {
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

    throw tiny::LexError("Unknown symbol '" + tiny::UnicodeParser::ToString(input) + "'", meta);
}

tiny::Lexeme tiny::Lexer::lexId() {
    auto meta = getMetadata();

    uint32_t input = s.get();
    if (input == StreamTerminator) {
        throw tiny::LexError("End-of-file while parsing ID", meta);
    }

    UnicodeCodepoints id(1, input);

    for (input = s.get();
         std::isalpha(wchar_t(input), locale) || isdigit(char(input)) || input == '_'; input = s.get()) {
        id.push_back(input);
    }

    // If we got here then we "overshot" the id by one char. Backup. We don't backup if we reached the EOF.
    if (s) {
        s.backup();
    }

    auto match = KEYWORD_TABLE.find(id);
    if (match != KEYWORD_TABLE.end()) {
        return Lexeme(match->second);
    }

    return Lexeme(tiny::Token::Id, tiny::UnicodeParser::ToString(id), meta);
}

tiny::Lexeme tiny::Lexer::lexNumericLiteral() {
    auto meta = getMetadata();

    // The narrowing conversion is fine since all the numbers are at the very beginning of the Unicode table
    char input = char(s.get());

    if (input == StreamTerminator) {
        throw tiny::LexError("End-of-file while parsing numeric literal", meta);
    }

    if (input == '0' && isdigit(char(s.peek()))) {
        // Disallow literals like 00.1 or 001 but allow 0.1 or 0
        throw tiny::LexError("Numeric literals can't start with zero", meta);
    }

    std::string number(1, input);

    bool isDecimal = false;
    for (char peek = char(s.peek()); isdigit(peek) || peek == '.'; peek = char(s.peek())) {
        if (peek == '.') {
            if (isDecimal) {
                throw tiny::LexError("Numeric literal has two decimal points", meta);
            }

            isDecimal = true;
        }

        number.push_back(char(s.get()));
    }

    return Lexeme(tiny::Token::LiteralNum, number, meta);
}

tiny::Lexeme tiny::Lexer::lexStrLiteral() {
    auto meta = getMetadata();

    if (!s) {
        throw tiny::LexError("End-of-file while parsing string literal", meta);
    }

    s.skip(); // Step-over the first "

    tiny::UnicodeCodepoints str;
    for (uint32_t peek = s.peek(); peek != '"'; peek = s.peek()) {
        if (peek == StreamTerminator) {
            throw tiny::LexError("End-of-file while parsing string literal", meta);
        }

        str.push_back(s.get());
    }

    s.skip(); // Step-over the second "

    return Lexeme(tiny::Token::LiteralStr, tiny::UnicodeParser::ToString(str), meta);
}

tiny::Lexeme tiny::Lexer::lexCharLiteral() {
    auto meta = getMetadata();

    if (!s) {
        throw tiny::LexError("End-of-file while parsing char literal", meta);
    }

    s.skip(); // Step-over the first '

    uint32_t next = s.get();
    if (s.get() != '\'') {
        throw tiny::LexError("Invalid char definition", meta);
    }

    return Lexeme(tiny::Token::LiteralChar, tiny::UnicodeParser::ToString(next), meta);
}

void tiny::Lexer::setMetadataFilename(std::string_view fn) {
    filename = fn;
}

tiny::Metadata tiny::Lexer::getMetadata() {
    return tiny::Metadata(filename, s.getIndex());
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
        case Token::KwFunc:
            return "<Keyword Func>";
        case Token::KwAs:
            return "<Keyword As>";
        case Token::KwIs:
            return "<Keyword Is>";
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

            // Integers
        case Token::TypeFixed8:
            return "<Type fixed8>";
        case Token::TypeUFixed8:
            return "<Type ufixed8>";
        case Token::TypeFixed16:
            return "<Type fixed16>";
        case Token::TypeUFixed16:
            return "<Type ufixed16>";
        case Token::TypeFixed32:
            return "<Type fixed32>";
        case Token::TypeUFixed32:
            return "<Type ufixed32>";
        case Token::TypeFixed64:
            return "<Type fixed64>";
        case Token::TypeUFixed64:
            return "<Type ufixed64>";

            // Fixed-point
        case Token::TypeInt8:
            return "<Type int8>";
        case Token::TypeUInt8:
            return "<Type uint8>";
        case Token::TypeInt16:
            return "<Type int16>";
        case Token::TypeUInt16:
            return "<Type uint16>";
        case Token::TypeInt32:
            return "<Type int32>";
        case Token::TypeUInt32:
            return "<Type uint32>";
        case Token::TypeInt64:
            return "<Type int64>";
        case Token::TypeUInt64:
            return "<Type uint64>";

            // Floating-point
        case Token::TypeFloat8:
            return "<Type float8>";
        case Token::TypeUFloat8:
            return "<Type ufloat8>";
        case Token::TypeFloat16:
            return "<Type float16>";
        case Token::TypeUFloat16:
            return "<Type ufloat16>";
        case Token::TypeFloat32:
            return "<Type float32>";
        case Token::TypeUFloat32:
            return "<Type ufloat32>";
        case Token::TypeFloat64:
            return "<Type float64>";
        case Token::TypeUFloat64:
            return "<Type ufloat64>";

        case Token::TypeBool:
            return "<Type bool>";
        case Token::TypeChar:
            return "<Type char>";
        case Token::TypeString:
            return "<Type string>";

        case Token::LiteralNone:
            return "<None Literal>";
        case Token::LiteralTrue:
            return "<True Literal>";
        case Token::LiteralFalse:
            return "<False Literal>";

        case Token::TypeCustom:
            return "<Type custom>";

            // Operands
        case Token::Sum:
            return "<Sum>";
        case Token::Sub:
            return "<Subtraction>";
        case Token::Multi:
            return "<Multiplication>";
        case Token::Div:
            return "<Division>";

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
        case Token::Colon:
            return "<Colon>";
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

            // Comments
        case Token::SinglelineComment:
            return "<Singleline Comment, \"" + value + "\">";
        case Token::MultilineComment:
            return "<Multiline Comment, \"" + value + "\">";

        default:
            return "<Unknown>";
    }
}
