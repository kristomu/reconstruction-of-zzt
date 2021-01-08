# Equality source

This directory contains the source from commit 2c18b5221f2c44d68, modified
minimally to support fuzz mode. It is used for the equality check, which
uses fuzzing to find test cases where the C++ implementation behaves
different than the Pascal implementation without the latter crashing.

The 2c18b commit has no anti-crash fixes and so should be as faithful to the
original as possible while still running under Linux.
