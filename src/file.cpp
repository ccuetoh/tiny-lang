#include "file.h"

#include "errors.h"

tiny::File tiny::FileSelector::getMetaFile() const {
    auto match = explorer.search("tiny.toml");
    if (match.empty()) {
        throw tiny::MetaNotFoundError("Tried to find metafile in '" + path.string() + " but none was found");
    }

    return {tiny::FileType::Meta, match[0]};
}

std::vector<tiny::File> tiny::FileSelector::getLocalSourceFiles() const {
    auto matches = explorer.searchMany({"*.ty"}, {"src"});
    if (matches.empty()) {
        throw tiny::SourcesNotFoundError("No source files found in the current directory");
    }

    std::vector<tiny::File> files;
    files.reserve(matches.size());
    for (auto const &p: matches) {
        files.push_back({tiny::FileType::Source, p});
    }

    return files;
}

std::vector<tiny::File> tiny::FileSelector::getFiles() const {
    auto files = getLocalSourceFiles();
    files.push_back(getMetaFile());

    return files;
}
