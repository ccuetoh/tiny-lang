#include "gtest/gtest.h"

#include "explorer.h"

#include <fstream>
#include <unordered_set>

const auto basePath = std::filesystem::current_path() / "sandbox";
const auto innerPath = basePath / "inner";

TEST(Explorer, SandboxSetup) {
    // Setup the sandbox
    std::filesystem::create_directory(basePath);
    std::ofstream file1(basePath / "test1.txt");
    std::ofstream file2(basePath / "test2.txt");
    std::ofstream file3(basePath / "test3.txt");
    std::ofstream file4(basePath / "test4.tst");
    std::ofstream file5(basePath / "test5.tst");
    std::ofstream file6(basePath / "te st6.tst"); // Spaced filename
    std::ofstream file7(basePath / "teñst7.tst"); // Non-ASCII filename

    std::filesystem::create_directory(innerPath);
    std::ofstream file8(innerPath / "test1_inner.txt");
    std::ofstream file9(innerPath / "test2_inner.txt");

    std::cout << "File Explorer Sandbox path is: " << basePath << std::endl;
}

TEST(Explorer, SearchNoRecursion) {
    tiny::Explorer explorer(basePath.string());
    explorer.setSearchDepth(0);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto &file: explorer.search("test1.txt")) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{"test1.txt"};

    ASSERT_EQ(got, expect);
}

TEST(Explorer, SearchNotFound) {
    tiny::Explorer explorer(basePath.string());
    explorer.setSearchDepth(0);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto &file: explorer.search("does_not_exist.txt")) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{};

    ASSERT_EQ(got, expect);
}

TEST(Explorer, SearchNoRecursionWildcard) {
    tiny::Explorer explorer(basePath.string());
    explorer.setSearchDepth(0);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto &file: explorer.search("*.txt")) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{"test1.txt", "test2.txt", "test3.txt"};

    ASSERT_EQ(got, expect);
}

TEST(Explorer, RecursiveSearch) {
    tiny::Explorer explorer(basePath.string());
    explorer.setSearchDepth(1);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto &file: explorer.search("test1_inner.txt")) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{"test1_inner.txt"};

    ASSERT_EQ(got, expect);
}

TEST(Explorer, RecursiveSearchWildcard) {
    tiny::Explorer explorer(basePath.string());
    explorer.setSearchDepth(1);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto &file: explorer.search("*.txt")) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{"test1.txt", "test2.txt", "test3.txt", "test1_inner.txt", "test2_inner.txt"};

    ASSERT_EQ(got, expect);
}

TEST(Explorer, RecursiveSearchMany) {
    tiny::Explorer explorer(basePath.string());
    explorer.setSearchDepth(1);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto &file: explorer.searchMany(std::vector<std::string>{"test1.txt", "test1_inner.txt"})) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{"test1.txt", "test1_inner.txt"};

    ASSERT_EQ(got, expect);
}

TEST(Explorer, SearchSpacedFilename) {
    tiny::Explorer explorer(basePath.string());
    explorer.setSearchDepth(0);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto &file: explorer.search("te st6.tst")) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{"te st6.tst"};

    ASSERT_EQ(got, expect);
}


TEST(Explorer, SearchNonASCIIFilename) {
    tiny::Explorer explorer(basePath.string());
    explorer.setSearchDepth(0);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto &file: explorer.search("teñst7.tst")) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{"teñst7.tst"};

    ASSERT_EQ(got, expect);
}

TEST(Explorer, Cleanup) {
    std::filesystem::remove_all(basePath);
}