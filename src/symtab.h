#ifndef TINY_SYMTAB_H
#define TINY_SYMTAB_H

#include <string>
#include <utility>
#include <vector>
#include <deque>
#include <optional>

#include "metadata.h"

namespace tiny {
    enum class SymTabType {
        Variable,
        Struct,
        Trait,
        Module
    };

    struct SymTabEntry {
        SymTabEntry() = default;
        SymTabEntry(const std::string_view &id,
                const std::string_view &name,
                tiny::Metadata meta,
                tiny::SymTabType type=SymTabType::Variable):
                    identifier(id), type_name(name), type(type), meta(std::move(meta)) {};

        std::string identifier;
        std::string type_name;
        tiny::SymTabType type = SymTabType::Variable;
        tiny::Metadata meta;

        bool is_const = false;
        bool is_pointer = false;

        bool operator==(const SymTabEntry &rhs) const {
            return identifier == rhs.identifier;
        };

        bool operator==(const std::string_view &id) const {
            return identifier == id;
        };
    };

    class Scope {
        std::vector<SymTabEntry> entries;

    public:
        [[nodiscard]] std::optional<SymTabEntry> lookup(const std::string_view &id) const;
        void addEntry(const tiny::SymTabEntry &entry);
    };

    class SymbolTable {
        std::deque<Scope> scopes;

    public:
        [[nodiscard]] std::optional<SymTabEntry> lookup(const std::string_view &id) const;
        void enterScope();
        void exitScope();
    };
}

#endif //TINY_SYMTAB_H
