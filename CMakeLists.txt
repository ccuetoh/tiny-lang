cmake_minimum_required(VERSION 3.16)
project(tiny)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(test)
# add_subdirectory(src/python)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
            EXPECTED_HASH SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
            TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE ${CMAKE_CURRENT_SOURCE_DIR}/conanfile.txt
        INSTALL_FOLDER ${CMAKE_BINARY_DIR}
        BUILD missing
        REMOTE conancenter
        SETTINGS ${settings})

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

find_package(utf8cpp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(pybind11 REQUIRED)

set(CONAN_PKGS utf8cpp::utf8cpp nlohmann_json::nlohmann_json pybind11::pybind11)

file(GLOB SOURCES "src/*.cpp")

add_executable(tiny ${SOURCES})
target_link_libraries(tiny ${CONAN_PKGS})
