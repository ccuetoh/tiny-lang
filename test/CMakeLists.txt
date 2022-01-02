cmake_minimum_required(VERSION 3.16)

project(tests)
enable_testing()

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/97a467571a0f615a4d96e79e4399c43221ca1232.zip
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

include(${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo.cmake)
conan_basic_setup()

add_executable(tests ${SOURCES} ${TESTS})
target_link_libraries(tests gtest gtest_main ${CONAN_LIBS})
