#!/bin/bash

# This script runs all the test scripts it finds in the current folder
# Tests are identified as having a file name of *-in.txt
# For failed tests, the collected output is kept in the corresponding folder

export LYX_ROOT=../../..
export LYX_EXE=$LYX_ROOT/src/lyx

if [ "$XVKBD_HACKED" != "" ]; then
    export XVKBD_EXE=${XVKBD:-./xvkbd/xvkbd};
    if [ ! -x $XVKBD_EXE ]; then
	echo "You need to build XVKBD first, try: cd xvkbd && xmkmf && make"
	exit -1;
    fi
fi

if [ "$(which wmctrl)" == "" ]; then
    echo "You need to install wmctrl first, try:"
    echo "  sudo apt-get install wmctrl"
    exit -1;
fi

export XVKBD_EXE=../$XVKBD_EXE
export KEYTEST=../keytest.py
LYX_HOME=out-home
export LYX_USERDIR=$(pwd)/$LYX_HOME/.lyx
# Create locale links 
export LOCALE_DIR=../locale

if [ ! -d ../../locale ]; then
    echo "Some tests may require the GUI showing up in a specified language."
    echo "In order to make it work, I'm going to run this command:"
    mkdir -p locale
    cmd="ln -s `pwd`/locale ../../"
    echo "  $cmd"
    ans=""
    while [ "$ans" != "y" -a "$ans" != "n" ]; do
	echo "Should I proceed (y/n) ?"
	read ans;
    done
    if [ "$ans" == "y" ]; then
	$cmd;
    fi;
fi

if [ "$#" -eq 0 ]; then
    TESTS=$(ls *-in.txt *-in.sh | sed -e 's/hello-world-in.txt\|first-time-in.txt//')
    rm -rf out-*;
else
    TESTS=$*
fi

echo

if [ ! -d $LYX_HOME ]; then
    mkdir -p $LYX_HOME
#    mkdir -p $LYX_USERDIR
#    cp preferences $LYX_USERDIR
    cd $LYX_HOME
    echo "Initializing testing environment . . ."
    if ! ../single-test.sh "../first-time-in.txt" > test-log.txt 2>&1; then
	echo "Some error occurred: check $(pwd)"
	exit -1;
    fi
    cd ..
fi

# Launch the emergency STOP button
./stop_autotests.tcl &
pid=$!

function stop_button() {
    kill $pid
    wait $pid > /dev/null 2>&1
}

echo "Running test cases . . ."
failed=0
for tf in $(echo "$TESTS"); do
    t=$(echo $tf | sed -e 's/-in.txt//g' | sed -e 's/-in.sh//g')
    printf "%40s: " $tf
    if [ -f "$t-in.txt" ]; then
	cmd="../single-test.sh ../$t-in.txt";
    elif [ -f "$t-in.sh" ]; then
	if [ ! -x "$t-in.sh" ]; then
	    echo "ERROR: $t-in.sh is not executable"
	    stop_button
	    exit -1;
	fi
	cmd="../$tf";
    else
	echo "ERROR: File not found: $t-in.txt or $t-in.sh"
	stop_button
	exit -1;
    fi
    rm -rf "out-$t"
    mkdir "out-$t"
    cd "out-$t"
    if $cmd > test-log.txt 2>&1; then
	echo Ok
	cd ..
	rm -rf "out-$t";
    else
	echo FAILED
	cd ..
	failed=$[$failed+1];
    fi;
done

stop_button

echo
if [ $failed -eq 0 ]; then
    echo "All tests SUCCESSFUL"
else
    echo "There were $failed FAILED tests";
fi

echo
