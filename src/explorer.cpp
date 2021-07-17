#include "explorer.h"

int tiny::Explorer::getSearchDepth() const {
    return searchDepth;
}

void tiny::Explorer::setSearchDepth(int depth) {
    Explorer::searchDepth = depth;
}

std::vector<std::filesystem::directory_entry> tiny::Explorer::search(const std::string &term) {
    return searchMany(std::vector<std::string>{term});
}

std::vector<std::filesystem::directory_entry> tiny::Explorer::searchMany(const std::vector<std::string> &terms) {
    // Prepare filetype wildcards (ex. *.txt -> txt)
    std::vector<std::string> extensions;
    for (const auto &term : terms) {
        if (term.length() < 3) {
            continue;
        }

        if (term.substr(0, 2) == "*.") {
            extensions.push_back(term.substr(1, term.length()));
        }
    }

    // Iterate over the file tree
    std::vector<std::filesystem::directory_entry> matches;
    auto i = std::filesystem::recursive_directory_iterator(baseDirectory);
    for (auto &p: i) {
        if (i.depth() > searchDepth) {
            continue;
        }

        if (!p.is_regular_file()) {
            continue;
        }

        // Try to match the extension
        for (auto &ext: extensions) {
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
