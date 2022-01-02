#ifndef TINY_FILE_H
#define TINY_FILE_H

#include <filesystem>
#include <utility>

#include "explorer.h"
#include "errors.h"

namespace tiny {
    enum class FileType {
        Meta,
        Source
    };

    struct File {
        tiny::FileType type = tiny::FileType::Source;
        std::filesystem::path path;
    };

    class FileSelector {
    public:
        explicit FileSelector() = default;
        explicit FileSelector(std::string_view pth): explorer(pth), path(pth) {};
        explicit FileSelector(const std::filesystem::path& pth): explorer(pth), path(pth) {};

        [[nodiscard]] tiny::File getMetaFile() const;
        [[nodiscard]] std::vector<tiny::File> getLocalSourceFiles() const;
        [[nodiscard]] std::vector<tiny::File> getFiles() const;

    private:
        tiny::Explorer explorer = tiny::Explorer(std::filesystem::current_path());
        std::filesystem::path path = std::filesystem::current_path();
    };
}

#endif //TINY_FILE_H
