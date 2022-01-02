#ifndef TINY_STREAM_H
#define TINY_STREAM_H

#include <algorithm>
#include <vector>
#include <iterator>

#include "unicode.h"

#include "utf8.h"

namespace tiny {
    //! The underlying type of the stream's data
    template<typename T>
    //! A WalkableStream wraps a vector so it can be accessed sequentially and similar to a stream
    class WalkableStream {
    public:
        /*!
         * \brief Creates an empty stream
         */
        explicit WalkableStream() =  default;

        /*!
         * \brief Use a collection for the creation of the stream and provide a terminator value
         * \param col A vector of items to add to the stream
         * \param terminator An optional terminator to return if the stream's length is exceeded
         */
        explicit WalkableStream(std::vector<T> col, T terminator = T{}) : collection(col), terminator(terminator) {};

        /*!
         * \brief Use a std::stream for the creation of the underlying vector
         * \param stream A stream of UTF-8 encoded characters to add to the WalkableStream
         * \param terminator An optional terminator to return if the stream's length is exceeded
         */
        explicit WalkableStream(std::istream &stream, T terminator = T{}) : terminator(terminator), collection(
                tiny::UnicodeParser::fromStream(stream)) {};

        /*!
         * \brief Check whether the end of the stream has been reached
         * \return True if there's still items in the stream, false otherwise
         */
        explicit operator bool() const {
            return index < collection.size();
        }

        /*!
         * \brief Fetches the next item in the stream and advances it
         * \return The next item if the stream is valid, the terminator otherwise
         *
         * Fetches the next item in the stream and advances its position by one. If the index is over the length of the
         * collection, a terminator value will be returned
         */
        [[nodiscard]] T get() {
            if (index >= collection.size()) {
                return terminator;
            }

            return collection[index++];
        }

        /*!
         * \brief Fetches the next item in the stream without advancing its position
         * \return The next item if the stream is valid, the terminator otherwise
         *
         * Fetches the next item without stepping over it, and keeps the index position the same. If the index is over
         * the length of the collection, a terminator value will be returned
         */
        [[nodiscard]] T peek() const {
            if (index >= collection.size()) {
                return terminator;
            }

            return collection[index];
        }

        //! Goes back one position
        void backup() {
            rewind(1);
        }

        //! Goes forwards one position
        void skip() {
            advance(1);
        }

        /*!
         * \brief Goes a number of positions forward
         * \param i The number of positions to advance
         */
        std::int32_t advance(std::int32_t i) {
            index += i;
            return index;
        }

        /*!
         * \brief Goes a number of positions backwards
         * \param i The number of positions to go back
         */
        std::int32_t rewind(unsigned long i) {
            // Make sure that if the index is over the collection size the rewind starts at the last item.
            if (index > collection.size() && index != 0) {
                index = collection.size() - 1;
            }

            if (index <= i) {
                index = 0; // Prevent overflows
                return index;
            }

            index--;
            return index;
        }

        /*!
         * \brief Set the stream's index
         * \param i The index of the stream
         */
        void seek(unsigned long i) {
            index = i;
        }

        /*!
         * \brief Gets the i-th element on the stream
         * \param i The index to get from
         *
         * Get the i-th element on the stream. If the index is over the length of the collection, a terminator value
         * will be returned
         */
        [[nodiscard]] T get(unsigned long i) const {
            if (i >= collection.size()) {
                return terminator;
            }

            return collection[i];
        }

        /*!
         * \brief Gets the i-th element on the stream
         * \param from The inclusive start of the vector
         * \param to  The exclusive end of the vector
         * \return A vector spanning [from:to[
         *
         * Gets a vector spanning [from:to[. The to parameter is bound-checked, and will return a vector up to the
         * length of the stream.
         */
        [[nodiscard]] std::vector<T> getVector(unsigned long from, unsigned long to) const {
            return std::vector<T>(collection.begin() + from,
                                  collection.begin() + (std::min)(to, (unsigned long) (collection.size())));
        }

        /*!
         * \brief Get a vector as a copy of the internal collection of the stream
         * \return A vector copy of the stream
         */
        [[nodiscard]] std::vector<T> getVector() const {
            return std::vector<T>(collection.begin(), collection.begin() + collection.size());
        }

        /*!
         * \brief Gets the current position of the stream
         * \return The stream's index
         */
        [[nodiscard]] unsigned long getIndex() const {
            return index;
        }

        /*!
         * \brief Gets the terminator value
         * \return The stream's terminator value
         */
        [[nodiscard]] T getTerminator() const {
            return terminator;
        }

        /*!
         * \brief Replaces the terminator value
         * \param t The new terminator value
         */
        void setTerminator(T t) const {
            terminator = t;
        }

        /*!
         * \brief Returns whether the provided value is the terminator
         * \param t A value to compare
         * \return True if it's the terminator, false otherwise
         */
        bool isTerminator(T t) {
            return terminator == t;
        }

        /*!
         * \brief Returns the length of the stream
         * \return The length of the stream
         */
        std::size_t length() {
            return collection.size();
        }

    private:
        //! Internal vector representation of the stream
        std::vector<T> collection;

        //! Current index of the cursor
        unsigned long index = 0;

        //! Terminator value
        T terminator{};
    };
}

#endif //TINY_STREAM_H
