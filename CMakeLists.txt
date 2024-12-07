cmake_minimum_required(VERSION 3.22.1)
project(razz)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -g")

find_package(Catch2 3 REQUIRED)
find_package(Curses REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${CURSES_INCLUDE_DIR})

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})

set(CMAKE_BUILD_TYPE Debug)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Unit testing

enable_testing()

file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
set(TEST_DEPENDENCIES
    src/LineGapBuffer.h
    src/LineGapBuffer.cpp
)

if (TEST_SOURCES)
    add_executable(tests ${TEST_SOURCES} ${TEST_DEPENDENCIES})
    target_link_libraries(tests PRIVATE Catch2::Catch2WithMain ${CURSES_LIBRARIES})
    add_test(NAME razz_unit_tests COMMAND tests)
endif()
