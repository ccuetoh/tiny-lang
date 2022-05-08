#ifndef TINY_SYMTAB_H
#define TINY_SYMTAB_H

#include <string>
#include <utility>
#include <vector>
#include <deque>
#include <optional>

#include "metadata.h"

namespace tiny {
    //! The posible value types stored inside the symbol table
    enum class SymTabType {
        Variable,
        Struct,
        Trait,
        Module
    };

    //! A single entry inside the symbol table
    struct SymTabEntry {
        //! Default constructor
        SymTabEntry() = default;

        //! Full constructor
        SymTabEntry(tiny::String id,
                tiny::String name,
                tiny::Metadata meta,
                tiny::SymTabType type=SymTabType::Variable):
                    identifier(std::move(id)),
                    type_name(std::move(name)),
                    type(type),
                    meta(std::move(meta)) {};

        //! Holds the name that identifies this declared value or type
        tiny::String identifier;

        //! A string representation of the declared type
        tiny::String type_name;

        //! The category of the declaration
        tiny::SymTabType type = SymTabType::Variable;

        //! Metadata of the declaration for logging and debugging
        tiny::Metadata meta;

        //! Whether the declaration is constant or not
        bool is_const = false;

        //! Whether the declaration is of a pointer type or not
        bool is_pointer = false;

        /*!
         * When a declaration is marked as virtual it means that it was used but not found, and it's possible that
         * it might be present on another file or module. Virtual declarations are then checked against other the system
         * tables created in other files.
         */
        bool is_virtual = false;

        //! Compares two entries by their identifier
        bool operator==(const SymTabEntry &rhs) const {
            return identifier == rhs.identifier;
        };

        //! Compares this entry to an identifier
        bool operator==(const tiny::String &id) const {
            return identifier == id;
        };
    };

    //! The entries inside the symbol table for a given scope or context inside the source program
    class Scope {
        //! The entries inside this scope
        std::vector<SymTabEntry> entries;

    public:
        //! Whether this is the global scope or not
        bool is_global = false;

        /*!
        * \brief Searches for an entry inside this scope
        * \param id The identifier to search for
        * \return An optional value that, if the identifier was matched, contains the entry and is empty otherwise
        */
        [[nodiscard]] std::optional<SymTabEntry> lookup(const tiny::String& id) const;

        /*!
        * \brief Adds an entry to the entries list of the scope. Fails if a module declaration with the same id exists
        * \param entry The entry to add
        */
        void addEntry(const tiny::SymTabEntry &entry);
    };

    /*! A symbol table contains a series of scopes that in turn contain entries pertaining to the declarations made by
     *  the source code for each step of the program. It's used for type and name checking in semantic analysis
     *
     *  \brief Holds scopes and their respective declarations for types and variables
     */
    class SymbolTable {
        //! The scopes inside the table
        std::deque<Scope> scopes;

    public:
        /*!
        * \brief Searches for an entry inside the scopes, prioritizing closer scopes first
        * \param id The identifier to search for
        * \return An optional value that, if the identifier was matched, contains the entry and is empty otherwise
        */
        [[nodiscard]] std::optional<SymTabEntry> lookup(const tiny::String& id) const;

        /*!
        * \brief Adds an entry to current scope. Fails if a module declaration with the same id exists
        * \param entry The entry to add
        */
        void addEntry(const tiny::SymTabEntry &entry);

        //! Creates a new scope
        void enterScope();

        //! Deletes the current scope. Does nothing if the current scope is global
        void exitScope();
    };
}

#endif //TINY_SYMTAB_H
