This is a C++ port! (Using a pascal-to-C translator and some manual coding)

Editor doesn't work yet.

Known bugs:
- Double buffering doesn't work, so closing a scroll blanks that area of the screen.
- Some errors in OOP parsing (particularly Preposterous Machines' title screen).
- No sound or music.
- Single-line messages don't show (regression).

How to compile?
- Replace ptoc/libptoc.a with a compiled version of the PtoC library: source can be found at https://github.com/knizhnik/ptoc
- run cmake .
