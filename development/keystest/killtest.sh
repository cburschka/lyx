#!/bin/sh
KT=`dirname "$0"`
AUTOLYX=`ps gux "-u$USER" | grep autolyx | grep -v grep | sed 's/[^ ]* //' | sed s/0.0.*//g`
TESTPY=`ps gux "-u$USER" | grep test.py | grep -v grep | sed 's/[^ ]* //' | sed s/0.0.*//g`

killall autolyx
killall test.py
killall lyx
killall gdb
killall xterm
killall replay.sh
killall xclip
kill $AUTOLYX $TESTPY
sleep 0.3
killall autolyx -9
killall test.py -9
killall lyx -9
killall gdb -9
killall xterm -9
killall replay.sh -9
killall xclip -9
$KT/killall_p reproduce
$KT/killall_p doNtimes
kill $AUTOLYX $TESTPY -9
