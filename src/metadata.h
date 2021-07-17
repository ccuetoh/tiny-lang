#ifndef TINY_METADATA_H
#define TINY_METADATA_H

#include "stream.h"
#include "unicode.h"

namespace tiny {
//! The Metadata struct contains information regarding the character's stream information, and it's used for debugging and
//! error logging.
    struct Metadata {
        //! Empty constructor for the metadata
        Metadata() = default;

        //! Full constructor for the metadata
        explicit Metadata(std::string_view fn, int p) : filename(fn), pos(p) {};

        //! Filename from which the character proceeds from
        std::string filename;

        //! Index of the stream's current position
        int pos = 0;

        //! Returns the position of the metadata index as a line-column pair
        [[nodiscard]] std::pair<int, int> getPosition(tiny::WalkableStream<uint32_t> &s) const;

        //! Returns a pair containing the strings around the error, cutoff by newlines and maxed at range and
        //! the position on which the error was generated inside the context.
        [[nodiscard]] std::pair<std::string, int> getContext(tiny::WalkableStream<uint32_t> &s, int range = 100) const;
    };
}

#endif //TINY_METADATA_H
