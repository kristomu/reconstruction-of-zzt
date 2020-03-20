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

### Known bugs

- Scroll furling doesn't render properly (missing VideoMove)
- Trying to create an object leads to a segfault. (Editing existing objects works.)
- Cursor is always shown, even when it's not desired.
- Objects don't run when playing a world.
- Unicode characters are garbled at the right edge of the screen due to Crt having been coaxed into something it really doesn't support.
- Pressing H for Help in the editor hangs ZZT.

### Known limitations

- Crt really isn't made for Unicode output and so has strange corner cases (see above)
- Even with the stdin-flushing fix, moving the player about by holding down a key is kinda janky. It probably can't be improved without going fully to ncurses or SDL.

### Suspected bugs

- Possible performance regressions involving SOUNDS timer.
