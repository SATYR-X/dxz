cmake_minimum_required(VERSION 3.10)

project(AlgorithmDXZ VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(MAIN_SOURCES
    src/main.cpp
    src/DLX_with_ZDD.cpp
    )

set(TEST_SOURCES
    test/test.cpp
    src/DLX_with_ZDD.cpp
    )

add_executable(main ${MAIN_SOURCES})
add_executable(test ${TEST_SOURCES})