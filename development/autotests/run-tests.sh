#!/bin/bash

# This script runs all the test scripts it finds in the current folder
# Tests are identified as having a file name of *-in.txt
# For failed tests, the collected output is kept in the corresponding folder

export LYX_EXE=../../../src/lyx
export KEYTEST=../keytest.py

if [ "$#" -eq 0 ]; then
    TESTS=$(ls *-in.txt);
else
    TESTS=$*
fi

echo
echo "Running test cases . . ."
failed=0
for t in $(echo "$TESTS" | sed -e 's/-in.txt//g'); do
    rm -rf "out-$t"
    mkdir "out-$t"
    cd "out-$t"
    printf "%40s: " $t
    if ../single-test.sh "../$t-in.txt" > keytest-log.txt 2>&1; then
	echo Ok
	cd ..
	rm -rf "out-$t";
    else
	echo FAILED
	cd ..
	failed=$[$failed+1];
    fi;
done

echo
if [ $failed -eq 0 ]; then
    echo "All tests SUCCESSFUL"
else
    echo "There were $failed FAILED tests";
fi

echo
