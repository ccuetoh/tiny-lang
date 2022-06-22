#include "symtab.h"
#include "logger.h"

void tiny::SymbolTable::build() {
    for (const auto &node: ast.statements) {
        update(std::make_shared<ASTNode>(node));
    }
}

void tiny::SymbolTable::update(const std::shared_ptr<ASTNode> &node, const String &withName) {
    switch (node->type) {
    case tiny::ASTNodeType::FunctionDeclaration:
        parseFunction(node);
        return;

    case tiny::ASTNodeType::BlockStatement:
        newInnerScope(withName);
        break;

    case tiny::ASTNodeType::OpAddition:
    case tiny::ASTNodeType::OpSubtraction:
    case tiny::ASTNodeType::OpMultiplication:
    case tiny::ASTNodeType::OpDivision:
    case tiny::ASTNodeType::OpExponentiate:
        parseOperation(node, tiny::Assertion::None);
        return;
    }

    for (const auto &c: node->children) {
        update(c);
    }
}

tiny::Scope* tiny::SymbolTable::getActive() {
    tiny::Scope* focus = &root;
    while (!focus->inner.empty()) {
        focus = &focus->inner.front();
    }

    return focus;
}

tiny::Assertion tiny::SymbolTable::parseOperation(const std::shared_ptr<ASTNode>& node, tiny::Assertion upstream)
{
    tiny::TypeInfo typeInfo(upstream);

    // These operations only work for numerics
    if (node->type == tiny::ASTNodeType::OpDivision
    || node->type == tiny::ASTNodeType::OpExponentiate
    || node->type == tiny::ASTNodeType::OpSubtraction) {
        typeInfo.setType(tiny::Assertion::IsNumeric, node->meta);
    }

    for (const auto &c: node->children) {
        if (c->type == tiny::ASTNodeType::LiteralInt) {
            typeInfo.setType(tiny::Assertion::IsNumeric, node->meta);
            continue;
        }

        if (c->type == tiny::ASTNodeType::LiteralChar || c->type == tiny::ASTNodeType::LiteralString) {
            typeInfo.setType(tiny::Assertion::IsText, node->meta);
            continue;
        }

        if (c->isOperation()) {
            typeInfo.setType(parseOperation(c, typeInfo.getType()), c->meta);
        } else {
            update(c);
        }
    }

    for (const auto &c: node->children) {
        if (c->type==tiny::ASTNodeType::Identifier) {
            getActive()->addPromise(tiny::Promise(
                    c->getStringVal(),
                    tiny::Assertion::IsDefined,
                    c->meta));

            if (typeInfo.isSet()) {
                getActive()->addPromise(tiny::Promise(
                        c->getStringVal(),
                        typeInfo.getType(),
                        c->meta));
            }
        }

        if (c->type==tiny::ASTNodeType::FunctionCall) {
            getActive()->addPromise(tiny::Promise(
                    c->getFirstChild()->getStringVal(),
                    tiny::Assertion::IsDefined,
                    c->meta));

            getActive()->addPromise(tiny::Promise(
                    c->getFirstChild()->getStringVal(),
                    tiny::Assertion::IsCallable,
                    c->meta));

            getActive()->addPromise(tiny::Promise(
                    c->getFirstChild()->getStringVal(),
                    tiny::Assertion::CallReturnCount,
                    "1",
                    c->meta));

            if (typeInfo.isSet()) {
                getActive()->addPromise(tiny::Promise(
                        c->getFirstChild()->getStringVal(),
                        tiny::Assertion::CallReturns,
                        typeInfo.getTypeName(c->meta),
                        0,
                        c->meta));
            }
        }
    }

    return typeInfo.getType();
}

void tiny::SymbolTable::parseFunction(const std::shared_ptr<ASTNode> &node)
{
    auto active = getActive();
    auto funcName = node->getParam(tiny::ParameterType::Name).getStringVal(node->meta);

    active->addFulfilment(tiny::Promise(
            funcName,
            tiny::Assertion::IsDefined,
            node->meta));

    active->addFulfilment(tiny::Promise(
            funcName,
            tiny::Assertion::IsCallable,
            node->meta));

    newInnerScope(funcName);
    auto funcScope = getActive();

    int i = 0;
    for (const auto &arg: node->getChild(tiny::ASTNodeType::FunctionArgumentDeclList)->children) {
        auto argName = arg->getParam(tiny::ParameterType::Name).getStringVal(node->meta);
        auto argType = arg->getStringVal();

        active->addFulfilment(tiny::Promise(
                funcName,
                tiny::Assertion::CallRequires,
                argType,
                i,
                arg->meta));

        funcScope->addFulfilment(tiny::Promise(
                argName,
                tiny::Assertion::IsDefined,
                arg->meta));

        funcScope->addFulfilment(tiny::Promise(
                argName,
                tiny::Assertion::IsOfType,
                argType,
                arg->meta));

        i++;
    }

    i = 0;
    for (const auto &arg: node->getChild(tiny::ASTNodeType::FunctionReturnDeclList)->children) {
        auto argName = arg->getParam(tiny::ParameterType::Name).getStringVal(node->meta);
        auto argType = arg->getStringVal();

        active->addFulfilment(tiny::Promise(
                funcName,
                tiny::Assertion::CallReturns,
                argType,
                i,
                arg->meta));

        funcScope->addFulfilment(tiny::Promise(
                argName,
                tiny::Assertion::IsDefined,
                arg->meta));

        funcScope->addFulfilment(tiny::Promise(
                argName,
                tiny::Assertion::IsOfType,
                argType,
                arg->meta));

        i++;
    }


    active->addFulfilment(tiny::Promise(
            funcName,
            tiny::Assertion::CallReturnCount,
            tiny::String(std::to_string(i)),
            node->meta));


    for (const auto &c: node->getChild(tiny::ASTNodeType::FunctionBody)->children) {
        update(c, funcName);
    }
}

void tiny::SymbolTable::newInnerScope(const tiny::String &name)
{
    getActive()->inner.emplace_back(tiny::Scope{tiny::ScopeType::NonGlobal, name});
}

void tiny::Scope::addPromise(tiny::Promise promise) {
    tiny::debug(promise.meta.file,
            "<- (" + (!name.codepoints.empty() ? name.toString() : "?") + ") " + promise.toString().toString());

    promises.push_back(std::move(promise));
}

void tiny::Scope::addFulfilment(tiny::Promise fulfilment) {
    tiny::debug(fulfilment.meta.file,
            "-> (" + (!name.codepoints.empty() ? name.toString() : "?") + ") " + fulfilment.toString().toString());

    fulfillments.push_back(std::move(fulfilment));
}

tiny::String tiny::Promise::toString() const
{
    switch (assertion) {
    case Assertion::None:
        return "Void assertion";
    case Assertion::IsDefined:
        return identifier + " is defined";
    case Assertion::HasMember:
        return identifier + " has member " + argument;
    case Assertion::IsIndexable:
        return identifier + " is indexable";
    case Assertion::IsStruct:
        return identifier + " is a struct";
    case Assertion::IsCallable:
        return identifier + " is callable";
    case Assertion::CallRequires:
        return identifier + " requires argument " + argument + " in position " + std::to_string(position);
    case Assertion::CallReturns:
        return identifier + " returns " + argument + " in position " + std::to_string(position);
    case Assertion::CallReturnCount:
        return identifier + " returns " + argument + " value(s)";
    case Assertion::IsNumeric:
        return identifier + " is of a numeric type";
    case Assertion::IsText:
        return identifier + " is type-compatible with " + argument;
    case Assertion::IsOfType:
        return identifier + " is of type " + argument;
    default:
        return "Invalid promise";
    }
}
