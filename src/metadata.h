#ifndef TINY_METADATA_H
#define TINY_METADATA_H

#include "stream.h"
#include "unicode.h"
#include "file.h"

namespace tiny {
    //! The Metadata struct contains information regarding the character's stream information
    struct Metadata {
        //! Empty constructor for the metadata
        Metadata() = default;

        /*!
         * \brief Constructor with the start and end positions
         * \param fn The file
         * \param startPos The index at which the metadata points
         * \param endPos The index at which the metadata stops
         */
        explicit Metadata(const tiny::File &f, std::uint64_t startPos, std::uint64_t endPos) :file(f),
                                                                                       start(startPos),
                                                                                       end(endPos) {};

        //! File from which the character proceeds from
        tiny::File file;

        //! Index of the token start
        std::uint64_t start = 0;

        //! Index of the token end
        std::uint64_t end = 0;

        /*!
         * \brief Returns the position of the metadata inside the provided stream [line, column] pair
         * \param s Stream in which the position is calculated
         * \return A [line, column] index pair
         */
        [[nodiscard]] std::pair<std::uint64_t, std::uint64_t> getPosition(tiny::Stream<std::uint32_t> &s) const;

        /*!
         * \brief Returns the context around the error and the position of the error in the context
         * \param s Stream in which the error was generated
         * \param range Optional maximum length of the context. Defaults to 100
         * \return A [context, error position] pair
         *
         * Returns a pair containing the strings around the error, cutoff by newlines and maxed by the provided range,
         * and the position on which the error was generated inside the context. The context is generated around the
         * error position. So a maximum of 2/range - len(error) characters will be to either side of the error string.
         */
        [[nodiscard]] std::pair<std::string, std::int32_t>
        getContext(tiny::Stream<std::uint32_t> &s, std::int32_t range = 100) const;

        /*!
         * \brief Returns the length between the start and end positions
         * \return The difference between the end and start positions. If the end is not set then 1 is returned.
         */
        [[nodiscard]] std::int32_t getLength() const;
    };
}

#endif //TINY_METADATA_H
