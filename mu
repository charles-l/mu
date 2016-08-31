#!/bin/bash
#
# Compile mu if necessary before running it.

# show make output only if something needs doing
make -q || make >&2 || exit 1

./mu_bin $FLAGS "$@"

# Scenarios considered:
#   mu
#   mu --help
#   mu test
#   mu test file1.mu
