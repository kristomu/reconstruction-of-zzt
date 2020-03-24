#!/bin/sh
# Compile with debug parameters
cd src
rm *.o
rm *.ppu
fpc -Cr -Ct -Co -Ci -g -gv -gl zzt.pas -ozzt
