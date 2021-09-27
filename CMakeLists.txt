cmake_minimum_required(VERSION 3.16)
project(tiny)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(test)

include(conanbuildinfo.cmake)
conan_basic_setup()

file(GLOB SOURCES "src/*.cpp")

add_executable(tiny ${SOURCES})
target_link_libraries(tiny ${CONAN_LIBS})
