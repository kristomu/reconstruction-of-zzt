cmake_minimum_required (VERSION 3.1)

# TODO? Separate out C++ ZZT into a library because the equality test
# needs it too.

set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)

project(zzt)

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# If you want to build a fuzzable binary, set AFL_FUZZER_DIR to the
# directory that AFL lives in.

set(AFL_FUZZER_DIR "" CACHE STRING "AFL fuzzer binary directory")

if(AFL_FUZZER_DIR)
	set(CMAKE_C_COMPILER "${AFL_FUZZER_DIR}/afl-clang-fast")
	set(CMAKE_CXX_COMPILER "${AFL_FUZZER_DIR}/afl-clang-fast++")
	set(ZZT_NAME "zzt-afl")
	set(FUZZT_NAME "fuzzt-afl")
else()
    set(CMAKE_C_COMPILER gcc)
    set(CMAKE_CXX_COMPILER g++)
    set(ZZT_NAME "zzt")
	set(FUZZT_NAME "fuzzt")

	# Can't be made in AFL mode because it requires a Pascal library and
	# FreePascal doesn't support AFL instrumentation.
	set(EQUALZZT_NAME "fuzzt-equal")
endif()

# Parameters
# Enable Wall later.
# For turbo mode, add -O9 -march=native.

# If we're in fuzz mode, enable all the sanity checks we can, except ASAN
# (because it doesn't play nice with afl), and enable turbo mode so we can
# test as many instances at once as possible.
if(AFL_FUZZER_DIR)
	set(CMAKE_CXX_FLAGS "-fsanitize-trap=undefined,integer 	-fno-omit-frame-pointer -O9 -march=native -ggdb -DTURBO_PASCAL")
else()
	set(CMAKE_CXX_FLAGS "-ggdb -DTURBO_PASCAL")
endif()

# Require ncursesw
#find_package(NCurses REQUIRED)
#include_directories(${CURSES_INCLUDE_DIR})

# Quick and dirty make specification.
add_library(ptoc SHARED IMPORTED)
if(AFL_FUZZER_DIR)
	set_target_properties(ptoc
		PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/afl-ptoc/libptoc.a)
else()
	add_library(paszzt_test SHARED IMPORTED)
	set_target_properties(ptoc
		PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/ptoc/libptoc.a)
	set_target_properties(paszzt_test
		PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/fpc-zzt/libpaszzt_test.so)
endif()

# TODO: Cleanup

add_executable(${ZZT_NAME}
	csrc/random.cc
	csrc/input.cc
	csrc/testing.cc
	csrc/zzt.cxx
	csrc/gamevars.cxx
	csrc/fileops.cxx
	csrc/io/curses.cc
	csrc/io/stub.cc
	csrc/hardware.cc
	csrc/video.cc
	csrc/unicode.cc
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

add_executable(${FUZZT_NAME}
	csrc/random.cc
	csrc/input.cc
	csrc/testing.cc
	csrc/fuzzt.cxx
	csrc/gamevars.cxx
	csrc/fileops.cxx
	csrc/io/curses.cc
	csrc/io/stub.cc
	csrc/hardware.cc
	csrc/video.cc
	csrc/unicode.cc
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

target_link_libraries(${ZZT_NAME} ptoc -lncursesw)
target_link_libraries(${FUZZT_NAME} ptoc -lncursesw)

# Equality test

if(EQUALZZT_NAME)
	add_executable(${EQUALZZT_NAME}
		csrc/equality_test.cc)
	target_link_libraries(${EQUALZZT_NAME} ptoc paszzt_test -lncursesw)
endif()