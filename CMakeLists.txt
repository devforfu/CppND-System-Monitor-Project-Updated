cmake_minimum_required(VERSION 3.10)

project(monitor)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

add_executable(monitor ${SOURCES})

set_property(TARGET monitor PROPERTY CXX_STANDARD 17)
target_link_libraries(monitor ${CURSES_LIBRARIES})
target_compile_options(monitor PRIVATE -Wall -Wextra -Werror)

add_executable(tests tests/test.cpp src/format.cpp src/linux_parser.cpp)
set_property(TARGET tests PROPERTY CXX_STANDARD 17)
target_compile_options(tests PRIVATE -Wall -Wextra)
