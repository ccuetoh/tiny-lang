#ifndef TINY_FILE_H
#define TINY_FILE_H

#include <filesystem>
#include <utility>

#include "explorer.h"
#include "errors.h"

namespace tiny {
    //! Represents whether the file is a metadata file or a source-code file
    enum class FileType {
        //! Metadata file
        Meta,
        //! Source-code file
        Source
    };

    //! Represents a file (but doesn't actually holds it), and contains its path and type
    struct File {
        tiny::FileType type = tiny::FileType::Source;
        std::filesystem::path path;
    };

    //! A FileSelector finds the metadata and source files to be used for the compiler
    class FileSelector {
    public:
        //! Builds a FileSelector using the current working directory
        explicit FileSelector() = default;

        //! Builds a FileSelector in the provided path's string representation
        explicit FileSelector(std::string_view pth) : explorer(pth), path(pth) {};

        //! Builds a FileSelector in the provided path
        explicit FileSelector(const std::filesystem::path &pth) : explorer(pth), path(pth) {};

        /*!
         * \brief Tries to find the metadata file (tiny.toml). If none is found MetaNotFoundError is raised
         * \return A File object detailing the found file
         *
         * Tries to find the metadata file (tiny.toml). If none is found MetaNotFoundError is raised. It searches in
         * the current folder, without diving inside children directories
         */
        [[nodiscard]] tiny::File getMetaFile() const;

        /*!
         * \brief Tries to find the source files (*.ty). If none is found SourcesNotFoundError is raised
         * \return A vector of File detailing the found sources
         *
         * Tries to find the source files (*.ty). If none is found SourcesNotFoundError is raised. It searches inside
         * the current directory and inside a "src" folder if present.
         */
        [[nodiscard]] std::vector<tiny::File> getLocalSourceFiles() const;

        /*!
         * \brief Tries to find the source files (*.ty) and a metadata file (tiny.toml)
         * \return A vector of File detailing the found sources and metadata files
         *
         * Tries to find the source files (*.ty) and a metadata file (tiny.toml). If no metadata file is found,
         * MetaNotFoundError is raised. If no sources are found, SourcesNotFoundError is raised.
         */
        [[nodiscard]] std::vector<tiny::File> getFiles() const;

    private:
        //! The Explorer used to search for the files
        tiny::Explorer explorer = tiny::Explorer(std::filesystem::current_path());
        //! Current search path. Defaults to the current path.
        std::filesystem::path path = std::filesystem::current_path();
    };
}

#endif //TINY_FILE_H
