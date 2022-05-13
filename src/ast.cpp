#include <fstream>
#include "ast.h"
#include "errors.h"

tiny::ASTNode::ASTNode(tiny::Metadata meta, tiny::ASTNodeType t, const tiny::ASTNode& c1)
        :type(t), meta(std::move(meta))
{
    addChildren(c1);
}

tiny::ASTNode::ASTNode(tiny::Metadata meta, tiny::ASTNodeType t, const tiny::ASTNode& c1, const tiny::ASTNode& c2)
        :type(t), meta(std::move(meta))
{
    addChildren(c1);
    addChildren(c2);
}

tiny::ASTNode::ASTNode(tiny::Metadata meta, tiny::ASTNodeType t, const tiny::ASTNode& c1, const tiny::ASTNode& c2,
        const tiny::ASTNode& c3)
        :type(t), meta(std::move(meta))
{
    addChildren(c1);
    addChildren(c2);
    addChildren(c3);
}

nlohmann::json tiny::ASTFile::toJson() const
{
    std::vector<nlohmann::json> jsonStmts;
    for (auto& s: statements) {
        jsonStmts.push_back(s.toJson());
    }

    std::vector<nlohmann::json> jsonImports;
    for (auto& i: imports) {
        jsonImports.push_back(i.toJson());
    }

    return nlohmann::json{
            {"file", {
                    {"path", filename},
                    {"module", mod.toString()},
                    {"imports", jsonImports},
                    {"statements", jsonStmts}
            }},
    };
}

nlohmann::json tiny::ASTNode::toJson() const
{
    nlohmann::json json;
    json["type"] = toString();

    std::vector<nlohmann::json> childrenJson;
    for (auto& c: children) {
        childrenJson.push_back(c->toJson());
    }

    json["children"] = childrenJson;

    if (auto strVal = tiny::toString(val); !strVal.empty()) {
        json["value"] = strVal;
    }

    if (!params.empty()) {
        std::vector<nlohmann::json> jsonParams;
        for (auto& p: params) {
            jsonParams.push_back(p.toJson());
        }

        json["parameters"] = jsonParams;
    }

    return json;
}

std::string tiny::ASTNode::toString() const
{
    switch (type) {
    case tiny::ASTNodeType::ExpressionList:
        return "ExpressionList";
    case tiny::ASTNodeType::ExpressionStatement:
        return "StatementExpression";
    case tiny::ASTNodeType::BlockStatement:
        return "BlockStatement";

    case tiny::ASTNodeType::LiteralInt:
        return "LiteralInt";
    case tiny::ASTNodeType::LiteralUInt:
        return "LiteralUInt";
    case tiny::ASTNodeType::LiteralDecimal:
        return "LiteralDecimal";
    case tiny::ASTNodeType::LiteralBool:
        return "LiteralBool";
    case tiny::ASTNodeType::LiteralNone:
        return "LiteralNone";
    case tiny::ASTNodeType::LiteralChar:
        return "LiteralChar";
    case tiny::ASTNodeType::LiteralString:
        return "LiteralString";

    case tiny::ASTNodeType::OpAddition:
        return "OpAddition";
    case tiny::ASTNodeType::OpSubtraction:
        return "OpSubtraction";
    case tiny::ASTNodeType::OpMultiplication:
        return "OpMultiplication";
    case tiny::ASTNodeType::OpDivision:
        return "OpDivision";
    case tiny::ASTNodeType::OpExponentiate:
        return "OpExponentiate";

    case tiny::ASTNodeType::Identifier:
        return "Identifier";
    case tiny::ASTNodeType::Initialization:
        return "Initialization";
    case tiny::ASTNodeType::Assignment:
        return "Assignment";
    case tiny::ASTNodeType::AssignmentSum:
        return "AssignmentSum";
    case tiny::ASTNodeType::AssignmentSub:
        return "AssignmentSub";
    case tiny::ASTNodeType::AssignmentMulti:
        return "AssignmentMulti";
    case tiny::ASTNodeType::AssignmentDiv:
        return "AssignmentDiv";
    case tiny::ASTNodeType::VarDeclaration:
        return "VarDeclaration";

    case tiny::ASTNodeType::ForStatement:
        return "ForStatement";
    case tiny::ASTNodeType::RangeExpression:
        return "RangeExpression";
    case tiny::ASTNodeType::RangeFromExpression:
        return "RangeFromExpression";
    case tiny::ASTNodeType::RangeToExpression:
        return "RangeToExpression";
    case tiny::ASTNodeType::RangeStepExpression:
        return "RangeStepExpression";
    case tiny::ASTNodeType::ForEachExpression:
        return "ForEachExpression";

    case tiny::ASTNodeType::IfStatement:
        return "IfStatement";
    case tiny::ASTNodeType::BranchCondition:
        return "BranchCondition";
    case tiny::ASTNodeType::BranchConsequent:
        return "BranchConsequent";
    case tiny::ASTNodeType::BranchAlternative:
        return "BranchAlternative";

    case tiny::ASTNodeType::CompareEq:
        return "CompareEq";
    case tiny::ASTNodeType::CompareNeq:
        return "CompareNeq";
    case tiny::ASTNodeType::CompareGt:
        return "CompareGt";
    case tiny::ASTNodeType::CompareGteq:
        return "CompareGteq";
    case tiny::ASTNodeType::CompareLt:
        return "CompareLt";
    case tiny::ASTNodeType::CompareLteq:
        return "CompareLteq";

    case tiny::ASTNodeType::LogicalAnd:
        return "LogicalAnd";
    case tiny::ASTNodeType::LogicalOr:
        return "LogicalOr";

    case tiny::ASTNodeType::UnaryNot:
        return "UnaryNot";
    case tiny::ASTNodeType::UnaryNegative:
        return "UnaryNegative";

    case tiny::ASTNodeType::ErrorHandle:
        return "ErrorHandle";

    case tiny::ASTNodeType::FunctionDeclaration:
        return "FunctionDeclaration";
    case tiny::ASTNodeType::FunctionArgumentDeclList:
        return "FunctionArgumentsDeclList";
    case tiny::ASTNodeType::FunctionArgumentDecl:
        return "FunctionArgumentsDecl";
    case tiny::ASTNodeType::FunctionReturnDeclList:
        return "FunctionReturnDeclList";
    case tiny::ASTNodeType::FunctionReturnDecl:
        return "FunctionReturnDecl";
    case tiny::ASTNodeType::FunctionBody:
        return "FunctionBody";
    case tiny::ASTNodeType::FunctionReturn:
        return "FunctionReturn";
    case tiny::ASTNodeType::MethodDeclaration:
        return "MethodDeclaration";
    case tiny::ASTNodeType::MethodType:
        return "MethodType";

    case tiny::ASTNodeType::FunctionCall:
        return "FunctionCall";
    case tiny::ASTNodeType::FunctionCallArgumentList:
        return "FunctionCallArgumentList";

    case tiny::ASTNodeType::Type:
        return "Type";
    case tiny::ASTNodeType::TypedExpression:
        return "TypedExpression";

    case tiny::ASTNodeType::MemberAccess:
        return "MemberAccess";
    case tiny::ASTNodeType::IndexedAccess:
        return "IndexedAccess";

    case tiny::ASTNodeType::StructDeclaration:
        return "StructDeclaration";
    case tiny::ASTNodeType::TraitDeclaration:
        return "TraitDeclaration";
    case tiny::ASTNodeType::TraitList:
        return "TraitList";
    case tiny::ASTNodeType::Trait:
        return "Trait";
    case tiny::ASTNodeType::StructField:
        return "StructField";
    case tiny::ASTNodeType::StructFieldList:
        return "StructFieldList";
    case tiny::ASTNodeType::Composition:
        return "Composition";
    case tiny::ASTNodeType::TraitFieldList:
        return "TraitFieldList";

    default:
        return "None";
    }
}

nlohmann::json tiny::Parameter::toJson() const
{
    return nlohmann::json{
            {"type",  toString()},
            {"value", tiny::toString(val)}
    };
}

nlohmann::json tiny::Import::toJson() const
{
    nlohmann::json json{
            {"module", mod.toString()},
    };

    if (!alias) {
        json["alias"] = alias.toString();
    }

    return json;
}

std::string tiny::Parameter::toString() const
{
    switch (type) {
    case tiny::ParameterType::Type:
        return "Type";
    case tiny::ParameterType::Const:
        return "Const";
    case tiny::ParameterType::Pointer:
        return "Pointer";
    case tiny::ParameterType::Dereference:
        return "Dereference";
    case tiny::ParameterType::ValueAt:
        return "ValueAt";
    case tiny::ParameterType::RangeIdentifier:
        return "RangeIdentifier";
    case tiny::ParameterType::ErrorCallback:
        return "ErrorCallback";
    case tiny::ParameterType::ErrorVarName:
        return "ErrorVarName";
    case tiny::ParameterType::Name:
        return "Name";
    case tiny::ParameterType::ComputedAccess:
        return "ComputedAccess";

    default:
        return "None";
    }
}

std::string tiny::toString(tiny::Value val)
{
    if (std::holds_alternative<tiny::String>(val)) {
        return std::get<tiny::String>(val).toString();
    }

    if (std::holds_alternative<std::int64_t>(val)) {
        return std::to_string(std::get<std::int64_t>(val));
    }

    if (std::holds_alternative<std::uint64_t>(val)) {
        return std::to_string(std::get<std::uint64_t>(val));
    }

    if (std::holds_alternative<long double>(val)) {
        return std::to_string(std::get<long double>(val));
    }

    if (std::holds_alternative<bool>(val)) {
        return std::get<bool>(val) ? "True" : "False";
    }

    return ""; // Empty val
}

std::optional<tiny::Parameter> tiny::ASTNode::getParam(tiny::ParameterType t) const
{
    for (const auto& p: params) {
        if (p.type==t) {
            return p;
        }
    }

    return {};
}

void tiny::ASTNode::addParam(const tiny::Parameter& p)
{
    params.push_back(p);
}

std::optional<std::shared_ptr<tiny::ASTNode>> tiny::ASTNode::getChild(tiny::ASTNodeType t) const
{
    for (const auto& c: children) {
        if (c->type==t) {
            return c;
        }
    }

    return {};
}

std::shared_ptr<tiny::ASTNode> tiny::ASTNode::mustGetChild(tiny::ASTNodeType t) const
{
    for (const auto& c: children) {
        if (c->type==t) {
            return c;
        }
    }

    throw tiny::NoSuchChild("Node of type '" + tiny::ASTNode(meta, t).toString() + "' expected but not found", meta);
}

void tiny::ASTNode::addChildren(const tiny::ASTNode& c)
{
    children.push_back(std::make_shared<tiny::ASTNode>(c));
}

void tiny::ASTNode::addChildren(const tiny::StatementList& cs)
{
    for (auto& n: cs) {
        children.push_back(std::make_shared<tiny::ASTNode>(n));
    }
}

std::shared_ptr<tiny::ASTNode> tiny::ASTNode::mustGetLHS() const
{
    if (children.empty()) {
        throw tiny::NoSuchChild("Tried to get the left-most child, but the node has no children", meta);
    }

    return children[0];
}

std::shared_ptr<tiny::ASTNode> tiny::ASTNode::mustGetRHS() const
{
    if (children.size()<2) {
        throw tiny::NoSuchChild("Tried to get the right-most child, but it doesn't exist", meta);
    }

    return children[1];
}

void tiny::ASTFile::dumpJson(const std::filesystem::path& path) const
{
    std::ofstream jsonOut;
    jsonOut.open(path);
    jsonOut << toJson().dump(4);
    jsonOut.close();
}