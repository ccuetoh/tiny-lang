#include "gtest/gtest.h"

#include "explorer.h"

#include <fstream>
#include <unordered_set>

const auto explorerSandboxPath = std::filesystem::current_path() / "sandbox"/ "explorer";
const auto explorerInnerPath = explorerSandboxPath / "inner";

TEST(Explorer, SandboxSetup) {
    // Set up the sandbox
    std::filesystem::create_directory(std::filesystem::current_path() / "sandbox");
    std::filesystem::create_directory(explorerSandboxPath);

    std::ofstream file1(explorerSandboxPath / "test1.txt");
    std::ofstream file2(explorerSandboxPath / "test2.txt");
    std::ofstream file3(explorerSandboxPath / "test3.txt");
    std::ofstream file4(explorerSandboxPath / "test4.tst");
    std::ofstream file5(explorerSandboxPath / "test5.tst");
    std::ofstream file6(explorerSandboxPath / "te st6.tst"); // Spaced filename
    std::ofstream file7(explorerSandboxPath / "teñst7.tst"); // Non-ASCII filename

    std::filesystem::create_directory(explorerInnerPath);
    std::ofstream file8(explorerInnerPath / "test1_inner.txt");
    std::ofstream file9(explorerInnerPath / "test2_inner.txt");

    std::cout << "File Explorer Sandbox path is: " << explorerSandboxPath << std::endl;
}

TEST(Explorer, SearchNoRecursion) {
    tiny::Explorer explorer(explorerSandboxPath);
    explorer.setSearchDepth(0);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto const &file: explorer.search("test1.txt")) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{"test1.txt"};

    ASSERT_EQ(got, expect);
}

TEST(Explorer, SearchNotFound) {
    tiny::Explorer explorer(explorerSandboxPath);
    explorer.setSearchDepth(0);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto const &file: explorer.search("does_not_exist.txt")) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{};

    ASSERT_EQ(got, expect);
}

TEST(Explorer, SearchNoRecursionWildcard) {
    tiny::Explorer explorer(explorerSandboxPath);
    explorer.setSearchDepth(0);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto const &file: explorer.search("*.txt")) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{"test1.txt", "test2.txt", "test3.txt"};

    ASSERT_EQ(got, expect);
}

TEST(Explorer, RecursiveSearch) {
    tiny::Explorer explorer(explorerSandboxPath);
    explorer.setSearchDepth(1);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto const &file: explorer.search("test1_inner.txt")) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{"test1_inner.txt"};

    ASSERT_EQ(got, expect);
}

TEST(Explorer, RecursiveSearchWildcard) {
    tiny::Explorer explorer(explorerSandboxPath);
    explorer.setSearchDepth(1);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto const &file: explorer.search("*.txt")) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{"test1.txt", "test2.txt", "test3.txt", "test1_inner.txt", "test2_inner.txt"};

    ASSERT_EQ(got, expect);
}

TEST(Explorer, RecursiveSearchMany) {
    tiny::Explorer explorer(explorerSandboxPath);
    explorer.setSearchDepth(1);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto const &file: explorer.search(std::vector<std::string>{"test1.txt", "test1_inner.txt"})) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{"test1.txt", "test1_inner.txt"};

    ASSERT_EQ(got, expect);
}

TEST(Explorer, SearchSpacedFilename) {
    tiny::Explorer explorer(explorerSandboxPath);
    explorer.setSearchDepth(0);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto const &file: explorer.search("te st6.tst")) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{"te st6.tst"};

    ASSERT_EQ(got, expect);
}


TEST(Explorer, SearchNonASCIIFilename) {
    std::filesystem::current_path() / "sandbox";
    tiny::Explorer explorer(explorerSandboxPath);
    explorer.setSearchDepth(0);

    std::unordered_set<std::string> got; // Use a set since the order is undefined
    for (auto const &file: explorer.search("teñst7.tst")) {
        got.insert(file.path().filename().string());
    }

    std::unordered_set<std::string> expect{"teñst7.tst"};

    ASSERT_EQ(got, expect);
}

TEST(Explorer, Cleanup) {
    std::filesystem::remove_all(explorerSandboxPath);
}