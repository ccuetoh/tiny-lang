//
// Created by Camilo Hernández on 20-06-2021.
//

#include "gtest/gtest.h"

#include "stream.h"

TEST(WalkableStream, StreamOperations) {
    std::vector<std::int32_t> vec = {1, 2, 3, 4, 5};
    auto ws = tiny::WalkableStream(vec);

    ASSERT_EQ(ws.get(), 1);
    ASSERT_EQ(ws.peek(), 2);
    ASSERT_EQ(ws.get(), 2);

    ws.seek(0);
    ASSERT_EQ(ws.get(), 1);

    ws.backup();
    ASSERT_EQ(ws.peek(), 1);

    ws.skip();
    ASSERT_EQ(ws.peek(), 2);

    ws.advance(50);
    ASSERT_EQ(ws.get(), 0);
    ASSERT_EQ(ws.peek(), 0);

    ws.backup();
    ws.backup();
    ASSERT_EQ(ws.peek(), 3);
}

TEST(WalkableStream, StreamLargeVector) {
    std::vector<long> vec;
    for (long i = 0; i < 9999999; i++) {
        vec.push_back(i);
    }

    auto ws = tiny::WalkableStream(vec);

    for (long i = 0; i < 9999999; i++) {
        ASSERT_EQ(ws.get(), i);
    }
}