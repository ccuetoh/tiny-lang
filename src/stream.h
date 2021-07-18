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
        //! Use a collection for the creation of the stream and provide a terminator value
        explicit WalkableStream(std::vector<T> col, T terminator = T{}) : collection(col), terminator(terminator) {};

        //! Use a std::stream for the creation of the underlying vector.
        explicit WalkableStream(std::istream &stream, T terminator = T{}) : terminator(terminator), collection(
                tiny::UnicodeParser::FromStream(stream)) {};

        //! Check whether the end of the stream has been reached
        explicit operator bool() const {
            return index < collection.size();
        }

        //! Fetches the next item in the stream. If the index is over the length of the collection, a terminator value will
        //! be returned
        [[nodiscard]] T get() {
            if (index >= collection.size()) {
                return terminator;
            }

            return collection[index++];
        }

        //! Fetches the next item without stepping over it, and keeps the index position the same. If the index is over
        //! the length of the collection, a terminator value will be returned
        [[nodiscard]] T peek() {
            if (index >= collection.size()) {
                return terminator;
            }

            return collection[index];
        }

        //! Go back one position
        void backup() {
            rewind(1);
        }

        //! Go forwards one position
        void skip() {
            advance(1);
        }

        //! Go i positions forward
        int advance(int i) {
            index += i;
            return index;
        }

        //! Go i positions backwards
        int rewind(unsigned long i) {
            // Make sure that if the index is over the collection size the rewind starts at the last item.
            if (index > collection.size()) {
                index = collection.size() - 1;
            }

            index = std::max(0ul, index - i);
            return index;
        }

        //! Set the index to i
        void seek(unsigned long i) {
            index = i;
        }

        //! Get the i-th element on the stream. If the index is over the length of the collection,
        //! a terminator value will be returned
        [[nodiscard]] T get(unsigned long i) {
            if (index >= collection.size()) {
                return terminator;
            }

            return collection[i];
        }

        //! Get a vector spanning [from:to[. The to parameter is bound-checked, and will return a vector up to the length
        //! of the stream.
        [[nodiscard]] std::vector<T> getVector(unsigned long from, unsigned long to) {
            return std::vector<T>(collection.begin() + from, collection.begin() + (std::min)(to, (unsigned long)(collection.size())));
        }

        //! Gets the current position
        [[nodiscard]] unsigned long getIndex() {
            return index;
        }

        //! Gets the terminator value
        [[nodiscard]] T getTerminator() {
            return terminator;
        }

        //! Sets the terminator to a new value
        void setTerminator(T t) {
            return t;
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
