#!/bin/sh
# Compile with release parameters
cd src
# Remove old .o and .ppu files if we compiled the debug version last time
rm *.{o,ppu}
fpc -O3 -Xs -Op3 zzt.pas -orel_zzt
rm *.{o,ppu}
fpc -XS -O3 -Op3 -Xs zztfuzz.pas -orel_zzt_fuzz
