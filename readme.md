# The Reconstruction of ZZT

The Reconstruction of ZZT is a reverse-engineered recreation of the source code to the last official release of
[ZZT](https://museumofzzt.com/about-zzt), ZZT 3.2. The output executable is byte-for-byte identical with said
release, which means that the source code accurately represents the original engine's behavior.

The intent behind this project is to facilitate improved preservation and accessibility of ZZT's worlds and community,
as well as facilitate new, exciting developments.

## This repo

Here I'll try to make the Reconstruction compile with Free Pascal in general,
and then on Linux in particular. As a consequence, I'll likely disable some
features, at least to begin with, so that ZZT can work in a terminal. Joystick
and mouse control, is not supported; nor is sound.

The cport branch is focused on making an almost-completely accurate
reproduction of the original ZZT in modern C/C++. Any intentional differences
are mentioned below; beyond these, any difference from DOS constitutes a bug.

## Linux status

### Intentional differences between DOS and Linux C++ ZZT.

- Padding bytes in .ZZT, .BRD, are set to zero on load, so as not to clutter the data structures too much. Under normal conditions, they make no difference as no ZZT object nor OOP command can read them or write to them.
- Pointers may have a different size than in DOS; however, this is invisible for the same reason. No ZZT objects do pointer arithmetic in a way that's visible to the user.

### Known missing features

- No joystick or mouse support.
- No sound or music.

### Known bugs

### Known limitations

- Even with the stdin-flushing fix, moving the player about by holding down a key is kinda janky. It probably can't be improved without going fully to ncurses or SDL.

### Notation

Bugfixes marked with IMP are portable, i.e. not specific to the Linux port, and
should probably be included in later versions of ZZT for DOS as well.
