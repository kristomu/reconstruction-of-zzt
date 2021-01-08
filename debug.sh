#!/bin/sh
# Compile with debug parameters
cd src
rm *.o *.ppu
# Stack size taken from the DOS $M spec
fpc -Cs49152 -Cr -Ct -Co -Ci -g -gv -gl zztfuzz.pas -ofuzzt
rm *.o *.ppu
fpc -Cs49152 -Cr -Ct -Co -Ci -g -gv -gl zzt.pas -ozzt
