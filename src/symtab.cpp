#include "symtab.h"
#include "logger.h"

void tiny::SymbolTable::build()
{
    for (const auto &node: ast.statements) {
        update(std::make_shared<ASTNode>(node));
    }
}

void tiny::SymbolTable::update(const std::shared_ptr<ASTNode> &node)
{
    switch (node->type) {
    case tiny::ASTNodeType::BlockStatement:
        getActive().inner.emplace_back();
        break;

    case tiny::ASTNodeType::OpAddition:
    case tiny::ASTNodeType::OpSubtraction:
    case tiny::ASTNodeType::OpMultiplication:
    case tiny::ASTNodeType::OpDivision:
    case tiny::ASTNodeType::OpExponentiate:
        getActive().parseOperation(node);
        return;
    }

    for (const auto &c: node->children) {
        update(c);
    }
}

tiny::Scope& tiny::SymbolTable::getActive()
{
    tiny::Scope& focus = root;
    while (!focus.inner.empty()) {
        focus = focus.inner.front();
    }

    return focus;
}

void tiny::Scope::parseOperation(const std::shared_ptr<ASTNode> &node)
{
    for (const auto &c: node->children) {
        if (c->type == tiny::ASTNodeType::Identifier) {
            addPromise(tiny::Promise(
                    c->getStringValue(),
                    tiny::Assertion::Exists,
                    c->meta));
            continue;
        }

        // TODO Member access
    }
}

void tiny::Scope::addPromise(tiny::Promise promise) {
    tiny::debug(promise.meta.file, "Promise -> " + promise.toString().toString());

    promises.push_back(std::move(promise));
}

void tiny::Scope::addFulfilment(tiny::Promise fulfilment) {
    tiny::debug(fulfilment.meta.file, "Fulfilment -> " + fulfilment.toString().toString());

    fulfillments.push_back(std::move(fulfilment));
}

tiny::String tiny::Promise::toString() const
{
    switch (assertion) {
    case Assertion::Exists:
        return identifier + " exists";
    case Assertion::HasMember:
        return identifier + " has member " + argument;
    case Assertion::IsIndexable:
        return identifier + " is indexable";
    case Assertion::IsStruct:
        return identifier + " is a struct";
    case Assertion::IsCallable:
        return identifier + " is callable";
    case Assertion::IsNumeric:
        return identifier + " is of a numeric type";
    case Assertion::IsOfType:
        return identifier + " is of type " + argument;
    default:
        return "Invalid promise";
    }
}
