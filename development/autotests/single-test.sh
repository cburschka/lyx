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

BASE=$( echo $KEYTEST_INFILE | sed 's/-in\.\(txt\|sh\)$//')
if [ -e $BASE.lyx.emergency ]; then
	echo "removing $BASE.lyx.emergency"
	rm $BASE.lyx.emergency
fi

export MAX_DROP=0
if [ "$(pidof lyx)" != "" ]; then
    export LYX_PID=$(pidof lyx)
    export LYX_WINDOW_NAME=$(wmctrl -l -p | grep " $LYX_PID " | cut -d ' ' -f 1);
fi
export MAX_LOOPS=1
export LYX_EXE=${LYX_EXE:-../../src/lyx}

if [ "$XVKBD_HACKED" != "" ]; then
    export XVKBD_EXE=${XVKBD_EXE:-./xvkbd/xvkbd};
else
    export XVKBD_EXE=${XVKBD_EXE:-xvkbd};
fi

$KEYTEST
