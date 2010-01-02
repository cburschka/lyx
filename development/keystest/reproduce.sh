#!/bin/bash
KT=`dirname $0`
AND_THEN_QUIT="y" time $KT/autolyx $1 
RESULT=$?
echo RESULT_REPRODUCE $RESULT
exit $RESULT
