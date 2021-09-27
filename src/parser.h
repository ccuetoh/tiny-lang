#ifndef TINY_PARSER_H
#define TINY_PARSER_H


#include "stream.h"
#include "ast.h"
#include "lexer.h"

namespace tiny {
    /*!
     * \brief Parser takes a stream of Lexemes and sequentially resolves them into an AST via recursive decent
     *
     * Parser takes a stream of Lexemes (as a WalkableStream) and sequentially resolves them into an Abstract
     * Syntax Tree using the recursive decent method.
     */
    class Parser {
    public:
        /*!
         * \brief Constructor from a WalkableStream
         * \param stream WalkableStream of Lexemes that represent a Tiny file
         */
        explicit Parser(tiny::WalkableStream<tiny::Lexeme> &stream) : s(stream) {};

        /*!
         * \brief Parses a complete file of source code
         * \param filename The path of the file for metadata
         * \return An ASTFile containing the Abstract Syntax Tree and data about the program
         */
        [[nodiscard]] tiny::ASTFile file(std::string_view filename = "");

    private:
        /*!
         * \brief Fetches the next Lexeme in the stream and advances the stream's position by one, then compares it
         * \param token The expected Token
         * \return If the Token matches the Lexeme is returned
         *
         * Fetches the next Lexeme in the stream and advances the stream's position by one, then checks whether the
         * Lexeme's Token is the matches the given Token's type. If they're mismatched a ParseError exception is thrown.
         * Otherwise the Lexeme is returned.
         */
        tiny::Lexeme consume(const tiny::Token &token);

        /*!
         * \brief Fetches the next Lexeme in the stream, compares it and advances the stream if they match
         * \param token The expected Token
         * \return True if the Token was matched and the stream advanced, false otherwise
         *
         * Fetches the next Lexeme in the stream and compares it against the provided Token. If they match the stream
         * is advanced by one and true is returned. If the Token is not matched the stream is not advanced and false is
         * return.
         */
        bool consumeOptional(const tiny::Token &token);

        /*!
         * \brief Consumes all the contiguous Lexemes with the provided Token until a non-matching Lexeme is found
         * \param token The Token to consume
         *
         * Consumes all the contiguous Lexemes with the provided Token until a non-matching Lexeme is found. It works
         * akin to a left-trim over the stream.
         */
        void exhaust(Token token);

        /*!
         * \brief Exhausts over the set of Token given
         * \param tokens The Tokens to consume
         */
        void exhaust(const std::vector<tiny::Token>& tokens);

        /*!
         * \brief Gets the next Token (without advancing the steam) and returns whether it matches the provided one
         * \param token The Token to check
         * \return True if the next Token in the stream matches, otherwise false
         *
         * Fetches the next Lexeme in the stream and compares it to the given token. If they're equal then true is
         * returned. Check doesn't advances the stream.
         */
        bool check(tiny::Token token);

        /*!
         * \brief Expects a module name in the stream, consume the corresponding tokens and returns the module name
         * \return The name of the module
         */
        [[nodiscard]] std::string moduleStatement();

        /*!
         * \brief Consumes an import statement from the stream
         * \return A vector of the parsed Imports
         */
        [[nodiscard]] std::vector<tiny::Import> importStatement();

        /*!
         * \brief Uses recursive decent to resolve a sequence of statements of a Tiny program
         * \return A vector of nodes (as an AST)
         *
         * Entrypoint of the main parser. Uses recursive decent to resolve a sequence of statements of a Tiny program,
         * and returns a vector of nodes (as an AST). If the program is invalid ParseError is thrown.
         */
        [[nodiscard]] tiny::StatementList statementList(tiny::Token stopToken=tiny::Token::None);

        /*!
         * \brief Consumes a single statement from the stream
         * \return An ASTNode of any of the downstream types
         */
        [[nodiscard]] tiny::ASTNode statement(std::vector<tiny::Token> terminators={tiny::Token::NewLine});

        /*!
         * \brief Consumes a single expression followed by a terminator
         * \param terminators Optional list of terminators as a vector. Defaults to only NewLine
         * \return An ASTNode of type ExpressionStatement or any of the downstream types
         */
        [[nodiscard]] tiny::ASTNode expressionStatement(std::vector<tiny::Token> terminators={tiny::Token::NewLine});

        /*!
         * \brief Consumes a block statement followed by an error handler from the stream
         * \return An ASTNode of type BlockStatement
         */
        [[nodiscard]] tiny::ASTNode errorHandledBlockStatement();

        /*!
         * \brief Consumes a statement encapsulated as a block from the stream
         * \return An ASTNode of type BlockStatement
         */
        [[nodiscard]] tiny::ASTNode blockStatement();

        /*!
         * \brief Consumes an if-branch definition from the stream
         * \return An ASTNode of type IfStatement
         */
        [[nodiscard]] tiny::ASTNode ifStatement();

        /*!
         * \brief Consumes a for-loop definition from the stream
         * \return An ASTNode of type ForStatement
         */
        [[nodiscard]] tiny::ASTNode forStatement();

        /*!
         * \brief Consumes an function or method declaration from the stream
         * \return An ASTNode of type FunctionDeclaration or MethodDeclaration
         */
        [[nodiscard]] tiny::ASTNode funcDeclStatement(bool hasBody=true);

        /*!
         * \brief Consumes a list of arguments as given inside a function or method declaration
         * \return An ASTNode of type FunctionArgumentDeclList with children of type FunctionArgumentDecl
         */
        [[nodiscard]] tiny::ASTNode argumentDeclList();

        /*!
         * \brief Consumes list of returns as given inside a function or method declaration
         * \return An ASTNode of type FunctionReturnDeclList with children of type FunctionReturnDec
         */
        [[nodiscard]] tiny::ASTNode returnDeclList();

        /*!
         * \brief Consumes a return statement (that might contain multiple return-expression) from the stream
         * \return An ASTNode of type FunctionReturn with one children for each return-expression
         */
        [[nodiscard]] tiny::ASTNode returnStatement();

        /*!
         * \brief Consumes a list of comma-separated expressions
         * \param terminators Optional list of terminators as a vector. Defaults to only NewLine
         * \return An ASTNode of type ExpressionList with a children for each expression
         */
        [[nodiscard]] tiny::ASTNode commaSeparatedExpressionList(const std::vector<tiny::Token>& terminators={tiny::Token::NewLine});

        /*!
         * \brief Consumes a struct definition from the stream
         * \return An ASTNode of type StructDeclaration with a StructFieldList and a TraitList children, and a Name parameter
         */
        [[nodiscard]] tiny::ASTNode structStatement();

        /*!
         * \brief Consumes a trait definition from the stream
         * \return An ASTNode of type TraitDeclaration with a TraitList and a TraitFieldList children, and a Name parameter
         */
        [[nodiscard]] tiny::ASTNode traitStatement();

        /*!
         * \brief Consumes a list of trait bounds in a struct or a function
         * \return An ASTNode of type TraitList with a child node of type Trait for each trait
         */
        [[nodiscard]] tiny::ASTNode traitListStatement();

        /*!
         * \brief Consumes a list of struct fields
         * \return An ASTNode of type StructFieldList that might contain either Composition or StructField children
         */
        [[nodiscard]] tiny::ASTNode structFieldList();

        /*!
         * \brief Consumes a list of trait fields
         * \return An ASTNode of type TraitFieldList that might contain either FunctionDeclaration or any of the typed-expression types children
         */
        [[nodiscard]] tiny::ASTNode traitFieldList();

        /*!
         * \brief Consumes a range expression inside a for-loop definition
         * \return An ASTNode of type RangeExpression with a RangeIdentifier, RangeFromExpression, RangeToExpression and RangeStep child
         */
        [[nodiscard]] tiny::ASTNode rangeExpression();

        /*!
         * \brief Consumes a for-each iterator expression inside a for-loop definition
         * \return An ASTNode of type ForEachExpression with a RangeIdentifier parameter and an additive expression or downstream node
         */
        [[nodiscard]] tiny::ASTNode forEachExpression();

        /*!
         * \brief Base expression handler
         * \return An ASTNode with any of the expression types
         */
        [[nodiscard]] tiny::ASTNode expression();

        /*!
         * \brief Consumes the invocation of an error handler for an expression or block
         * \return An ASTNode of type ErrorHandle and any of the assignment expression or downstream nodes
         */
        [[nodiscard]] tiny::ASTNode errorHandleExpression();

        /*!
         * \brief Consumes an assignment expression or derivatives to the downstream parsers
         * \return An ASTNode of type Assignment, AssignmentSum, AssignmentSub, AssignmentMulti, AssignmentDiv, Init or any of the downstream nodes
         */
        [[nodiscard]] tiny::ASTNode assignmentExpression();

        /*!
         * \brief Consumes an expression built from a logical comparison or derivatives to the downstream parsers
         * \return An ASTNode of type LogicalAnd or LogicalOr or any of the downstream nodes
         */
        [[nodiscard]] tiny::ASTNode logicalExpression();

        /*!
         * \brief Consumes an expression built from an equality or un-equality check or derivatives to the downstream parsers
         * \return An ASTNode of type CompareEq, CompareNeq, CompareGteq or CompareLteq or any of the downstream nodes
         */
        [[nodiscard]] tiny::ASTNode equalityExpression();

        /*!
         * \brief Consumes an expression built from a relational comparison between values or derivatives to the downstream parsers
         * \return An ASTNode of type CompareGt or CompareLt or any of the downstream nodes
         */
        [[nodiscard]] tiny::ASTNode relationalExpression();

        /*!
         * \brief Consumes an expression built from the + or - operators or derivatives to the downstream parsers
         * \return An ASTNode of type OpAddition or OpSubtraction or any of the downstream nodes
         */
        [[nodiscard]] tiny::ASTNode additiveExpression();

        /*!
         * \brief Consumes an expression built from the * or / operators or derivatives to the downstream parsers
         * \return An ASTNode of type OpMultiplication or OpDivision or any of the downstream nodes
         */
        [[nodiscard]] tiny::ASTNode multiplicativeExpression();

        /*!
         * \brief Consumes an expression built from the ** operator or derivatives to the downstream parsers
         * \return An ASTNode of type OpExponentiate or any of the downstream nodes
         */
        [[nodiscard]] tiny::ASTNode exponentiatingExpression();

        /*!
         * \brief Consumes an unary expression that actuates over an expression or derivatives to the downstream parsers
         * \return An ASTNode of any of the downstream nodes that might have an unary property added
         *
         * Consumes an unary expression that actuates over an expression, such as a negation (!x), value-at ($x),
         * dereference (&x) or a negative (-x) or derivatives to the downstream parsers
         */
        [[nodiscard]] tiny::ASTNode unaryExpression();

        /*!
         * \brief Consumes an expression that does a call to a method or function or derivatives to the downstream parsers
         * \return An ASTNode of type FunctionCall with the FunctionCallArgumentList child or any of the downstream nodes
         */
        [[nodiscard]] tiny::ASTNode callExpression();

        /*!
         * \brief Consumes an expression that accesses a member of an struct such as x.y or derivatives to the downstream parsers
         * \return An ASTNode of type MemberAccess or IndexedAccess with the ComputedAccess parameter set or any of the downstream nodes
         */
        [[nodiscard]] tiny::ASTNode memberExpression();

        /*!
         * \brief Consumes a parenthesised expression or derivatives to the downstream parsers
         * \return An ASTNode of the type any of the downstream nodes
         */
        [[nodiscard]] tiny::ASTNode parenthExpression();

        /*!
         * \brief Consumes an expression that results in an assignable LHS or derivatives to the downstream parsers
         * \return An ASTNode of the type any of the downstream nodes
         *
         * Consumes an expression that results in an assignable left-hand-side value such as an identifier or a variable
         * declaration or derivatives to the downstream parsers
         */
        [[nodiscard]] tiny::ASTNode assignableLHSExpression();

        /*!
         * \brief Consumes an expression that utilizes a value whose type is explicitly declared or derivatives to the downstream parsers
         * \return An ASTNode of type TypedExpression or any of the downstream nodes
         */
        [[nodiscard]] tiny::ASTNode typedExpression();

        /*!
         * \brief The primary expression over a single unitary token
         * \return An ASTNode of a parenthesised expression, a LHS assignable expression or a literal
         */
        [[nodiscard]] tiny::ASTNode primary();

        /*!
         * \brief Consumes any type of literal expression
         * \return An ASTNode of a type LiteralInt, LiteralDecimal, LiteralString, LiteralChar, LiteralBool or LiteralNone
         */
        [[nodiscard]] tiny::ASTNode literal();

        /*!
         * \brief Consumes a numeric literal
         * \return An ASTNode of type LiteralInt or LiteralDecimal
         */
        [[nodiscard]] tiny::ASTNode literalNum();

        /*!
         * \brief Consumes a string literal
         * \return An ASTNode of type LiteralString
         */
        [[nodiscard]] tiny::ASTNode literalStr();

        /*!
         * \brief Consumes a character literal
         * \return An ASTNode of type LiteralChar
         */
        [[nodiscard]] tiny::ASTNode literalChar();

        /*!
         * \brief Consumes a boolean literal
         * \return An ASTNode of type LiteralBool
         */
        [[nodiscard]] tiny::ASTNode literalBool();

        /*!
         * \brief Consumes a none literal
         * \return An ASTNode of type LiteralNone
         */
        [[nodiscard]] tiny::ASTNode literalNone();

        /*!
         * \brief Consumes an identifier that might have an access modifier
         * \return An ASTNode of type Identifier
         *
         * Consumes an identifier that might (but might not) have an access modifier such as a pointer access or a
         * value-at dereference
         */
        [[nodiscard]] tiny::ASTNode addressableIdentifier();

        /*!
         * \brief Consumes a plain identifier
         * \return An ASTNode of type Identifier
         */
        [[nodiscard]] tiny::ASTNode identifier();

        /*!
         * \brief Consumes an addressable built-in or custom type
         * \return An ASTNode of type Type
         *
         * A built-in type or an identifier (custom type) that might or might not have an access modifier (pointer
         * access or value-at dereference) and might or might not be defined as constant (preceded by the 'const'
         * keyword)
         */
        [[nodiscard]] tiny::ASTNode addressableType();

        /*!
         * \brief Fetches the metadata of the latest lexeme without modifying the stream's position
         * \return The Metadata object of the latest lexeme in the stream
         */
        [[nodiscard]] inline tiny::Metadata getMetadata() const {
            return s.get(s.getIndex()-1).metadata;
        }

        //! A list of skipable tokens that provide no semantic meaning
        const std::vector<tiny::Token> SKIPABLE_TOKENS{ tiny::Token::SinglelineComment,
                                                        tiny::Token::MultilineComment,
                                                        tiny::Token::NewLine};

        //! The program stream as a WalkableStream of Lexemes
        tiny::WalkableStream<tiny::Lexeme> &s;

    };
}

#endif //TINY_PARSER_H
