//
// Created by Camilo Hernández on 22-06-2021.
//

#include "gtest/gtest.h"

#include <map>

#include "stream.h"
#include "comparator.h"

TEST(StreamComparator, CompareEqual) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto ws = tiny::WalkableStream(vec);

    auto comp = tiny::StreamComparator(ws);
    int seq[] = {1, 2, 3, 4, 5};

    ASSERT_TRUE(comp.compare(seq));
}

TEST(StreamComparator, ComparePeek) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto ws = tiny::WalkableStream(vec);

    auto comp = tiny::StreamComparator(ws);
    int seq[] = {1, 2, 3, 4, 5};

    ASSERT_TRUE(comp.comparePeek(seq));
    ASSERT_EQ(ws.getIndex(), 0);
}

TEST(StreamComparator, CompareUnequal) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto ws = tiny::WalkableStream(vec);

    auto comp = tiny::StreamComparator(ws);
    int seq[] = {1, 3, 2, 4, 5};

    ASSERT_FALSE(comp.compare(seq));
}

TEST(StreamComparator, CompareDiferentLengths) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto ws = tiny::WalkableStream(vec);

    auto comp = tiny::StreamComparator(ws);
    int seq[] = {1};

    ASSERT_TRUE(comp.compare(seq));
}

TEST(StreamComparator, CompareNot0Index) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto ws = tiny::WalkableStream(vec);

    ws.skip();

    auto comp = tiny::StreamComparator(ws);
    int seq[] = {1};

    ASSERT_FALSE(comp.compare(seq));
}

TEST(StreamComparator, Match) {
    std::vector<int> vec = {1, 2, 3, 4, 5};

    auto ws = tiny::WalkableStream(vec);
    auto comp = tiny::StreamComparator(ws);

    std::map<std::vector<int>, int> seq{
            {{1, 1}, 1},
            {{2},    2},
            {{1, 2}, 3},
    };

    ASSERT_EQ(comp.match(seq), 3);
    ASSERT_EQ(ws.getIndex(), 2);
}

TEST(StreamComparator, NoMatch) {
    std::vector<int> vec = {1, 2, 3, 4, 5};

    auto ws = tiny::WalkableStream(vec);
    auto comp = tiny::StreamComparator(ws);

    std::map<std::vector<int>, int> seq{
            {{1, 1}, 1},
            {{2},    2},
            {{1, 3}, 3},
    };

    ASSERT_EQ(comp.match(seq), 0);
    ASSERT_EQ(ws.getIndex(), 0);
}

TEST(StreamComparator, NoMatchWithDefault) {
    std::vector<int> vec = {1, 2, 3, 4, 5};

    auto ws = tiny::WalkableStream(vec);
    auto comp = tiny::StreamComparator(ws);

    std::map<std::vector<int>, int> seq{
            {{1, 1}, 1},
            {{2},    2},
            {{1, 3}, 3},
    };

    ASSERT_EQ(comp.match(seq, -1), -1);
    ASSERT_EQ(ws.getIndex(), 0);
}

TEST(StreamComparator, MatchPeek) {
    std::vector<int> vec = {1, 2, 3, 4, 5};

    auto ws = tiny::WalkableStream(vec);
    auto comp = tiny::StreamComparator(ws);

    std::map<std::vector<int>, int> seq{
            {{1, 1}, 1},
            {{2},    2},
            {{1, 2}, 3},
    };

    ASSERT_EQ(comp.matchPeek(seq), 3);
    ASSERT_EQ(ws.getIndex(), 0);
}
