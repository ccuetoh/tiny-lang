#include "symtab.h"
#include "errors.h"

std::optional<tiny::SymTabEntry> tiny::SymbolTable::lookup(const std::string_view& id) const
{
    for (auto const &scope: scopes) {
        auto const &result = scope.lookup(id);
        if (result.has_value()) {
            return result;
        }
    }

    return {};
}

void tiny::SymbolTable::enterScope()
{
    scopes.emplace_front();
}

void tiny::SymbolTable::exitScope()
{
    scopes.emplace_front();
}

std::optional<tiny::SymTabEntry> tiny::Scope::lookup(const std::string_view& id) const
{
    for (auto const& entry: entries) {
        if (entry == id) {
            return entry;
        }
    }

    return {};
}

void tiny::Scope::addEntry(const tiny::SymTabEntry &entry)
{
    if (auto const& exists = lookup(entry.identifier);
    exists.has_value() && exists.value().type == tiny::SymTabType::Module) {
        throw tiny::IlegalRedefinitionError("Module names can't be reused", entry.meta); // TODO Clearer message?
    }

    entries.push_back(entry);
}