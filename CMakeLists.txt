cmake_minimum_required (VERSION 3.1)

set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)

project(zzt)

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Parameters
# Enable Wall later.
# For turbo mode, add -O9 -march=native.
set(CMAKE_CXX_FLAGS "-ggdb -DTURBO_PASCAL")

# Require ncursesw
#find_package(NCurses REQUIRED)
#include_directories(${CURSES_INCLUDE_DIR})

# Quick and dirty make specification.
add_library(ptoc SHARED IMPORTED)
set_target_properties(ptoc
	PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/ptoc/libptoc.a)

# TODO: Cleanup

add_executable(zzt
	csrc/testing.cc
	csrc/zzt.cxx
	csrc/gamevars.cxx
	csrc/fileops.cxx
	csrc/io/curses.cc
	csrc/io/stub.cc
	csrc/hardware.cc
	csrc/video.cc
	csrc/unicode.cxx
	csrc/txtwind.cxx
	csrc/game.cxx
	csrc/minmax.cxx
	csrc/elements.cxx
	csrc/oop.cxx
	csrc/sounds.cxx
	csrc/editor.cxx
	csrc/tools.cc
	csrc/board.cc
	csrc/serialization.cc
	csrc/world.cc)

add_executable(fuzzt
	csrc/testing.cc
	csrc/fuzzt.cxx
	csrc/gamevars.cxx
	csrc/fileops.cxx
	csrc/io/curses.cc
	csrc/io/stub.cc
	csrc/hardware.cc
	csrc/video.cc
	csrc/unicode.cxx
	csrc/txtwind.cxx
	csrc/game.cxx
	csrc/minmax.cxx
	csrc/elements.cxx
	csrc/oop.cxx
	csrc/sounds.cxx
	csrc/editor.cxx
	csrc/tools.cc
	csrc/board.cc
	csrc/serialization.cc
	csrc/world.cc)

# For some bizarre reason, ncurses wide detection
# (see commented-out lines above) doesn't work on my system, something
# about ${CURSES_NCURSES_LIBRARY} already being defined before the
# CURSES_NEED_WIDE check being done in FindCurses.cmake. As I have more
# pressing things to do than chase a bug, I'll just kludge ncursesw in here.

target_link_libraries(zzt ptoc -lncursesw)
target_link_libraries(fuzzt ptoc -lncursesw)