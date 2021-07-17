#ifndef TINY_LEXER_H
#define TINY_LEXER_H

#include <iostream>
#include <ios>
#include <unordered_map>
#include <utility>
#include <map>
#include <locale>

#include "stream.h"
#include "errors.h"
#include "comparator.h"
#include "metadata.h"

namespace tiny {
    //! A Token is an identifier of the semantic context-less meaning of the code fragment.
    enum struct Token {
        // Zero-value
        None,

        // Identifier
        Id,

        // Keywords
        KwConst,
        KwImport,
        KwModule,
        KwStruct,
        KwFunc,
        KwAs, // Import alias
        KwIs, // Type alias TODO Reconsider Kw
        KwIf,
        KwFor,
        KwIn, // Iterator
        KwReturn,

        //  Logical operators
        KwAnd,
        KwOr,

        // Types

        // Integers
        TypeInt8,
        TypeUInt8,
        TypeInt16,
        TypeUInt16,
        TypeInt32,
        TypeUInt32,
        TypeInt64,
        TypeUInt64,

        // Fixed-point
        TypeFixed8,
        TypeUFixed8,
        TypeFixed16,
        TypeUFixed16,
        TypeFixed32,
        TypeUFixed32,
        TypeFixed64,
        TypeUFixed64,

        // Floating-point
        TypeFloat8,
        TypeUFloat8,
        TypeFloat16,
        TypeUFloat16,
        TypeFloat32,
        TypeUFloat32,
        TypeFloat64,
        TypeUFloat64,

        TypeBool,
        TypeChar,
        TypeString,

        TypeAny,

        TypeCustom,

        // Operands
        Sum,
        Sub,
        Multi,
        Div,
        Exp,

        // Comparators
        Eq,
        Neq,
        Gt,
        Gteq,
        Lt,
        Lteq,

        // Assigns
        Assign,
        AssignSum,
        AssignSub,
        AssignDiv,
        AssignMulti,

        // Symbols
        Comma,
        NewLine,
        Init,
        Negation,
        Colon,
        Doublebang,

        // Parenthesis
        OParenthesis,
        CParenthesis,
        OBraces,
        CBraces,
        OBrackets,
        CBrackets,

        // Literals
        LiteralNone,
        LiteralTrue,
        LiteralFalse,

        LiteralNum,
        LiteralStr,
        LiteralChar,

        // Comments
        SinglelineComment,
        MultilineComment,
    };

    static const std::map<tiny::UnicodeCodepoints, Token> KEYWORD_TABLE{
            // Keywords
            {tiny::UnicodeParser::FromString("const"),    Token::KwConst},
            {tiny::UnicodeParser::FromString("import"),   Token::KwImport},
            {tiny::UnicodeParser::FromString("module"),   Token::KwModule},
            {tiny::UnicodeParser::FromString("struct"),   Token::KwStruct},
            {tiny::UnicodeParser::FromString("func"),     Token::KwFunc},
            {tiny::UnicodeParser::FromString("as"),       Token::KwAs},
            {tiny::UnicodeParser::FromString("is"),       Token::KwIs},
            {tiny::UnicodeParser::FromString("if"),       Token::KwIf},
            {tiny::UnicodeParser::FromString("for"),      Token::KwFor},
            {tiny::UnicodeParser::FromString("in"),       Token::KwIn},
            {tiny::UnicodeParser::FromString("return"),   Token::KwReturn},
            {tiny::UnicodeParser::FromString("and"),      Token::KwAnd},
            {tiny::UnicodeParser::FromString("or"),       Token::KwOr},

            // Types

            // Integer
            {tiny::UnicodeParser::FromString("int"),      Token::TypeInt32}, // int is an alias for int32
            {tiny::UnicodeParser::FromString("uint"),     Token::TypeUInt32}, // uint is an alias for uint32
            {tiny::UnicodeParser::FromString("uint8"),    Token::TypeUInt8},
            {tiny::UnicodeParser::FromString("int16"),    Token::TypeInt16},
            {tiny::UnicodeParser::FromString("uint16"),   Token::TypeUInt16},
            {tiny::UnicodeParser::FromString("int32"),    Token::TypeInt32},
            {tiny::UnicodeParser::FromString("uint32"),   Token::TypeUInt32},
            {tiny::UnicodeParser::FromString("int64"),    Token::TypeInt64},
            {tiny::UnicodeParser::FromString("uint64"),   Token::TypeUInt64},

            // Fixed-point
            {tiny::UnicodeParser::FromString("fixed"),    Token::TypeFixed32}, // fixed is an alias for fixed32
            {tiny::UnicodeParser::FromString("ufixed"),   Token::TypeUFixed32}, // ufixed is an alias for ufixed32
            {tiny::UnicodeParser::FromString("ufixed8"),  Token::TypeUFixed8},
            {tiny::UnicodeParser::FromString("fixed16"),  Token::TypeFixed16},
            {tiny::UnicodeParser::FromString("ufixed16"), Token::TypeUFixed16},
            {tiny::UnicodeParser::FromString("fixed32"),  Token::TypeFixed32},
            {tiny::UnicodeParser::FromString("ufixed32"), Token::TypeUFixed32},
            {tiny::UnicodeParser::FromString("fixed64"),  Token::TypeFixed64},
            {tiny::UnicodeParser::FromString("ufixed64"), Token::TypeUFixed64},

            // Floating-point
            {tiny::UnicodeParser::FromString("float"),    Token::TypeFloat32}, // float is an alias for float32
            {tiny::UnicodeParser::FromString("ufloat"),   Token::TypeUFloat32}, // ufloat is an alias for ufloat32
            {tiny::UnicodeParser::FromString("float8"),   Token::TypeFloat8},
            {tiny::UnicodeParser::FromString("ufloat8"),  Token::TypeUFloat8},
            {tiny::UnicodeParser::FromString("float16"),  Token::TypeFloat16},
            {tiny::UnicodeParser::FromString("ufloat16"), Token::TypeUFloat16},
            {tiny::UnicodeParser::FromString("float32"),  Token::TypeFloat32},
            {tiny::UnicodeParser::FromString("ufloat32"), Token::TypeUFloat32},
            {tiny::UnicodeParser::FromString("float64"),  Token::TypeFloat64},
            {tiny::UnicodeParser::FromString("ufloat64"), Token::TypeUFloat64},

            {tiny::UnicodeParser::FromString("bool"),     Token::TypeBool},
            {tiny::UnicodeParser::FromString("char"),     Token::TypeChar},
            {tiny::UnicodeParser::FromString("string"),   Token::TypeString},

            {tiny::UnicodeParser::FromString("any"),      Token::TypeAny},

            {tiny::UnicodeParser::FromString("None"),     Token::LiteralNone},
            {tiny::UnicodeParser::FromString("True"),     Token::LiteralTrue},
            {tiny::UnicodeParser::FromString("False"),    Token::LiteralFalse},
    };

    //! A Lexeme is the product of the Lexer. It defines a token and optionally it's associated data.
    struct Lexeme {
        //! Create a lexeme with a blank value.
        explicit Lexeme(Token token) : token(token) {};

        //! Create a new lexeme with a blank value but with metadata.
        explicit Lexeme(Token token, tiny::Metadata md) : token(token), metadata(std::move(md)) {};

        //! Create a new lexeme with a value.
        explicit Lexeme(Token token, std::string_view val) : token(token), value(val) {};

        //! Create a new lexeme with a value and metadata.
        explicit Lexeme(Token token, std::string_view val, tiny::Metadata md) : token(token), value(val),
                                                                                metadata(std::move(md)) {};

        //! The Token of the lexeme.
        Token token = Token::None;

        //! The optional associated data of the lexeme.
        std::string value;

        //! Information of the file that produced the token
        tiny::Metadata metadata;

        //! Returns a string representation of the lexeme.
        [[nodiscard]] std::string string() const;

        //! Comparison of a lexeme's internal token with another token.
        bool operator==(const Token &rhs) const {
            return token == rhs;
        }

        //! Compares the token and value of a Lexeme to another one and returns true if equal.
        bool operator==(const Lexeme &rhs) const {
            return token == rhs.token && value == rhs.value;
        }

        //! Compares the token and value of a Lexeme to another one and returns true if different.
        bool operator!=(const Lexeme &rhs) const {
            return token != rhs.token || value != rhs.value;
        }

        //! Comparison of a lexeme's internal token with another token.
        bool operator!=(const Token &rhs) const {
            return token != rhs;
        }

        //! Check whether the internal token is a NoOp token.
        explicit operator bool() const {
            return isNone();
        }

        //! Returns whether the token corresponds to a built-in type token.
        [[nodiscard]] bool isBuiltinType() const {
            return token >= Token::TypeInt8 && token <= Token::TypeString; // Type range in the token enum
        }

        //! Returns whether the token corresponds to a type token.
        [[nodiscard]] bool isType() const {
            return token >= Token::TypeInt8 && token <= Token::TypeCustom; // Type range in the token enum
        }

        //! Returns whether the token is a NoOp token.
        [[nodiscard]] bool isNone() const {
            return token == Token::None;
        }

        //! Returns whether the token corresponds to literal.
        [[nodiscard]] bool isLiteral() const {
            return token >= Token::LiteralNum && token <= Token::LiteralChar; // Type range in the token enum
        }

        //! Returns whether the token corresponds to an operand.
        [[nodiscard]] bool isOperand() const {
            return token >= Token::Sum && token <= Token::Exp; // Type range in the token enum
        }

        //! Returns whether the token corresponds to a comparator operator.
        [[nodiscard]] bool isComparator() const {
            return token >= Token::Eq && token <= Token::Lteq; // Type range in the token enum
        }
    };

    //! The Lexer takes a stream of source-code and tokenizes it into lexemes.
    class Lexer {
    public:
        //! Builds the lexer stream from a WalkableStream<char>
        explicit Lexer(tiny::WalkableStream<uint32_t> &stream) : s(stream) {};

        //! Builds the lexer stream from an istream.
        explicit Lexer(std::istream &stream) : s(stream, StreamTerminator) {};

        //! Checks whether the underlying stream has readable data.
        explicit operator bool() {
            return bool(s);
        }

        //! Runs the lexer until a lexeme can be returned. In some cases a none-value Lexeme will be returned. For
        //! example if there's still unlexed chars, but the remainder are just blanks and an EOF is reached. The caller
        //! should check with the isNone() method.
        [[nodiscard]] Lexeme lex();

        //! Lexes the stream until the end and returns the found Lexemes. None-value Lexemes are discarded.
        [[nodiscard]] std::vector<Lexeme> lexAll();

        //! Contains the basic token constructs used by the lexer. Not all cases are addressed, but most single and
        //! double token constructs will be matched.
        const std::map<std::vector<uint32_t>, tiny::Token> TOKEN_TABLE{
                {{','},      Token::Comma},
                {{'+', '='}, Token::AssignSum},
                {{'+'},      Token::Sum},
                {{'-', '='}, Token::AssignSub},
                {{'-'},      Token::Sub},
                {{'*', '='}, Token::AssignMulti},
                {{'*', '*'}, Token::Exp},
                {{'*'},      Token::Multi},
                {{'('},      Token::OParenthesis},
                {{')'},      Token::CParenthesis},
                {{'{'},      Token::OBraces},
                {{'}'},      Token::CBraces},
                {{'['},      Token::OBrackets},
                {{']'},      Token::CBrackets},
                {{':', '='}, Token::Init},
                {{':'},      Token::Colon},
                {{'>', '='}, Token::Gteq},
                {{'>'},      Token::Gt},
                {{'<', '='}, Token::Lteq},
                {{'<'},      Token::Lt},
                {{'=', '='}, Token::Eq},
                {{'='},      Token::Assign},
                {{'/', '/'}, Token::SinglelineComment},
                {{'/', '*'}, Token::MultilineComment},
                {{'/', '='}, Token::AssignDiv},
                {{'/'},      Token::Div},
                {{'!', '!'}, Token::Doublebang},
                {{'!', '='}, Token::Neq},
                {{'!'},      Token::Negation},
        };

        //! locale is used for interpreting the tokens in UTF-8. It defaults to en_US.UTF8
        const std::locale locale = std::locale("en_US.UTF8");

        // Metadata
        //! Sets the metadata's filename
        void setMetadataFilename(std::string_view filename);

    private:
        //! The stream terminator used by the Lexer.
        static const char StreamTerminator = '\0';

        //! The source-code stream
        tiny::WalkableStream<uint32_t> s;

        //! Lexes an identifier from the stream. (funcName)
        Lexeme lexId();

        //! Lexes a numeric literal from the stream. (1)
        Lexeme lexNumericLiteral();

        //! Lexes a string literal from the stream ("str")
        Lexeme lexStrLiteral();

        //! Lexes a char literal from the stream ('c')
        Lexeme lexCharLiteral();

        // Metadata
        //! Filename to be used in the metadata
        std::string filename;

        //! Current token's metadata
        tiny::Metadata getMetadata();
    };
}

#endif //TINY_LEXER_H
