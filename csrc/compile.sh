#!/bin/bash
# Example arguments: zzt.cxx gamevars.cxx fileops.cxx curses.cc

g++ -DTURBO_PASCAL $* ptoc/libptoc.a -lm -ggdb -lncursesw 
