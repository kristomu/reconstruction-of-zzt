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

- Modifier keys (Shift, Ctrl, Alt...) don't work.
- Positioning is off on certain terminals (e.g. ssh, xterm).
- Characters look horrible on most terminals.
- Moving the player around is still choppy and hard.
- Scroll furling/unfurling doesn't render properly (missing VideoMove)
- Games start on the title screen instead of the specified first board.

### Suspected bugs

- Possible performance regressions involving SOUNDS timer
- File I/O may not be working (needs testing)
