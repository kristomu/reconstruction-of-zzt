#!/bin/sh

# Replace the parameter below with the location of your install of afl.
AFL_FUZZER_DIR="/usr/local/bin"

cmake -DAFL_FUZZER_DIR="$AFL_FUZZER_DIR" .

AFL_HARDEN=1 make
