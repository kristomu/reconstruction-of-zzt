#!/bin/sh

# Replace the parameter below with the location of your install of afl.
AFL_FUZZER_DIR="/usr/local/bin"

AFL_AUTORESUME=1

# Use some boards from TOWN.ZZT as source files for the fuzzer.
$AFL_FUZZER_DIR/afl-fuzz -i testcase/town_boards/ -o fuzz_results/ -f TESTING.ZZT -- ./fuzzt-afl TESTING.ZZT
