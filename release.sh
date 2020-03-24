#!/bin/sh
# Compile with release parameters
cd src
# Remove old .o and .ppu files if we compiled the debug version last time
rm *.{o,ppu}
fpc -O3 -Xs zzt.pas -orel_zzt
