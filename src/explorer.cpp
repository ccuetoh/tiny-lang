#include "explorer.h"

#include <algorithm>

std::int32_t tiny::Explorer::getSearchDepth() const {
    return searchDepth;
}

void tiny::Explorer::setSearchDepth(std::int32_t depth) {
    searchDepth = depth;
}

std::vector<std::filesystem::directory_entry> tiny::Explorer::search(const std::string &term) const {
    return searchMany({term});
}

std::vector<std::filesystem::directory_entry> tiny::Explorer::searchMany(const std::vector<std::string> &terms,
                                                                         const std::vector<std::string> &folders) const {
    // Prepare filetype wildcards (ex. *.txt -> txt)
    std::vector<std::string> extensions;
    for (const auto &term: terms) {
        if (term.length() < 3) {
            continue;
        }

        if (term.substr(0, 2) == "*.") {
            extensions.push_back(term.substr(1, term.length()));
        }
    }

    // Iterate over the file tree
    std::vector<std::filesystem::directory_entry> matches;
    auto i = std::filesystem::recursive_directory_iterator(path);
    for (auto &p: i) {
        if (i.depth() > searchDepth) {
            continue;
        }

        if (!p.is_regular_file()) {
            continue;
        }

        if (!folders.empty() && i.depth() > 1) {
            // Folders whitelist

            auto parent = std::filesystem::path(p).parent_path();
            if (std::find(folders.begin(), folders.end(), parent) == folders.end()) {
                continue; // Not in whitelist
            }
        }

        // Try to match the extension
        for (auto const &ext: extensions) {
            if (p.path().extension() == ext) {
                matches.push_back(p);
                break;
            }
        }

        // Try to match full filenames
        for (auto &term: terms) {
            if (p.path().filename() == term) {
                matches.push_back(p);
                break;
            }
        }
    }

    return matches;
}
