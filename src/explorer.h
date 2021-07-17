#ifndef TINY_EXPLORER_H
#define TINY_EXPLORER_H

#include <string>
#include <utility>
#include <filesystem>
#include <vector>

namespace tiny {

    /*!
     * The Explorer class navigates, finds and filters files. Under the hood it wraps std::filesystem.
     */
    class Explorer {
    public:
        //! Creates an Explorer instance with the current working directory as base path.
        explicit Explorer() : baseDirectory(std::filesystem::current_path().string()) {};

        //! Creates an Explorer instance with the base directory set to the given path.
        explicit Explorer(std::string baseDirectory) : baseDirectory(std::move(baseDirectory)) {};

        /*!
         * Iterates over the set directory and searches for a matching filename. The term might be a full filename
         * like example.txt or an extension wildcard like *.txt. The search will enter folders recursively for the depth
         * set with the setSearchDepth() function (defaults to 1).
         */
        std::vector<std::filesystem::directory_entry> search(const std::string &term);

        //! Executes a search over multiple terms. See the documentation for the 'search' function.
        std::vector<std::filesystem::directory_entry> searchMany(const std::vector<std::string> &terms);

        //! Returns the current search depth.
        [[nodiscard]] int getSearchDepth() const;

        //! Sets the recursive search depth. If 0 is set only the root folder will be examined.
        void setSearchDepth(int depth);

    private:
        //! The root directory for the search.
        std::string baseDirectory;

        //! The maximum allowed search depth. Defaults to 1.
        int searchDepth = 1;

    };
}

#endif //TINY_EXPLORER_H
