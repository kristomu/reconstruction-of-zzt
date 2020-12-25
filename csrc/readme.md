This is a C++ port! (Using a pascal-to-C translator and some manual coding)

Editor doesn't work yet.

Known bugs:
- Some errors in OOP parsing (particularly Preposterous Machines' title screen).
- No sound or music.

How to compile?
- Replace ptoc/libptoc.a with a compiled version of the PtoC library: source can be found at https://github.com/knizhnik/ptoc
- run cmake .

Other thoughts:
- I need some kind of test harness that uses the FreePascal version as a reference, so I can clean up the code and still be (reasonably) sure I'm being bug-compatible.
