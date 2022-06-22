#ifndef TINY_SYMTAB_H
#define TINY_SYMTAB_H

#include <utility>

#include "unicode.h"
#include "metadata.h"
#include "ast.h"
#include "errors.h"

namespace tiny {
    enum class Assertion {
        None,
        IsDefined,
        HasMember,
        IsIndexable,
        IsStruct,
        IsCallable,
        CallReturns,
        CallReturnCount,
        CallRequires,
        IsNumeric,
        IsText,
        IsOfType
    };

    struct Promise {
    public:
        Promise(tiny::String identifier, tiny::Assertion assertion, tiny::Metadata meta):
        identifier(std::move(identifier)), assertion(assertion), meta(std::move(meta)) {};

        Promise(tiny::String identifier, tiny::Assertion assertion, tiny::String arg, tiny::Metadata meta):
                identifier(std::move(identifier)), assertion(assertion), argument(std::move(arg)), meta(std::move(meta)) {};

        Promise(tiny::String identifier, tiny::Assertion assertion, tiny::String arg, std::uint32_t pos, tiny::Metadata meta):
                identifier(std::move(identifier)), assertion(assertion), argument(std::move(arg)), position(pos), meta(std::move(meta)) {};

        tiny::String identifier;
        tiny::Assertion assertion;
        tiny::String argument;
        std::uint32_t position;

        tiny::Metadata meta;

        [[nodiscard]] tiny::String toString() const;
    };

    enum class ScopeType {
        Global,
        NonGlobal
    };

    struct Scope {
    public:
        tiny::ScopeType type = tiny::ScopeType::Global;
        tiny::String name;

        std::vector<tiny::Promise> promises = {};
        std::vector<tiny::Promise> fulfillments = {};

        std::vector<tiny::Scope> inner = {};

        void addPromise(tiny::Promise promise);
        void addFulfilment(tiny::Promise fulfilment);
    };

    struct SymbolTable {
    public:
        explicit SymbolTable(const tiny::ASTFile &ast): ast(ast) {};

        const tiny::ASTFile &ast;
        tiny::Scope root = {tiny::ScopeType::Global, "global"};

        void build();
        void update(const std::shared_ptr<ASTNode> &node, const String &withName= "");

        void validate();

        [[nodiscard]] tiny::Scope* getActive();

    private:
        void newInnerScope(const tiny::String &name = "");

        tiny::Assertion parseOperation(const std::shared_ptr<ASTNode>& node, tiny::Assertion upstream);
        void parseFunction(const std::shared_ptr<ASTNode>& shared_ptr);
    };

    class TypeInfo {
    public:
        TypeInfo() = default;
        explicit TypeInfo(tiny::Assertion t): type(t) {};

        [[nodiscard]] tiny::Assertion getType() const
        {
            return type;
        }

        [[nodiscard]] tiny::String getTypeName(const tiny::Metadata &md) const
        {
            switch (type) {
            case Assertion::None:
                return "unknown";
            case Assertion::IsNumeric:
                return "numeric";
            case Assertion::IsText:
                return  "text";
            default:
                throw UnknownTypeError("", md);
            }
        }

        void setType(tiny::Assertion t, const tiny::Metadata &md)
        {
            if (t == tiny::Assertion::None) {
                return;
            }

            if (t != tiny::Assertion::IsText &&t != tiny::Assertion::IsNumeric) {
                throw tiny::InvalidTypeError("The provided assertion is not a type assertion", md);
            }

            if (type != tiny::Assertion::None && type != t) {
                throw tiny::IncompatibleTypesError("", md);
            }

            type = t;
        }

        bool isSet() {
            return type != tiny::Assertion::None;
        }

    private:
        tiny::Assertion type = tiny::Assertion::None;
    };
}

#endif //TINY_SYMTAB_H
