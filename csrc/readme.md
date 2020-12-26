This is a C++ port! (Using a pascal-to-C translator and some manual coding)

Known bugs and missing functionality:
- No sound or music.
- Blinking sometimes fails to work; in particular, try to change the speed slider. (This one is strange because blinking works when I first initialize curses.)

Notable coding conventions:
- Everything that uses underscore_naming_convention is C or C++ based and (unless otherwise marked) zero-indexed. Most of what uses PascalCase is one-based, with TBoard, TStat etc. being the most notable exceptions. (I need to change these and TVideoLine to underscore case). The point of doing this is to keep zero-based string processing and one-based separate, and thus lower the risk of off-by-one errors.

How to compile?
- Replace ptoc/libptoc.a with a compiled version of the PtoC library: source can be found at https://github.com/knizhnik/ptoc
- run cmake .

Other thoughts:
- I need some kind of test harness that uses the FreePascal version as a reference, so I can clean up the code and still be (reasonably) sure I'm being bug-compatible.
- The implementation currently does the Mandelbrot render from Preposterous Machines in 34 minutes. My recorded time for this render on the FPC Linux Reconstruction is 50 minutes.
