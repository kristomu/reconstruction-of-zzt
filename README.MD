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
