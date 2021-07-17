cmake_minimum_required(VERSION 3.16)

project(tests)
enable_testing()

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG master
        SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/googletest-src"
        BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/googletest-build"
)
FetchContent_MakeAvailable(googletest)

include_directories("../src")

file(GLOB TESTS "*.cpp")
file(GLOB SOURCES "../src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*main.cpp$")

include(GoogleTest)
gtest_add_tests(TARGET tests SOURCES ${TESTS})

include(../conanbuildinfo.cmake)
conan_basic_setup()

add_executable(tests ${SOURCES} ${TESTS})
target_link_libraries(tests gtest gtest_main ${CONAN_LIBS})
