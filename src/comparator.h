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
        //! Create a comparator from a WalkableStream.
        explicit StreamComparator(tiny::WalkableStream<T> &stream) : s(stream) {};

        //! Compares the sequence with the stream. The stream's index will advance the length of the sequence.
        //! If the sequence's length is shorter than the stream, only the it's length will be
        //! compared ([1, 2] will match [1, 2, 3]). It will always return false on an empty sequence.
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
        template<int N>
        //! Shorthand for calling compare with an array
        [[nodiscard]] bool compare(T (&seq)[N]) {
            return compare(std::vector(seq, seq + N));
        }

        //! Compares the sequence with the stream without changing it's index will not be changed. If the sequence's
        //! length is shorter than the stream, only the it's length will be compared ([1, 2] will match [1, 2, 3]).
        //! It will always return false on an empty sequence.
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
        template<int N>
        //! Shorthand for calling comparePeek with an array
        [[nodiscard]] bool comparePeek(T (&seq)[N]) {
            return comparePeek(std::vector(seq, seq + N));
        }

        //! The resulting value of the tuple of the matched case
        template<typename Result>
        //! Compares each of the std::vector<T> sequences and returns the matched case's value. If more than one case matched the first
        //! case will be returned. The index of the stream will not be modified. If a default (def) value is provided
        //! I'll be returned when no case matches. If no default value is provided the 0 value of the type will be
        //! returned. The return value is packed as a tuple of std::vector<T>, Result
        [[nodiscard]] std::pair<std::vector<T>, Result>
        matchTuple(std::map<std::vector<T>, Result> cases, Result def = Result{}) {
            int maxLength = 0;
            for (auto const&[seq, _]: cases) {
                if (seq.size() > maxLength) {
                    maxLength = seq.size();
                }
            }

            int start = s.getIndex();
            int end = start + maxLength;

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
        //! Compares each of the std::vector<T> sequences and returns the matched case's value. If more than one case matched the first
        //! case will be returned. The index of the stream will not be modified. If a default (def) value is provided
        //! I'll be returned when no case matches. If no default value is provided the 0 value of the type will be
        //! returned.
        [[nodiscard]] Result matchPeek(std::map<std::vector<T>, Result> cases, Result def = Result{}) {
            return matchTuple(cases, def).second;
        }

        //! The resulting value of the matched case
        template<typename Result>
        //! Compares each of the std::vector<T> sequences and returns the matched case's value. If more than one case matched the first
        //! case will be returned. The index of the stream will advance the length of the matched sequence. If a
        //! default (def) value is provided I'll be returned when no case matches. If no default value is provided
        //! the 0 value of the type will be returned.
        [[nodiscard]] Result match(std::map<std::vector<T>, Result> cases, Result def = Result{}) {
            auto matched = matchTuple(cases, def);
            s.advance(std::get<0>(matched).size());

            return matched.second;
        }

    private:
        //! Stream to compare
        WalkableStream <T> &s;
    };
}

#endif //TINY_COMPARATOR_H
