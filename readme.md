# The Reconstruction of ZZT

The Reconstruction of ZZT is a reverse-engineered recreation of the source code to the last official release of
[ZZT](https://museumofzzt.com/about-zzt), ZZT 3.2. The output executable is byte-for-byte identical with said
release, which means that the source code accurately represents the original engine's behavior.

The intent behind this project is to facilitate improved preservation and accessibility of ZZT's worlds and community,
as well as facilitate new, exciting developments.

## This repo

Here I'll try to make the Reconstruction compile with Free Pascal in general,
and then on Linux in particular. As a consequence, I'll likely disable some
features, at least to begin with, so that ZZT can work in a terminal.

In particular, I'm likely going to disable sound because direct PC speaker
access is hard, and mouse and joystick support for similar reasons.

## Linux status

### Known missing features

- No joystick or mouse support.
- No sound or music.

### Known bugs not in original ZZT

- Cursor is always shown, even when it's not desired.
- Unicode characters are garbled at the right edge of the screen due to Crt having been coaxed into something it really doesn't support.
- Duplicating something with stats onto the player causes a range check error. (game.pas:386, only intermittently)
- Fails to load files in very long paths, e.g. /a/b/c/d/e/f/g/h/findings//.cur_input
- Sometimes happens: file not found when trying to play a world that has been successfully loaded. (NOSTATS.ZZT)
- Regression: memory leaks on opening and/or closing boards. Fix later once ZZT is robust to fuzzing (probably by making it crash when a leak is detected, and then rerunning fuzz).

### Known bugs also in original ZZT

### Known limitations

- Crt really isn't made for Unicode output and so has strange corner cases (see above)
- Even with the stdin-flushing fix, moving the player about by holding down a key is kinda janky. It probably can't be improved without going fully to ncurses or SDL.

### Suspected bugs

- Possible performance regressions involving SOUNDS timer.
- Will probably crash on getting a black key since memory access is much stricter here.

### Fuzzing

ZZT really isn't equipped for fuzz testing yet. Here's how to do it in its
current state:

- Remove every video output call and Delay call
- Compile zztfuzz.pas to get a tester (release.sh now does so)
- If using afl: put the tester file in a directory with tests/ and findings/
subdirectories.
- Run afl-fuzz -Q -f TEST.ZZT -i tests/ -o findings/ ../src/rel_zzt_fuzz TEST.ZZT
- After finding a bug, debug zzt with it and set a breakpoint on SYSTEM_$$_INTERNALEXIT or just inspect the runtime error trace.

### Notation

Bugfixes marked with IMP are portable, i.e. not specific to the Linux port, and
should probably be included in later versions of ZZT for DOS as well.
