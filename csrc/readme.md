This is a C++ port! (Using a pascal-to-C translator and some manual coding)

Editor doesn't work yet.

Known bugs:
- Some errors in OOP parsing (particularly Preposterous Machines' title screen).
- No sound or music.
- Single-line messages don't show (regression).
- Line walls look weird.

How to compile?
- Replace ptoc/libptoc.a with a compiled version of the PtoC library: source can be found at https://github.com/knizhnik/ptoc
- run cmake .
