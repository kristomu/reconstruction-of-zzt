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

- Cursor is always shown, even when it's not desired.
- Unicode characters are garbled at the right edge of the screen due to Crt having been coaxed into something it really doesn't support.
- Pressing H for Help in the editor hangs ZZT. (Not a hang, the rendering just stops. Blindly pressing Q,Y gets you out of the game.)
- Energizers in demo.zzt get the wrong color and character after reading and closing a scroll. Related to the crude double-buffer.
- Picking up an item with data (object or scroll) in the editor and then going to a different board leads to use-after-free and a potential crash because ZZT doesn't spare the Data field when deallocating the board. It only works in DOS because DOS has no memory protection.
- Duplicating something with stats onto the player causes a range check error. (game.pas:386, only intermittently)

### Known limitations

- Crt really isn't made for Unicode output and so has strange corner cases (see above)
- Even with the stdin-flushing fix, moving the player about by holding down a key is kinda janky. It probably can't be improved without going fully to ncurses or SDL.

### Suspected bugs

- Possible performance regressions involving SOUNDS timer.

### Notation

Bugfixes marked with IMP are portable, i.e. not specific to the Linux port, and
should probably be included in later versions of ZZT for DOS as well.
