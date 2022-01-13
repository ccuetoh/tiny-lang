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

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
            EXPECTED_HASH SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
            TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE ${CMAKE_CURRENT_SOURCE_DIR}/../conanfile.txt
        INSTALL_FOLDER ${CMAKE_BINARY_DIR}
        BUILD missing
        REMOTE conancenter
        SETTINGS ${settings})

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

find_package(utf8cpp REQUIRED)
find_package(nlohmann_json REQUIRED)

set(CONAN_PKGS utf8cpp::utf8cpp nlohmann_json::nlohmann_json)

add_executable(tests ${SOURCES} ${TESTS})
target_link_libraries(tests gtest gtest_main ${CONAN_PKGS})
