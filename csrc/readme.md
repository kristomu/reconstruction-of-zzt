This is a C++ port! (Using a pascal-to-C translator and some manual coding)

Known bugs and missing functionality:
- No sound or music.
- High score file loading and writing isn't implemented.
- ZZT isn't aware of the size of the terminal and so doesn't do the right thing with less than 25 lines available.
- The Town of ZZT: Entering the prison level crashes.

Notable coding conventions:
- Everything that uses underscore_naming_convention is C or C++ based and (unless otherwise marked) zero-indexed. Most of what uses PascalCase is one-based, with TBoard, TStat etc. being the most notable exceptions. (I need to change these and TVideoLine to underscore case). The point of doing this is to keep zero-based string processing and one-based separate, and thus reduce the risk of off-by-one errors.

How to compile?
- Replace ptoc/libptoc.a with a compiled version of the PtoC library: source can be found at https://github.com/knizhnik/ptoc
- run cmake .

Other thoughts:
- I need some kind of test harness that uses the FreePascal version as a reference, so I can clean up the code and still be (reasonably) sure I'm being bug-compatible.
- The implementation currently does the Mandelbrot render from Preposterous Machines in 34 minutes. My recorded time for this render on the FPC Linux Reconstruction is 50 minutes.

Current test failures:
- ./testcase/crash/CNOSTAT.ZZT
- ./testcase/crash/CR12.ZZT
- ./testcase/crash/CR14_DUP.ZZT
- ./testcase/crash/CR16.ZZT
- ./testcase/crash/CR19.ZZT
- ./testcase/crash/CR5.ZZT
- ./testcase/crash/CR6.ZZT
- ./testcase/crash/CR8.ZZT
- ./testcase/crash/CYCLE0.ZZT
- ./testcase/crash/INVAR6.ZZT
- ./testcase/crash/LONG.ZZT
- ./testcase/crash/OBJ_OOB.ZZT
- ./testcase/crash/OOBSHOT.ZZT
- ./testcase/crash/STRANGE.ZZT
- ./testcase/EDLEAK2.ZZT
- ./testcase/hang/CYCLE0.ZZT
- ./testcase/hang/PLAYER0.ZZT
- ./testcase/LEAK.ZZT
- ./testcase/MISALLOC.ZZT
- ./testcase/MONIBOMB.ZZT
- ./testcase/NOSTATS.ZZT
- ./testcase/OVERFLOW.ZZT

