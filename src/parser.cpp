#include "parser.h"
#include "errors.h"

#include <utility>

tiny::Lexeme tiny::Parser::consume(const tiny::Token &token) {
    auto got = s.get();

    if (s.isTerminator(got)) {
        // Use the metadata of the last token
        throw tiny::ParseError("Unexpected end-of-file", getMetadata());
    }

    if (token != got.token) {
        throw tiny::ParseError(
                "Unexpected token: expected " + tiny::Lexeme(token).string() + " but got " + got.string(),
                got.metadata);
    }

    return got;
}

bool tiny::Parser::consumeOptional(const tiny::Token &token) {
    if (s.peek() == token) {
        s.skip();
        return true;
    }

    return false;
}

void tiny::Parser::exhaust(tiny::Token token) {
    while (consumeOptional(token));
}

void tiny::Parser::exhaust(const std::vector<tiny::Token> &tokens) {
    while (std::find(tokens.begin(), tokens.end(), s.peek().token) != tokens.end()) {
        exhaust(s.peek().token);
    }
}

bool tiny::Parser::check(tiny::Token token) {
    auto peek = s.peek();
    if (s.isTerminator(peek) || token != peek.token) {
        return false;
    }

    return true;
}

/*
 *  File ::= <StatementList>
 *     -> Imports
 *     -> ModuleName
 */
tiny::ASTFile tiny::Parser::file(std::string_view filename, bool requireModule) {
    auto mod = moduleStatement(!requireModule);
    auto imprts = importStatement();

    return tiny::ASTFile(
            filename,
            mod,
            imprts,
            statementList());
}

/*
 *  ModuleStatement ::= module STRING
 */
tiny::String tiny::Parser::moduleStatement(bool optional) {
    exhaust(SKIPABLE_TOKENS);

    if (!consumeOptional(tiny::Token::KwModule)) {
        if (optional) {
            return "";
        }

        throw tiny::ParseError("No module name defined", getMetadata());
    }

    return consume(tiny::Token::Id).value;
}

/*
 *  ImportStatement ::= [(SKIPABLE_TOKENS*) import { [(SKIPABLE_TOKENS*) STRING ]+ } | e]
 */
std::vector<tiny::Import> tiny::Parser::importStatement() {
    exhaust(SKIPABLE_TOKENS);

    std::vector<tiny::Import> imports;

    if (!consumeOptional(tiny::Token::KwImport)) {
        return imports; // No imports
    }

    consume(tiny::Token::OParenthesis);
    while (true) {
        exhaust(SKIPABLE_TOKENS);

        tiny::Import imprt(consume(tiny::Token::Id).value);

        // Import alias?
        if (consumeOptional(tiny::Token::KwAs)) {
            imprt.alias = consume(tiny::Token::Id).value;
        }

        imports.push_back(imprt);

        if (!consumeOptional(tiny::Token::Comma)) {
            exhaust(SKIPABLE_TOKENS);
            consume(tiny::Token::CParenthesis);

            return imports;
        }
    }
}

/*
 *  StatementList ::= <Statement>
 *                 |  <StatementList> <Statement> -> <Statement> <Statement> ...
 */
tiny::StatementList tiny::Parser::statementList(tiny::Token stopToken) {
    tiny::StatementList statements;

    while (s) {
        if (check(stopToken)) {
            s.skip();
            break;
        }

        exhaust(SKIPABLE_TOKENS);
        statements.push_back(statement({tiny::Token::NewLine, stopToken}));
        exhaust(SKIPABLE_TOKENS);
    }

    return statements;
}

/*
 *  Statement ::= <ExpressionStatement>
 *             |  <BlockStatement>
 *             |  <ErrorHandledBlockStatement>
 *             |  <IfStatement>
 *             |  <ForStatement>
 *             |  <ReturnStatement>
 */
tiny::ASTNode tiny::Parser::statement(std::vector<tiny::Token> terminators) {
    switch (s.peek().token) {
        case tiny::Token::OBraces:
            return errorHandledBlockStatement();
        case tiny::Token::KwIf:
            return ifStatement();
        case tiny::Token::KwFor:
            return forStatement();
        case tiny::Token::KwFunc:
            return funcDeclStatement();
        case tiny::Token::KwReturn:
            return returnStatement();
        case tiny::Token::KwStruct:
            return structStatement();
        case tiny::Token::KwTrait:
            return traitStatement();
        case tiny::Token::KwModule:
            throw tiny::ParseError("The module name can only be defined once at the start of the file", getMetadata());
        case tiny::Token::KwImport:
            throw tiny::ParseError("Import statements can only be placed immediately after the module name",
                                   getMetadata());
        default:
            return expressionStatement(std::move(terminators));
    }
}

/*
 *  ErrorHandledBlockStatement ::= <BlockStatement> !! <Identifier> <BlockStatement>
 *                             |   <BlockStatement>
 */
tiny::ASTNode tiny::Parser::errorHandledBlockStatement() {
    auto lhs = blockStatement();

    if (consumeOptional(tiny::Token::Doublebang)) {
        auto id = consume(tiny::Token::Id);

        if (check(tiny::Token::OBraces)) {
            // Inlined block handle
            auto node = tiny::ASTNode(tiny::ASTNodeType::ErrorHandle, lhs, blockStatement());

            auto varNameParam = tiny::Parameter(tiny::ParameterType::ErrorVarName, id.value);
            node.addParam(varNameParam);

            return node;
        }

        // Callback handler
        auto node = tiny::ASTNode(tiny::ASTNodeType::ErrorHandle, lhs);

        auto callbackName = tiny::Parameter(tiny::ParameterType::ErrorCallback, id.value);
        node.addParam(callbackName);

        return node;
    }

    return lhs;
}

/*
 *  BlockStatement ::= { (\n*) ([<ExpressionStatement>\n|\n]*) } (\n*)
 */
tiny::ASTNode tiny::Parser::blockStatement() {
    consume(tiny::Token::OBraces);
    exhaust(tiny::Token::NewLine);

    tiny::ASTNode exp(tiny::ASTNodeType::BlockStatement);
    exp.addChildren(statementList(tiny::Token::CBraces));
    exhaust(tiny::Token::NewLine);

    return exp;
}

/*
 *  IfStatement ::= if (\n*) <ExpressionStatement> { (\n*) <Statement>*) } (\n*)
 *               |  if (\n*) <ExpressionStatement> { (\n*) <Statement>*) } (\n*) else (\n*) { (\n*) <Statement>*) } (\n*)
 */
tiny::ASTNode tiny::Parser::ifStatement() {
    consume(tiny::Token::KwIf);
    exhaust(tiny::Token::NewLine);

    auto condition = tiny::ASTNode(tiny::ASTNodeType::BranchCondition, expression());

    exhaust(tiny::Token::NewLine);
    auto consequent = tiny::ASTNode(tiny::ASTNodeType::BranchConsequent, blockStatement());
    exhaust(tiny::Token::NewLine);

    if (!consumeOptional(tiny::Token::KwElse)) {
        // No else
        return tiny::ASTNode(tiny::ASTNodeType::IfStatement, condition, consequent);
    }

    exhaust(tiny::Token::NewLine);
    auto alternative = tiny::ASTNode(tiny::ASTNodeType::BranchAlternative, blockStatement());
    exhaust(tiny::Token::NewLine);

    return tiny::ASTNode(tiny::ASTNodeType::IfStatement, condition, consequent, alternative);
}


/*
 *  ForStatement ::= for (\n*) [<RangeExpression>|<ForEachExpression>|<Expression>|""] { (\n*) <Statement>+) } (\n*)
 */
tiny::ASTNode tiny::Parser::forStatement() {
    consume(tiny::Token::KwFor);
    exhaust(tiny::Token::NewLine);

    tiny::ASTNode conditionDownstream;
    std::uint64_t streamCheckpoint = s.getIndex();

    // Try each of the possible expressions that can go inside the for. For correctly predicting this choice
    // more than L(1) is needed; thus we try the for-specific expressions in order and else default to a
    // generic Expression node.
    if (check(tiny::Token::OBraces)) {
        // Empty for (infinite loop). Equivalent to "for true {}"

        conditionDownstream = tiny::ASTNode(tiny::ASTNodeType::LiteralBool);
        conditionDownstream.val = true;

    } else {
        try {
            conditionDownstream = rangeExpression();
        } catch (tiny::ParseError &) {
            s.seek(streamCheckpoint);
            try {
                conditionDownstream = forEachExpression();
            } catch (tiny::ParseError &) {
                s.seek(streamCheckpoint);
                conditionDownstream = expression();

                // Don't catch the last one, since if it fails the for is malformed and the error should propagate
            }
        }
    }


    auto condition = tiny::ASTNode(tiny::ASTNodeType::BranchCondition, conditionDownstream);

    exhaust(tiny::Token::NewLine);
    auto consequent = tiny::ASTNode(tiny::ASTNodeType::BranchConsequent, blockStatement());
    exhaust(tiny::Token::NewLine);

    return tiny::ASTNode(tiny::ASTNodeType::ForStatement, condition, consequent);
}

/*
 *  RangeExpression ::= <Identifier> := <AdditiveExpression>..(<AdditiveExpression>) (-> <AdditiveExpression>)
 */
tiny::ASTNode tiny::Parser::rangeExpression() {
    tiny::ASTNode node(tiny::ASTNodeType::RangeExpression);

    auto id = consume(tiny::Token::Id);
    node.addParam(tiny::Parameter(tiny::ParameterType::RangeIdentifier, id.value));

    consume(tiny::Token::Init);

    // Don't accept anything upstream from additive since it might involve non-numeric operands
    node.addChildren(tiny::ASTNode(tiny::ASTNodeType::RangeFromExpression, additiveExpression())); // From
    consume(tiny::Token::Range);

    exhaust(tiny::Token::NewLine);
    if (!check(tiny::Token::OBraces) && !check(tiny::Token::Step)) {
        node.addChildren(tiny::ASTNode(tiny::ASTNodeType::RangeToExpression, additiveExpression())); // To
    } else {
        node.addChildren(tiny::ASTNode(tiny::ASTNodeType::RangeToExpression)); // Placeholder to
    }

    if (consumeOptional(tiny::Token::Step)) {
        node.addChildren(tiny::ASTNode(tiny::ASTNodeType::RangeStepExpression, additiveExpression())); // StageStep
    } else {
        node.addChildren(tiny::ASTNode(tiny::ASTNodeType::RangeStepExpression)); // Placeholder step
    }

    return node;
}

/*
 *  ForEachExpression ::= <Identifier> in <AdditiveExpression>
 */
tiny::ASTNode tiny::Parser::forEachExpression() {
    tiny::ASTNode node(tiny::ASTNodeType::ForEachExpression);

    auto id = consume(tiny::Token::Id);
    node.addParam(tiny::Parameter(tiny::ParameterType::RangeIdentifier, id.value));

    consume(tiny::Token::KwIn);

    // Don't accept anything upstream from additive since it might involve non-numeric operands
    node.addChildren(additiveExpression()); // From

    return node;
}

/*
 *  FuncDeclStatement ::= func ('(' <TypedExpression> ')') <Identifier> <FunctionArgumentDeclList> <ReturnDeclStatement> \n* <BlockStatement>
 */
tiny::ASTNode tiny::Parser::funcDeclStatement(bool isPrototype) {
    consume(tiny::Token::KwFunc);

    tiny::ASTNode node(tiny::ASTNodeType::FunctionDeclaration);

    // Method?
    if (!isPrototype && consumeOptional(tiny::Token::OParenthesis)) {
        node.type = tiny::ASTNodeType::MethodDeclaration;

        auto typeExp = typedExpression();
        typeExp.type = tiny::ASTNodeType::MethodType;

        node.addChildren(typeExp);

        consume(tiny::Token::CParenthesis);
    }

    auto id = consume(tiny::Token::Id);
    node.addParam(tiny::Parameter(tiny::ParameterType::Name, id.value));

    node.addChildren(argumentDeclList(!isPrototype));
    node.addChildren(returnDeclList());

    exhaust(tiny::Token::NewLine);

    if (!isPrototype) {
        node.addChildren(tiny::ASTNode(ASTNodeType::FunctionBody, blockStatement()));
    }

    return node;
}

/*
 *  FunctionArgumentDeclList ::= ([<AddressableType> (ConstraintList)[, <AddressableType> (ConstraintList)]*|e])
 */
tiny::ASTNode tiny::Parser::argumentDeclList(bool hasNamedArgs) {
    tiny::ASTNode node(tiny::ASTNodeType::FunctionArgumentDeclList);

    consume(tiny::Token::OParenthesis);

    while (!check(tiny::Token::CParenthesis)) {
        // Expect an argument, then if no comma is found break the loop

        auto arg = addressableType();
        arg.type = tiny::ASTNodeType::FunctionArgumentDecl;

        if (hasNamedArgs) {
            arg.addParam(tiny::Parameter(tiny::ParameterType::Name, identifier().val));
        }

        // Argument constrains
        if (check(tiny::Token::OBrackets)) {
            arg.addChildren(traitListStatement());
        }

        node.addChildren(arg);

        if (!consumeOptional(tiny::Token::Comma)) {
            break;
        }
    }

    consume(tiny::Token::CParenthesis);

    return node;
}

tiny::ASTNode tiny::Parser::returnDeclList() {
    tiny::ASTNode node(tiny::ASTNodeType::FunctionReturnDeclList);

    bool isParenthesised = consumeOptional(tiny::Token::OParenthesis);

    while (check(tiny::Token::Id) || s.peek().isType()) {
        // Expect a return type, then if no comma is found break the loop

        auto ret = addressableType();
        ret.type = tiny::ASTNodeType::FunctionArgumentDecl;

        node.addChildren(ret);

        if (!isParenthesised) {
            break;
        }

        if (!consumeOptional(tiny::Token::Comma)) {
            break;
        }
    }

    if (isParenthesised) {
        consume(tiny::Token::CParenthesis);
    }

    return node;
}

/*
 *  ReturnStatement ::= return ([<Expression>[, <Expression>]*|e])
 */
tiny::ASTNode tiny::Parser::returnStatement() {
    consume(tiny::Token::KwReturn);

    auto node = commaSeparatedExpressionList();
    node.type = tiny::ASTNodeType::FunctionReturn;

    return node;
}

/*
 *  CommaSeparatedExpressionList ::= [<Expression>[, <Expression>]*|e] (TERMINATOR)
 */
tiny::ASTNode tiny::Parser::commaSeparatedExpressionList(const std::vector<tiny::Token> &terminators) {
    tiny::ASTNode node(tiny::ASTNodeType::ExpressionList);

    while (std::find(terminators.begin(), terminators.end(), s.peek().token) == terminators.end()) {
        node.addChildren(expression());

        if (!consumeOptional(tiny::Token::Comma)) {
            return node;
        }
    }

    return node;
}

/*
 *  StructStatement ::= struct <Identifier> (/n*) (<TraitListStatement>) (/n*) <StructFieldList> (/n*)
 */
tiny::ASTNode tiny::Parser::structStatement() {
    consume(tiny::Token::KwStruct);

    tiny::ASTNode node(tiny::ASTNodeType::StructDeclaration);
    node.addParam(tiny::Parameter(tiny::ParameterType::Name, identifier().val));

    exhaust(tiny::Token::NewLine);

    if (check(tiny::Token::OBrackets)) {
        node.addChildren(traitListStatement());
    }

    exhaust(tiny::Token::NewLine);

    node.addChildren(structFieldList());

    exhaust(tiny::Token::NewLine);

    return node;
}

/*
 *  TraitStatement ::= trait <Identifier> (/n*) (<TraitListStatement>) (/n*) <TraitFieldList> (/n*)
 */
tiny::ASTNode tiny::Parser::traitStatement() {
    consume(tiny::Token::KwTrait);

    tiny::ASTNode node(tiny::ASTNodeType::TraitDeclaration);
    node.addParam(tiny::Parameter(tiny::ParameterType::Name, identifier().val));

    exhaust(tiny::Token::NewLine);

    if (check(tiny::Token::OBrackets)) {
        node.addChildren(traitListStatement());
    }

    exhaust(tiny::Token::NewLine);

    node.addChildren(traitFieldList());

    exhaust(tiny::Token::NewLine);

    return node;
}


/*
 *  TraitListStatement ::= '[' (<Identifier> (/n*) [, <Identifier>]*|e) ']'
 */
tiny::ASTNode tiny::Parser::traitListStatement() {
    consume(tiny::Token::OBrackets);

    tiny::ASTNode node(tiny::ASTNodeType::TraitList);

    while (!check(tiny::Token::CBrackets)) {
        exhaust(tiny::Token::NewLine);

        auto trait = identifier();
        trait.type = tiny::ASTNodeType::Trait;

        node.addChildren(trait);

        if (!consumeOptional(tiny::Token::Comma)) {
            break;
        }
    }

    consume(tiny::Token::CBrackets);
    return node;
}

/*
 *  StructFieldList ::= { <Identifier> [,  (/n*) <Identifier>]*|e) }
 */
tiny::ASTNode tiny::Parser::structFieldList() {
    tiny::ASTNode node(tiny::ASTNodeType::StructFieldList);

    consume(tiny::Token::OBraces);
    exhaust(tiny::Token::NewLine);

    // A field can be a composition or a member value
    while (!check(tiny::Token::CBraces)) {
        // Composition?
        if (check(tiny::Token::Id)) {
            auto id = identifier();
            if (check(tiny::Token::Comma)) {
                // Lone ID. Composition
                id.type = tiny::ASTNodeType::Composition;
                node.addChildren(id);

                if (!consumeOptional(tiny::Token::Comma)) {
                    exhaust(tiny::Token::NewLine);
                    break;
                }

                exhaust(tiny::Token::NewLine);

                continue;
            }

            // No comma afterwards, revert and try to interpret it as a value
            s.backup();
        }

        auto field = typedExpression();
        field.type = tiny::ASTNodeType::StructField;

        if (field.getParam(tiny::ParameterType::Const).has_value()) {
            s.backup();
            throw tiny::ParseError("Constant types are not allowed inside structs", s.get().metadata);
        }

        node.addChildren(field);

        if (!consumeOptional(tiny::Token::Comma)) {
            exhaust(tiny::Token::NewLine);
            break;
        }

        exhaust(tiny::Token::NewLine);
    }

    consume(tiny::Token::CBraces);
    return node;
}

/*
 *  TraitFieldList ::= { <Identifier> [,  (/n*) <Identifier>]*|e) }
 */
tiny::ASTNode tiny::Parser::traitFieldList() {
    tiny::ASTNode node(tiny::ASTNodeType::TraitFieldList);

    consume(tiny::Token::OBraces);
    exhaust(tiny::Token::NewLine);

    // A trait can be a member value or a function
    while (!check(tiny::Token::CBraces)) {
        // Function?
        if (check(tiny::Token::KwFunc)) {
            node.addChildren(funcDeclStatement(true));

            if (!consumeOptional(tiny::Token::Comma)) {
                exhaust(tiny::Token::NewLine);
                break;
            }

            exhaust(tiny::Token::NewLine);

            continue;
        }

        auto field = typedExpression();
        if (field.getParam(tiny::ParameterType::Const).has_value()) {
            s.backup();
            throw tiny::ParseError("Constant types are not allowed inside traits", s.get().metadata);
        }

        node.addChildren(field);

        if (!consumeOptional(tiny::Token::Comma)) {
            exhaust(tiny::Token::NewLine);
            break;
        }

        exhaust(tiny::Token::NewLine);
    }

    consume(tiny::Token::CBraces);
    return node;
}

/*
 *  ExpressionStatement ::= <Expression>
 */
tiny::ASTNode tiny::Parser::expressionStatement(std::vector<tiny::Token> terminators) {
    tiny::ASTNode exp(tiny::ASTNodeType::ExpressionStatement);
    exp.addChildren(expression());

    // We reached the EOF. This means there's a missing newline at the end.
    if (!s) {
        return exp;
    }

    // Check if the next token is a terminator
    if (std::find(terminators.begin(), terminators.end(), s.peek().token) == terminators.end()) {
        throw ParseError("Invalid expression. Multiple statements", s.peek().metadata);
    }

    return exp;
}

/*
 *  Expression ::= <AssignmentExpression>
 */
tiny::ASTNode tiny::Parser::expression() {
    return errorHandleExpression();
}

/*
 *  ErrorHandleExpression ::= <AssignmentExpression> !! <Identifier> <BlockStatement>
 *                         |  <AssignmentExpression>
 */
tiny::ASTNode tiny::Parser::errorHandleExpression() {
    auto lhs = assignmentExpression();

    if (consumeOptional(tiny::Token::Doublebang)) {
        auto id = consume(tiny::Token::Id);

        if (check(tiny::Token::OBraces)) {
            // Inlined block handle
            auto node = tiny::ASTNode(tiny::ASTNodeType::ErrorHandle, lhs, blockStatement());

            auto varNameParam = tiny::Parameter(tiny::ParameterType::ErrorVarName, id.value);
            node.addParam(varNameParam);

            return node;
        }

        // Callback handler
        auto node = tiny::ASTNode(tiny::ASTNodeType::ErrorHandle, lhs);

        auto callbackName = tiny::Parameter(tiny::ParameterType::ErrorCallback, id.value);
        node.addParam(callbackName);

        return node;
    }

    return lhs;
}

/*
 *  AssignmentExpression ::= <LogicalExpression>
 *                        |  <TypedExpression> ':=' <LogicalExpression>
 */
tiny::ASTNode tiny::Parser::assignmentExpression() {
    auto lhs = logicalExpression();

    tiny::ASTNodeType op;
    switch (s.peek().token) {
        case tiny::Token::Init:
            op = tiny::ASTNodeType::Initialization;
            break;
        case tiny::Token::Assign:
            op = tiny::ASTNodeType::Assignment;
            break;
        case tiny::Token::AssignSum:
            op = tiny::ASTNodeType::AssignmentSum;
            break;
        case tiny::Token::AssignSub:
            op = tiny::ASTNodeType::AssignmentSub;
            break;
        case tiny::Token::AssignMulti:
            op = tiny::ASTNodeType::AssignmentMulti;
            break;
        case tiny::Token::AssignDiv:
            op = tiny::ASTNodeType::AssignmentDiv;
            break;
        default: {
            if (lhs.type == tiny::ASTNodeType::TypedExpression) {
                // The downstream expression says this is an assignment, but there is no assignment operation so
                // this mut be a declaration without initialization
                lhs.type = tiny::ASTNodeType::VarDeclaration;
            }

            return lhs; // Not an assignment
        }

    }

    s.skip(); // Go over the assignment token

    if (lhs.type != tiny::ASTNodeType::TypedExpression && lhs.type != tiny::ASTNodeType::Identifier
        && lhs.type != tiny::ASTNodeType::MemberAccess && lhs.type != tiny::ASTNodeType::IndexedAccess) {
        throw tiny::ParseError("Invalid assignment. Can only assign a value to an identifier", s.get().metadata);
    }

    return tiny::ASTNode(op, lhs, logicalExpression());
}

/*
 *  LogicalExpression ::= <EqualityExpression>
 *                      |  <EqualityExpression> [[<=|>=|==|!=] <EqualityExpression>]*
 */
tiny::ASTNode tiny::Parser::logicalExpression() {
    auto lhs = equalityExpression();

    while (true) {
        tiny::ASTNodeType op;

        switch (s.peek().token) {
            case tiny::Token::KwAnd: {
                s.skip();
                op = tiny::ASTNodeType::LogicalAnd;
                break;
            }
            case tiny::Token::KwOr: {
                s.skip();
                op = tiny::ASTNodeType::LogicalOr;
                break;
            }
            default:
                return lhs;
        }

        lhs = tiny::ASTNode(op, lhs, equalityExpression());
    }
}

/*
 *  EqualityExpression ::= <RelationalExpression>
 *                      |  <RelationalExpression> [[and|or] <RelationalExpression>]*
 */
tiny::ASTNode tiny::Parser::equalityExpression() {
    auto lhs = relationalExpression();

    while (true) {
        tiny::ASTNodeType op;

        switch (s.peek().token) {
            case tiny::Token::Eq: {
                s.skip();
                op = tiny::ASTNodeType::CompareEq;
                break;
            }
            case tiny::Token::Neq: {
                s.skip();
                op = tiny::ASTNodeType::CompareNeq;
                break;
            }
            case tiny::Token::Gteq: {
                s.skip();
                op = tiny::ASTNodeType::CompareGteq;
                break;
            }
            case tiny::Token::Lteq: {
                s.skip();
                op = tiny::ASTNodeType::CompareLteq;
                break;
            }
            default:
                return lhs;
        }

        lhs = tiny::ASTNode(op, lhs, relationalExpression());
    }
}

/*
 *  RelationalExpression ::= <AdditiveExpression>
 *                        |  <AdditiveExpression> [[>|<] <AdditiveExpression>]*
 */
tiny::ASTNode tiny::Parser::relationalExpression() {
    auto lhs = additiveExpression();

    while (true) {
        tiny::ASTNodeType op;

        switch (s.peek().token) {
            case tiny::Token::Gt: {
                s.skip();
                op = tiny::ASTNodeType::CompareGt;
                break;
            }
            case tiny::Token::Lt: {
                s.skip();
                op = tiny::ASTNodeType::CompareLt;
                break;
            }
            default:
                return lhs;
        }

        lhs = tiny::ASTNode(op, lhs, additiveExpression());
    }
}

/*
 *  AdditiveExpression ::= <MultiplicativeExpression>
 *                      |  <MultiplicativeExpression> [[+|-] <MultiplicativeExpression>]*
 */
tiny::ASTNode tiny::Parser::additiveExpression() {
    auto lhs = multiplicativeExpression();

    while (true) {
        tiny::ASTNodeType op;

        switch (s.peek().token) {
            case tiny::Token::Sum: {
                s.skip();
                op = tiny::ASTNodeType::OpAddition;
                break;
            }
            case tiny::Token::Sub: {
                s.skip();
                op = tiny::ASTNodeType::OpSubtraction;
                break;
            }
            default:
                return lhs;
        }

        lhs = tiny::ASTNode(op, lhs, multiplicativeExpression());
    }
}

/*
 *  MultiplicativeExpression ::= <ExponentiatingExpression>
 *                            |  <ExponentiatingExpression> [[*|'/'] <ExponentiatingExpression>]*
 */
tiny::ASTNode tiny::Parser::multiplicativeExpression() {
    auto lhs = exponentiatingExpression();

    while (true) {
        tiny::ASTNodeType op;

        switch (s.peek().token) {
            case tiny::Token::Multi: {
                s.skip();
                op = tiny::ASTNodeType::OpMultiplication;
                break;
            }
            case tiny::Token::Div: {
                s.skip();
                op = tiny::ASTNodeType::OpDivision;
                break;
            }
            default:
                return lhs;
        }

        lhs = tiny::ASTNode(op, lhs, exponentiatingExpression());
    }
}

/*
 *  ExponentiatingExpression ::= <UnaryExpression>
 *                            |  <UnaryExpression> [** <UnaryExpression>]*
 */
tiny::ASTNode tiny::Parser::exponentiatingExpression() {
    auto lhs = unaryExpression();

    while (consumeOptional(tiny::Token::Exp)) {
        lhs = tiny::ASTNode(tiny::ASTNodeType::OpExponentiate, lhs, unaryExpression());
    }

    return lhs;
}

/*
 *  UnaryExpression ::= <MemberExpression>
 *                   |  -<MemberExpression>
 *                   |  !<MemberExpression>
 *                   |  $<MemberExpression>
 *                   |  &<MemberExpression>
 */
tiny::ASTNode tiny::Parser::unaryExpression() {
    if (consumeOptional(tiny::Token::Sub)) {
        return tiny::ASTNode(tiny::ASTNodeType::UnaryNegative, unaryExpression());
    }

    if (consumeOptional(tiny::Token::Negation)) {
        return tiny::ASTNode(tiny::ASTNodeType::UnaryNot, unaryExpression());
    }

    if (consumeOptional(tiny::Token::ValueAt)) {
        auto node = callExpression();
        node.addParam(tiny::Parameter(tiny::ParameterType::ValueAt));

        return node;
    }

    if (consumeOptional(tiny::Token::Dereference)) {
        auto node = callExpression();
        node.addParam(tiny::Parameter(tiny::ParameterType::Dereference));

        return node;
    }

    return callExpression();
}

/*
 *  CallExpression ::= <MemberExpression> (CommaSeparatedExpressionList)
 *                  |  <MemberExpression>
 */
tiny::ASTNode tiny::Parser::callExpression() {
    auto lhs = memberExpression();

    while (consumeOptional(tiny::Token::OParenthesis)) {
        auto args = commaSeparatedExpressionList({tiny::Token::CParenthesis});
        args.type = ASTNodeType::FunctionCallArgumentList;

        consume(tiny::Token::CParenthesis);

        lhs = tiny::ASTNode(tiny::ASTNodeType::FunctionCall, lhs, args);
    }

    return lhs;
}

/*
 *  MemberExpression ::= <MemberExpression>.<Identifier>
 *                    |  <MemberExpression>[<Expression>]
 *                    |  <Primary>
 */
tiny::ASTNode tiny::Parser::memberExpression() {
    auto lhs = primary();

    while (check(tiny::Token::MemberAccess) || check(tiny::Token::OBrackets)) {
        // <MemberExpression>.<Identifier>
        if (consumeOptional(tiny::Token::MemberAccess)) {
            lhs = tiny::ASTNode(tiny::ASTNodeType::MemberAccess, lhs, identifier());
            lhs.addParam(tiny::Parameter(tiny::ParameterType::ComputedAccess, false));
        }

        // <MemberExpression>[<Expression>]
        if (consumeOptional(tiny::Token::OBrackets)) {
            // Don't use expression() since it'll allow for error-handled expressions
            lhs = tiny::ASTNode(tiny::ASTNodeType::IndexedAccess, lhs, assignmentExpression());
            lhs.addParam(tiny::Parameter(tiny::ParameterType::ComputedAccess, true));

            consumeOptional(tiny::Token::CBrackets);
        }
    }

    return lhs;
}

/*
 *  Primary ::= <ParenthExpression>
 *           |  <Literal>
 *           |  <LHSAssignmentExpression>
 */
tiny::ASTNode tiny::Parser::primary() {
    switch (s.peek().token) {
        case tiny::Token::OParenthesis:
            return parenthExpression();
        case tiny::Token::KwConst:
        case tiny::Token::Multi: // Pointer
        case tiny::Token::Dereference:
        case tiny::Token::ValueAt:
        case tiny::Token::Id:
            return assignableLHSExpression();
        default: {
            if (s.peek().isType()) {
                return assignableLHSExpression();
            }

            return literal();
        }
    }

}

/*
 *  AssignableLHSExpression ::= <TypedExpression>
 *                           |  <Identifier>
 */
tiny::ASTNode tiny::Parser::assignableLHSExpression() {
    auto checkpoint = s.getIndex();
    try {
        return typedExpression();
    } catch (tiny::ParseError &) {
        s.seek(checkpoint);
    }

    return identifier();
}

/*
 *  TypedExpression ::= <AddressableType> <Identifier>
 *                   |  (const) <AddressableIdentifier> <Identifier>
 */
tiny::ASTNode tiny::Parser::typedExpression() {
    // <AddressableType> <Identifier>
    auto checkpoint = s.getIndex();
    try {
        tiny::ASTNode node(tiny::ASTNodeType::TypedExpression);
        node.addChildren(addressableType());

        node.val = consume(tiny::Token::Id).value;

        return node;
    } catch (tiny::ParseError &) {
        s.seek(checkpoint);
    }

    // (const) <AddressableIdentifier> <Identifier>
    tiny::ASTNode node(tiny::ASTNodeType::TypedExpression);

    auto isConst = consumeOptional(tiny::Token::KwConst);

    auto id1 = addressableIdentifier();
    id1.type = tiny::ASTNodeType::Type;

    if (isConst) {
        id1.addParam(tiny::Parameter(tiny::ParameterType::Const));
    }

    node.addChildren(id1);

    node.val = identifier().val;
    return node;
}

/*
 *  ParenthExpression ::= '(' <Expression> ')'
 */
tiny::ASTNode tiny::Parser::parenthExpression() {
    consume(tiny::Token::OParenthesis);
    auto exp = expression();
    consume(tiny::Token::CParenthesis);

    return exp;
}

/*
 *  Literal ::= <LiteralNum>
 *           |  <LiteralStr>
 *           |  <LiteralChar>
 *           |  <LiteralBool>
 *           |  <LiteralNone>
 */
tiny::ASTNode tiny::Parser::literal() {
    switch (s.peek().token) {
        case tiny::Token::LiteralNum:
            return literalNum();
        case tiny::Token::LiteralStr:
            return literalStr();
        case tiny::Token::LiteralChar:
            return literalChar();
        case tiny::Token::LiteralTrue:
        case tiny::Token::LiteralFalse:
            return literalBool();
        case tiny::Token::LiteralNone:
            return literalNone();
        default:
            throw ParseError("Invalid literal", s.peek().metadata);
    }
}

/*
 *  LiteralNum ::= [0-9]*(.[0-9]*)
 */
tiny::ASTNode tiny::Parser::literalNum() {
    // TODO ? Check if the number doesn't fit into a int64 or it's an unsigned number
    auto lexeme = consume(tiny::Token::LiteralNum);

    if (lexeme.value.toString().find('.') != std::string::npos) { // Check if it's a decimal number
        auto node = tiny::ASTNode(tiny::ASTNodeType::LiteralDecimal);
        node.val = std::stold(lexeme.value.toString());

        return node;
    }

    // Default to an int64
    auto node = tiny::ASTNode(tiny::ASTNodeType::LiteralInt);
    node.val = std::int64_t(std::stoll(lexeme.value.toString(), nullptr, 0));

    return node;
}

/*
 *  LiteralStr ::= STRING
 */
tiny::ASTNode tiny::Parser::literalStr() {
    auto lexeme = consume(tiny::Token::LiteralStr);

    auto node = tiny::ASTNode(tiny::ASTNodeType::LiteralString);
    node.val = tiny::String(lexeme.value);

    return node;
}


/*
 *  LiteralChar ::= CHAR
 */
tiny::ASTNode tiny::Parser::literalChar() {
    auto lexeme = consume(tiny::Token::LiteralChar);

    auto node = tiny::ASTNode(tiny::ASTNodeType::LiteralChar);
    node.val = tiny::String(lexeme.value);

    return node;
}

/*
 *  LiteralBool ::= True|False
 */
tiny::ASTNode tiny::Parser::literalBool() {
    auto node = tiny::ASTNode(tiny::ASTNodeType::LiteralBool);

    auto got = s.get();
    switch (got.token) {
        case tiny::Token::LiteralTrue:
            node.val = true;
            return node;
        case tiny::Token::LiteralFalse:
            node.val = false;
            return node;
        default:
            throw tiny::ParseError("Invalid boolean literal", got.metadata);
    }
}

/*
 *  LiteralNone ::= None
 */
tiny::ASTNode tiny::Parser::literalNone() {
    auto lexeme = consume(tiny::Token::LiteralNone);
    return tiny::ASTNode(tiny::ASTNodeType::LiteralNone);
}

/*
 *  AddressableIdentifier ::= [$|&|e]<Identifier>
 */
tiny::ASTNode tiny::Parser::addressableIdentifier() {
    if (consumeOptional(tiny::Token::ValueAt)) {
        auto node = identifier();
        node.addParam(tiny::Parameter(tiny::ParameterType::ValueAt));

        return node;
    } else {
        if (consumeOptional(tiny::Token::Dereference)) {
            auto node = identifier();
            node.addParam(tiny::Parameter(tiny::ParameterType::Dereference));

            return node;
        }
    }

    return identifier();
}

/*
 *  Identifier ::= STRING
 */
tiny::ASTNode tiny::Parser::identifier() {
    return tiny::ASTNode(tiny::ASTNodeType::Identifier, consume(tiny::Token::Id).value);
}

/*
 *  AddressableType ::= (const) [*|&|e][<Identifier>|BUILTIN_TYPE]
 */
tiny::ASTNode tiny::Parser::addressableType() {
    tiny::ASTNode node(tiny::ASTNodeType::Type);

    if (consumeOptional(tiny::Token::KwConst)) {
        node.addParam(tiny::Parameter(tiny::ParameterType::Const));
    }

    if (consumeOptional(tiny::Token::Multi)) { // Pointer
        node.addParam(tiny::Parameter(tiny::ParameterType::Pointer));
    } else {
        if (consumeOptional(tiny::Token::Dereference)) {
            node.addParam(tiny::Parameter(tiny::ParameterType::Dereference));
        }
    }

    if (s.peek().isType()) {
        node.val = tiny::getTypeName(s.get().token);
    } else {
        node.val = identifier().val;
    }

    return node;
}
