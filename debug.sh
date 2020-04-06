#!/bin/sh
# Compile with debug parameters
cd src
rm *.o *.ppu
fpc -Cr -Ct -Co -Ci -g -gv -gl zztfuzz.pas -ofuzzt
rm *.o *.ppu
fpc -Cr -Ct -Co -Ci -g -gv -gl zzt.pas -ozzt
