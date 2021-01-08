#!/bin/sh
# Compile library with release parameters.
# (We only need it to be fast.)
cd src/equality
# Remove old .o and .ppu files
rm *.{o,ppu}
fpc -Cs49152 -XS -O3 -Op3 -Xs -fPIC paszzt_test.pas
mv libpaszzt_test.so ../../lib/fpc-zzt
