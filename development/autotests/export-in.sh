#!/bin/bash

failed=0
for libsubdir in doc examples; do
    for format in xhtml lyx16x; do
        for f in $LYX_ROOT/lib/$libsubdir/*lyx; do
            if $LYX_EXE -e $format $f >> lyx-log.txt 2>&1; then
                echo $format $f TEST_GOOD
            else
                echo $format $f TEST_BAD
                failed=$[$failed+1];
            fi;
        done;
    done;
done

if [ $failed -eq 0 ]; then
    echo "All formats SUCCESSFUL"
    exit 0;
else
    echo "There were $failed FAILED format tests"
    exit -1;
fi
