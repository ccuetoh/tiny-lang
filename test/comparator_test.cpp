#include "gtest/gtest.h"

#include <map>

#include "stream.h"
#include "comparator.h"

TEST(StreamComparator, CompareEqual) {
    std::vector<std::int32_t> vec = {1, 2, 3, 4, 5};
    auto ws = tiny::Stream(vec);

    auto comp = tiny::StreamComparator(ws);

    ASSERT_TRUE(comp.compare({1, 2, 3, 4, 5}));
}

TEST(StreamComparator, ComparePeek) {
    std::vector<std::int32_t> vec = {1, 2, 3, 4, 5};
    auto ws = tiny::Stream(vec);

    auto comp = tiny::StreamComparator(ws);

    ASSERT_TRUE(comp.comparePeek({1, 2, 3, 4, 5}));
    ASSERT_EQ(ws.getIndex(), 0);
}

TEST(StreamComparator, CompareUnequal) {
    std::vector<std::int32_t> vec = {1, 2, 3, 4, 5};
    auto ws = tiny::Stream(vec);

    auto comp = tiny::StreamComparator(ws);

    ASSERT_FALSE(comp.compare({1, 3, 2, 4, 5}));
}

TEST(StreamComparator, CompareDiferentLengths) {
    std::vector<std::int32_t> vec = {1, 2, 3, 4, 5};
    auto ws = tiny::Stream(vec);

    auto comp = tiny::StreamComparator(ws);

    ASSERT_TRUE(comp.compare({1}));
}

TEST(StreamComparator, CompareNot0Index) {
    std::vector<std::int32_t> vec = {1, 2, 3, 4, 5};
    auto ws = tiny::Stream(vec);

    ws.skip();

    auto comp = tiny::StreamComparator(ws);

    ASSERT_FALSE(comp.compare({1}));
}

TEST(StreamComparator, Match) {
    std::vector<std::int32_t> vec = {1, 2, 3, 4, 5};

    auto ws = tiny::Stream(vec);
    auto comp = tiny::StreamComparator(ws);

    std::map<std::vector<std::int32_t>, std::int32_t> seq{
            {{1, 1}, 1},
            {{2},    2},
            {{1, 2}, 3},
    };

    ASSERT_EQ(comp.match(seq), 3);
    ASSERT_EQ(ws.getIndex(), 2);
}

TEST(StreamComparator, NoMatch) {
    std::vector<std::int32_t> vec = {1, 2, 3, 4, 5};

    auto ws = tiny::Stream(vec);
    auto comp = tiny::StreamComparator(ws);

    std::map<std::vector<std::int32_t>, std::int32_t> seq{
            {{1, 1}, 1},
            {{2},    2},
            {{1, 3}, 3},
    };

    ASSERT_EQ(comp.match(seq), 0);
    ASSERT_EQ(ws.getIndex(), 0);
}

TEST(StreamComparator, NoMatchWithDefault) {
    std::vector<std::int32_t> vec = {1, 2, 3, 4, 5};

    auto ws = tiny::Stream(vec);
    auto comp = tiny::StreamComparator(ws);

    std::map<std::vector<std::int32_t>, std::int32_t> seq{
            {{1, 1}, 1},
            {{2},    2},
            {{1, 3}, 3},
    };

    ASSERT_EQ(comp.match(seq, -1), -1);
    ASSERT_EQ(ws.getIndex(), 0);
}

TEST(StreamComparator, MatchPeek) {
    std::vector<std::int32_t> vec = {1, 2, 3, 4, 5};

    auto ws = tiny::Stream(vec);
    auto comp = tiny::StreamComparator(ws);

    std::map<std::vector<std::int32_t>, std::int32_t> seq{
            {{1, 1}, 1},
            {{2},    2},
            {{1, 2}, 3},
    };

    ASSERT_EQ(comp.matchPeek(seq), 3);
    ASSERT_EQ(ws.getIndex(), 0);
}
