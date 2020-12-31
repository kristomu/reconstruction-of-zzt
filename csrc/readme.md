This is a C++ port! (Using a pascal-to-C translator and some manual coding)

Known bugs and missing functionality:
- No sound or music.
- High score file loading and writing isn't implemented.
- ZZT isn't aware of the size of the terminal and so doesn't do the right thing with less than 25 lines available.

Notable coding conventions:
- Everything that uses underscore_naming_convention is C or C++ based and (unless otherwise marked) zero-indexed. Most of what uses PascalCase is one-based, with TBoard, TStat etc. being the most notable exceptions. (I need to change these and TVideoLine to underscore case). The point of doing this is to keep zero-based string processing and one-based separate, and thus reduce the risk of off-by-one errors.

How to compile?
- Replace ptoc/libptoc.a with a compiled version of the PtoC library: source can be found at https://github.com/knizhnik/ptoc
- run cmake .

Other thoughts:
- I need some kind of test harness that uses the FreePascal version as a reference, so I can clean up the code and still be (reasonably) sure I'm being bug-compatible.
- The implementation currently does the Mandelbrot render from Preposterous Machines in 34 minutes. My recorded time for this render on the FPC Linux Reconstruction is 50 minutes.
- I still have to import the changes to fuzz branch's editor.

Current test failures:
- ./testcase/crash/SLIME.ZZT
