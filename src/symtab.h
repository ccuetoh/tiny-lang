#ifndef TINY_SYMTAB_H
#define TINY_SYMTAB_H

#include <string>
#include <vector>

namespace tiny {
    struct SymTabEntry {
        SymTabEntry() = default;
        SymTabEntry(std::string_view id, std::string_view type): identifier(id), type(type) {};

        std::string identifier;
        std::string type;

        bool is_const = false;
        bool is_pointer = false;

        bool operator==(const SymTabEntry &rhs) const {
            return identifier == rhs.identifier;
        };

        bool operator==(const std::string &id) const {
            return identifier == id;
        };
    };

    class Context {
        std::vector<SymTabEntry> entries;
    };

    class SymbolTable {
        
    };
}

#endif //TINY_SYMTAB_H
