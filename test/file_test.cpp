#include "gtest/gtest.h"

#include <fstream>

#include "file.h"

const auto fileSelectorSandboxPath = std::filesystem::current_path() / "sandbox" / "file_selector";
const auto fileSelectorInnerPath = fileSelectorSandboxPath / "inner";
const auto fileSelectorInner2Path = fileSelectorSandboxPath / "inner2";


TEST(FileSelector, SandboxSetup) {
    std::filesystem::create_directory(std::filesystem::current_path() / "sandbox");
    std::filesystem::create_directory(fileSelectorSandboxPath);

    std::ofstream meta(fileSelectorSandboxPath / "tiny.toml");
    std::ofstream file1(fileSelectorSandboxPath / "test1.ty");
    std::ofstream file2(fileSelectorSandboxPath / "test2.ty");
    std::ofstream file3(fileSelectorSandboxPath / "test3.ty");
    std::ofstream file4(fileSelectorSandboxPath / "test4.ty");
    std::ofstream file5(fileSelectorSandboxPath / "test5.ty");
    std::ofstream file6(fileSelectorSandboxPath / "te st6.ty"); // Spaced filename
    std::ofstream file7(fileSelectorSandboxPath / "teñst7.ty"); // Non-ASCII filename

    std::filesystem::create_directory(fileSelectorInnerPath);
    std::ofstream file8(fileSelectorInnerPath / "inner1.ty");
    std::ofstream file9(fileSelectorInnerPath / "inner2.ty");

    std::filesystem::create_directory(fileSelectorInner2Path);

    std::cout << "File Selector Sandbox path is: " << fileSelectorSandboxPath << std::endl;
}

TEST(FileSelector, GetLocalSources) {
    tiny::FileSelector fs(fileSelectorSandboxPath);
    try {
        auto sources = fs.getLocalSourceFiles();
        if (sources.size() != 7) {
            // Inner files should be ignored
            FAIL();
        }

        for (auto &source: sources) {
            if (source.type != tiny::FileType::Source || source.path.parent_path() != fileSelectorSandboxPath) {
                FAIL();
            }
        }


    } catch (tiny::SourcesNotFoundError &){
        FAIL();
    }
}

TEST(FileSelector, GetMetaNotFound) {
    tiny::FileSelector fs(fileSelectorInner2Path);
    try {
        auto meta = fs.getMetaFile();
        FAIL(); // Should throw

    } catch (tiny::MetaNotFoundError &){
        SUCCEED();
    }
}

TEST(FileSelector, GetMetaTooMany) {
    tiny::FileSelector fs(fileSelectorInner2Path);
    try {
        auto meta = fs.getMetaFile();
        FAIL(); // Should throw

    } catch (tiny::MetaNotFoundError &){
        SUCCEED();
    }
}

TEST(FileSelector, GetLocalSourcesNotFound) {
    tiny::FileSelector fs(fileSelectorInner2Path);
    try {
        auto sources = fs.getLocalSourceFiles();
        FAIL(); // Should throw
    } catch (tiny::SourcesNotFoundError &){
        SUCCEED();
    }
}

TEST(FileSelector, GetFiles) {
    std::int32_t sources = 0;
    std::int32_t meta = 0;

    tiny::FileSelector fs(fileSelectorSandboxPath);
    try {
        auto files = fs.getFiles();
        if (files.size() != 8) {
            // Inner files should be ignored
            FAIL();
        }

        for (auto &file: files) {
            if (file.path.parent_path() != fileSelectorSandboxPath) {
                FAIL();
            }

            if (file.type == tiny::FileType::Source) {
                sources++;
            }

            if (file.type == tiny::FileType::Meta) {
                meta++;
            }
        }
    } catch (tiny::SourcesNotFoundError &){
        FAIL();
    } catch (tiny::MetaNotFoundError &){
        FAIL();
    } catch (tiny::TooManyMetaFiles &){
        FAIL();
    }

    if (sources != 7 || meta != 1) {
        FAIL();
    }
}

TEST(FileSelector, Cleanup) {
    std::filesystem::remove_all(fileSelectorSandboxPath);
}
