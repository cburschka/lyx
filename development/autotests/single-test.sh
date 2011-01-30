#!/bin/bash

# This script invokes the keytest.py script with the simple set-up needed
# to re-run deterministic regression tests that one would like to have.
#
# Usage: run-test.sh <test-in.txt>
#
# See the hello-world-in.txt for an example syntax and description

KEYTEST=${KEYTEST:-./keytest.py}

export KEYTEST_OUTFILE=out.txt
export KEYTEST_INFILE=in-sample.txt
if [ "$1" != "" ]; then
    KEYTEST_INFILE="$1";
fi

export MAX_DROP=0
export LYX_WINDOW_NAME=$(wmctrl -l | grep -v Mozilla | grep -v Firefox | grep LyX | sed -e 's/.*\(LyX: .*\)$/\1/')
export MAX_LOOPS=1
export LYX_EXE=${LYX_EXE:-../../src/lyx}

$KEYTEST
