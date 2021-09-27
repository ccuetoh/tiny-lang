#ifndef TINY_COMPARATOR_H
#define TINY_COMPARATOR_H

#include <vector>
#include <tuple>

#include "stream.h"

namespace tiny {
    //! The underlying type of the WalkableStream and the sequences.
    template<typename T>
    //! A StreamComparator helps to compare a WalkableStream to an array of values.
    class StreamComparator {
    public:
        /*!
         * \brief Create a comparator from a WalkableStream
         * \param stream Stream to compare to
         */
        explicit StreamComparator(tiny::WalkableStream<T> &stream) : s(stream) {};

        /*!
         * \brief Compares the sequence with the stream
         * \param seq Sequence of values to compare to the stream
         * \return True if the sequence matches or false if the sequence is empty or doesn't match
         *
         * Compares the sequence with the stream. The stream's index will advance the length of the sequence.
         * If the sequence's length is shorter than the stream, only the it's length will be
         * compared ([1, 2] will match [1, 2, 3]). It will always return false on an empty sequence.
         */
        [[nodiscard]] bool compare(std::vector<T> seq) {
            if (seq.empty()) {
                return false;
            }

            for (T val: seq) {
                if (val != s.get()) {
                    return false;
                }
            }

            return true;
        }

        //! Size of the array
        template<std::int32_t N>
        /*!
         * \brief Shorthand for calling compare with an array
         * \param seq Sequence to compare to as an array of values
         * \return True if the sequence matches or false if the sequence is empty or doesn't match
         */
        [[nodiscard]] bool compare(T (&seq)[N]) {
            return compare(std::vector(seq, seq + N));
        }

        /*!
         * \brief Compares the sequence with the stream without changing it's index
         * \param seq Sequence of values to compare to the stream
         * \return True if the sequence matches or false if the sequence is empty or doesn't match
         *
         * Compares the sequence with the stream without changing it's index. If the sequence's
         * length is shorter than the stream, only the it's length will be compared ([1, 2] will match [1, 2, 3]).
         * It will always return false on an empty sequence.
         */
        [[nodiscard]] bool comparePeek(std::vector<T> seq) {
            if (seq.empty()) {
                return false;
            }

            auto i = s.getIndex();

            for (T val: seq) {
                if (val != s.get(i)) {
                    return false;
                }

                i++;
            }

            return true;
        }

        //! Size of the array
        template<std::int32_t N>
        /*!
         * \brief Shorthand for calling comparePeek with an array
         * \return True if the sequence matches or false if the sequence is empty or doesn't match
         */
        [[nodiscard]] bool comparePeek(T (&seq)[N]) {
            return comparePeek(std::vector(seq, seq + N));
        }

        //! The resulting value of the pair of the matched case
        template<typename Result>
        /*!
         * \brief Compares each of the std::vector<T> sequences and returns the matched case's value
         * \param cases A std::map of possible cases
         * \param def The value to return if no case matches. By default it gets set to the default constructor of the type
         * \return The matched case's value. If none matches def is returned
         *
         * Compares each of the std::vector<T> sequences and returns the matched case's value. If more than one case matched the first
         * case will be returned. The index of the stream will not be modified. If a default (def) value is provided
         * I'll be returned when no case matches. If no default value is provided the 0 value of the type will be
         * returned. The return value is packed as a std::pair<std::vector<T>, Result>
         */
        [[nodiscard]] std::pair<std::vector<T>, Result>
        matchCase(std::map<std::vector<T>, Result> cases, Result def = Result{}) {
            std::int32_t maxLength = 0;
            for (auto const&[seq, _]: cases) {
                if (seq.size() > maxLength) {
                    maxLength = seq.size();
                }
            }

            unsigned long start = s.getIndex();
            unsigned long  end = start + maxLength;

            while (end > start) {
                auto seq = s.getVector(start, end);
                if (seq.size() < (end - start)) {
                    // Adjust if the stream is shorter than expected because it's near the end
                    end = start + seq.size();
                }

                auto res = cases.find(seq);
                if (res != cases.end()) {
                    return std::make_pair(res->first, res->second);
                }

                end--;
            }

            return std::make_pair(std::vector<T>(), def);
        }

        //! The resulting value of the matched case
        template<typename Result>
        /*!
         * \brief Compares each of the std::vector<T> sequences and returns the matched case's value without changing the position of the stream
         * \param cases A std::map of possible cases
         * \param def The value to return if no case matches. By default it gets set to the default constructor of the type
         * \return The matched case's value. If none matches def is returned
         *
         * Compares each of the std::vector<T> sequences and returns the matched case's value without changing the
         * position of the stream. If more than one case matched the first case will be returned. If a default (def)
         * value is provided it will be returned when no case matches. If no default value is provided the
         * zero-value of the type will be returned.
         */
        [[nodiscard]] Result matchPeek(std::map<std::vector<T>, Result> cases, Result def = Result{}) {
            return matchCase(cases, def).second;
        }

        //! The resulting value of the matched case
        template<typename Result>
        /*!
         * \brief Compares each of the std::vector<T> sequences and returns the matched case's value
         * \param cases A std::map of possible cases
         * \param def The value to return if no case matches. By default it gets set to the default constructor of the type
         * \return The matched case's value. If none matches def is returned
         *
         * Compares each of the std::vector<T> sequences and returns the matched case's value. If more than one case matched the first
         * case will be returned. The index of the stream will advance the length of the matched sequence. If a
         * default (def) value is provided I'll be returned when no case matches. If no default value is provided
         * the zero-value of the type will be returned.
         */
        [[nodiscard]] Result match(std::map<std::vector<T>, Result> cases, Result def = Result{}) {
            auto matched = matchCase(cases, def);
            s.advance(std::get<0>(matched).size());

            return matched.second;
        }

    private:
        //! Stream to compare
        WalkableStream <T> &s;
    };
}

#endif //TINY_COMPARATOR_H
