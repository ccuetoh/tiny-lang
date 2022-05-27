#ifndef TINY_SYMTAB_H
#define TINY_SYMTAB_H

#include <utility>

#include "unicode.h"
#include "metadata.h"
#include "ast.h"

namespace tiny {
    enum class Assertion {
        Exists,
        HasMember,
        IsIndexable,
        IsStruct,
        IsCallable,
        IsNumeric, // TODO Utilize
        IsOfType
    };

    struct Promise {
    public:
        Promise(tiny::String identifier, tiny::Assertion assertion, const tiny::Metadata &meta):
        identifier(std::move(identifier)), assertion(assertion), meta(meta) {};

        tiny::String identifier;
        tiny::Assertion assertion;
        tiny::String argument;
        tiny::Metadata meta;

        tiny::String toString() const;
    };

    enum class ScopeType {
        Global,
        NonGlobal
    };

    struct Scope {
    public:
        tiny::ScopeType type = tiny::ScopeType::Global;

        std::vector<tiny::Promise> promises = {};
        std::vector<tiny::Promise> fulfillments = {};

        std::vector<tiny::Scope> inner = {};

        void addPromise(tiny::Promise promise);
        void addFulfilment(tiny::Promise fulfilment);

        void parseOperation(const std::shared_ptr<ASTNode>& node);
    };

    struct SymbolTable {
    public:
        explicit SymbolTable(const tiny::ASTFile &ast): ast(ast) {};

        const tiny::ASTFile &ast;
        tiny::Scope root = {tiny::ScopeType::Global};

        void build();
        void update(const std::shared_ptr<ASTNode> &node);

        [[nodiscard]] tiny::Scope &getActive();
    };
}

#endif //TINY_SYMTAB_H
