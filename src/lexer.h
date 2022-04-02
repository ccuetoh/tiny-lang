#ifndef TINY_LEXER_H
#define TINY_LEXER_H

#include <iostream>
#include <ios>
#include <unordered_map>
#include <utility>
#include <map>
#include <locale>

#include "stream.h"
#include "comparator.h"
#include "metadata.h"

namespace tiny {
    //! A Token is an identifier of the semantic context-less meaning of the code fragment.
    enum class Token {
        // Zero-value
        None,

        // Identifier
        Id,

        // Keywords
        KwConst, // const
        KwImport, // import
        KwModule, // module
        KwStruct, // struct
        KwTrait, // trait
        KwFunc, //func
        KwAs, // as
        KwIn, // in
        KwIf, //if
        KwElse, // else
        KwFor, // for
        KwReturn, // return

        //  Logical operators
        KwAnd, // and
        KwOr, // or

        // Types

        // Integers
        TypeInt8, // int8
        TypeInt16, // int16
        TypeInt32, // int32, int
        TypeInt64, // int64

        TypeUInt8, // uint8
        TypeUInt16, // uint16
        TypeUInt32, // uint32, uint
        TypeUInt64, // uint64

        // Fixed-point
        TypeFixed8, // fixed8
        TypeFixed16, // fixed16
        TypeFixed32, // fixed32, fixed
        TypeFixed64, // fixed64

        TypeUFixed8, // ufixed8
        TypeUFixed16, // ufixed16
        TypeUFixed32, // ufixed32, ufixed
        TypeUFixed64, // ufixed64

        // Floating-point
        TypeFloat8, // float8
        TypeFloat16, // float16
        TypeFloat32, // float32, float
        TypeFloat64, // float64

        TypeBool, // bool
        TypeChar, // char
        TypeString, // string

        TypeList, // list
        TypeDict, // dict

        TypeAny, // any

        // Operands
        Sum, // +
        Sub, // -
        Multi, // *
        Div, // /
        Exp, // **

        // Range operands
        Range, // ..
        Step, // ->

        // Comparators
        Eq, // ==
        Neq, // !=
        Gt, // >
        Gteq, // >=
        Lt, // <
        Lteq, // <=

        // Assigns
        Assign, // =
        AssignSum, // =+
        AssignSub, // =-
        AssignDiv, // =/
        AssignMulti, // =*

        // Symbols
        Comma, // ,
        NewLine, // \n
        Init, // :=
        Negation, // !
        MemberAccess,
        Doublebang, // !!
        Dereference, // &
        ValueAt, // $

        // Parenthesis
        OParenthesis, // (
        CParenthesis, // )
        OBraces, // {
        CBraces, // }
        OBrackets, // [
        CBrackets, // ]

        // Literals
        LiteralNone, // None
        LiteralTrue, // True
        LiteralFalse, // False

        LiteralNum,
        LiteralStr,
        LiteralChar,

        // Comments
        SinglelineComment,
        MultilineComment,
    };

    /*!
     * \brief Returns an UnicodeCodepoints representation of the name of the token type
     * \param t Token to stringify
     * \return An UnicodeCodepoints representation of the name of the token type
     *
     * Returns an UnicodeCodepoints representation of the name of the token type. For example Token::TypeString will
     * become "string".
     */
    [[nodiscard]] tiny::UnicodeString getTypeName(tiny::Token t);

    //! Holds a table between keywords and their token. For example the UnicodeCodepoints for "string" maps to Token::TypeString
    static const std::map<tiny::UnicodeString, Token> KEYWORD_TABLE{
            // Keywords
            {"const",    Token::KwConst},
            {"import",   Token::KwImport},
            {"module",   Token::KwModule},
            {"struct",   Token::KwStruct},
            {"trait",    Token::KwTrait},
            {"func",     Token::KwFunc},
            {"as",       Token::KwAs},
            {"if",       Token::KwIf},
            {"in",       Token::KwIn},
            {"else",     Token::KwElse},
            {"for",      Token::KwFor},
            {"return",   Token::KwReturn},
            {"and",      Token::KwAnd},
            {"or",       Token::KwOr},

            // Types

            // Integer
            {"int",      Token::TypeInt32}, // int is an alias for int32
            {"int16",    Token::TypeInt16},
            {"int32",    Token::TypeInt32},
            {"int64",    Token::TypeInt64},

            {"uint",     Token::TypeUInt32}, // uint is an alias for uint32
            {"uint8",    Token::TypeUInt8},
            {"uint16",   Token::TypeUInt16},
            {"uint32",   Token::TypeUInt32},
            {"uint64",   Token::TypeUInt64},

            // Fixed-point
            {"fixed",    Token::TypeFixed32}, // fixed is an alias for fixed32
            {"fixed16",  Token::TypeFixed16},
            {"fixed32",  Token::TypeFixed32},
            {"fixed64",  Token::TypeFixed64},

            {"ufixed",   Token::TypeUFixed32}, // ufixed is an alias for ufixed32
            {"ufixed8",  Token::TypeUFixed8},
            {"ufixed16", Token::TypeUFixed16},
            {"ufixed32", Token::TypeUFixed32},
            {"ufixed64", Token::TypeUFixed64},

            // Floating-point
            {"float",    Token::TypeFloat32}, // float is an alias for float32
            {"float8",   Token::TypeFloat8},
            {"float16",  Token::TypeFloat16},
            {"float32",  Token::TypeFloat32},
            {"float64",  Token::TypeFloat64},

            {"bool",     Token::TypeBool},
            {"char",     Token::TypeChar},
            {"string",   Token::TypeString},

            {"list",     Token::TypeList},
            {"dict",     Token::TypeDict},

            {"any",      Token::TypeAny},

            {"None",     Token::LiteralNone},
            {"True",     Token::LiteralTrue},
            {"False",    Token::LiteralFalse},
    };

    //! A Lexeme is the product of the Lexer. It defines a token and optionally it's associated data.
    struct Lexeme {
        explicit Lexeme() = default;

        /*!
         * \brief Create a lexeme with a blank value
         * \param token Token held by the Lexeme
         */
        explicit Lexeme(Token token) : token(token) {};

        /*!
         * \brief Create a new lexeme with a blank value but with metadata
         * \param token Token held by the Lexeme
         * \param md Metadata of the Lexeme
         */
        explicit Lexeme(Token token, tiny::Metadata md) : token(token), metadata(std::move(md)) {};

        /*!
         * \brief Create a new lexeme with a value
         * \param token Token held by the Lexeme
         * \param val Value of the Lexeme
         */
        explicit Lexeme(Token token, tiny::UnicodeString val) : token(token), value(std::move(val)) {};

        /*!
          * \brief Create a new lexeme with a value and metadata
          * \param token Token held by the Lexeme
          * \param val Value of the Lexeme
          * \param md Metadata of the Lexeme
          */
        explicit Lexeme(Token token, tiny::UnicodeString val, tiny::Metadata md) : token(token), value(std::move(val)),
                                                                                metadata(std::move(md)) {};

        //! The Token of the lexeme.
        Token token = Token::None;

        //! The optional associated data of the lexeme.
        tiny::UnicodeString value;

        //! Information of the file that produced the token
        tiny::Metadata metadata;

        /*!
         * \brief Gets a string representation of the lexeme
         * \return A string with a description of the lexeme
         */
        [[nodiscard]] std::string string() const;

        /*!
         * \brief Compares the Lexeme's Token with the one provided
         * \return Whether the Lexeme's and the provided Token are equal
         */
        bool operator==(const Token &rhs) const {
            return token == rhs;
        }

        /*!
         * \brief Asserts the inequality of the Lexeme's Token with the provided Token
         * \return Whether the Lexeme's Token and the provided one are unequal
         */
        bool operator!=(const Token &rhs) const {
            return token != rhs;
        }

        /*!
         * \brief Asserts the equality of two Lexemes
         * \return Whether the Lexemes are equal
         *
         * Compares the Lexemes by asserting that both the Lexeme's Token and internal value are the same
         */
        bool operator==(const Lexeme &rhs) const {
            return token == rhs.token && value == rhs.value;
        }

        /*!
         * \brief Asserts the inequality of the Lexemes
         * \return Whether the Lexemes are unequal
         *
         * Asserts that the Lexemes are unequal by returning the negation of the == operator
         */
        bool operator!=(const Lexeme &rhs) const {
            return token != rhs.token || value != rhs.value;
        }

        /*!
         * \brief Asserts whether the Token is a valid
         * \return True if the Token is diferent from a None Token
         */
        explicit operator bool() const {
            return !isNone();
        }

        /*!
         * \brief Checks whether the token corresponds to a built-in type Token
         * \return True if the token is a built-in type token
         */
        [[nodiscard]] bool isType() const {
            return token >= Token::TypeInt8 && token <= Token::TypeAny; // Type range in the token enum
        }

        /*!
         * \brief Asserts whether the Token is a None Token.
         * \return True if the Token is a None Token
         */
        [[nodiscard]] bool isNone() const {
            return token == Token::None;
        }

        /*!
         * \brief Asserts whether the Token is a literal
         * \return True if the Token is a literal
         */
        [[nodiscard]] bool isLiteral() const {
            return token >= Token::LiteralNum && token <= Token::LiteralChar; // Type range in the token enum
        }

        /*!
         * \brief Asserts whether the Token is an operand
         * \return True if the Token is an operand
         */
        [[nodiscard]] bool isOperand() const {
            return token >= Token::Sum && token <= Token::Exp; // Type range in the token enum
        }

        /*!
         * \brief Asserts whether the Token is a comparator
         * \return True if the Token is a comparator
         */
        [[nodiscard]] bool isComparator() const {
            return token >= Token::Eq && token <= Token::Lteq; // Type range in the token enum
        }
    };

    //! The Lexer takes a stream of source-code and tokenizes it into lexemes.
    class Lexer {
    public:
        /*!
         * \brief Builds an empty Lexer
         */
        explicit Lexer() = default;

        /*!
         * \brief Builds the Lexer stream from a Stream<char>
         * \param stream The character stream to feed into the Lexer
         */
        explicit Lexer(const tiny::Stream<std::uint32_t> &stream) : s(stream) {};

        /*!
         * \brief Builds the lexer stream from an std::istream
         * \param stream The character stream to feed into the Lexer
         */
        explicit Lexer(std::istream &stream) : s(stream) { s.setTerminator(StreamTerminator); };

        /*!
         * \brief Checks whether the underlying stream has readable data
         * \return True if the stream is readable
         */
        explicit operator bool() const {
            return bool(s);
        }

        /*!
         * \brief Runs the lexer until a lexeme can be returned
         * \return A single Lexeme that might be empty
         *
         * Runs the lexer until a lexeme can be returned. In some cases a none-value Lexeme will be returned (for
         * example if there's still unlexed chars, but the remainder are just blanks). The caller should make sure the
         * returned value is valid. The stream's position will be advanced according to the lexed characters.
         *
         * If the Lexer encounters an invalid program or an unknown character the LexError exception is thrown with
         * a description of the error.
         */
        [[nodiscard]] Lexeme lex();

        /*!
         * \brief Lexes the stream until the end, and returns the tokenized Lexemes
         * \return A vector with all the tokenizable Lexemes in the stream.
         *
         * Lexes the stream until the end, and returns the tokenized Lexemes. Invalid Lexemes are discarded. The
         * stream's position will advanced to the end.
         *
         * If the Lexer encounters an invalid program or an unknown character the LexError exception is thrown with
         * a description of the error.
         */
        [[nodiscard]] std::vector<Lexeme> lexAll();

        /*!
         * \brief Basic token constructs used by the lexer and their Token
         *
         * Contains the basic token constructs used by the lexer and their Token. Not all cases are addressed, but most
         * single- and double- character token constructs are included.
         */
        const std::map<std::vector<std::uint32_t>, tiny::Token> TOKEN_TABLE{
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
                {{'.'},      Token::MemberAccess},
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
                {{'.', '.'}, Token::Range},
                {{'-', '>'}, Token::Step},
                {{'&'},      Token::Dereference},
                {{'$'},      Token::ValueAt},
        };

        /*!
         * \brief Sets the filename to be included in the metadata of the generated Lexemes
         * \param filename The file's path
         */
        void setMetadataFilename(std::string_view filename);

    private:
        //! The stream terminator used by the Lexer
        static const char StreamTerminator = '\0';

        //! The source-code stream
        tiny::Stream<std::uint32_t> s{};

        /*!
         * \brief Lexes an identifier from the stream
         * \return A Lexeme containing the identifier
         *
         * Lexes an identifier from the stream. If an invalid identifier is encountered the LexError exception is thrown
         */
        Lexeme lexId();

        /*!
         * \brief Lexes a numeric literal from the stream
         * \return A Lexeme containing the numeric literal
         *
         * Lexes a numeric literal from the stream. If an invalid numeric literal is encountered the LexError exception
         * is thrown
         */
        Lexeme lexNumericLiteral();

        /*!
         * \brief Lexes a string literal from the stream
         * \return A Lexeme containing the string literal
         *
         * Lexes a string literal from the stream. If an invalid string literal is encountered the LexError exception is
         * thrown
         */
        Lexeme lexStrLiteral();

        /*!
         * \brief Lexes a char literal from the stream
         * \return A Lexeme containing the string literal
         *
         * Lexes a char literal from the stream. If an invalid char literal is encountered the LexError exception is
         * thrown
         */
        Lexeme lexCharLiteral();

        //! Filename to be used in the metadata
        std::string filename;

        //! Current token's metadata
        [[nodiscard]] tiny::Metadata getMetadata() const;
    };
}

#endif //TINY_LEXER_H
