AUTOLYX=`ps gaux | grep autolyx | grep -v grep | sed 's/[^ ]* //' | sed s/0.0.*//g`
killall autolyx
killall test.py
killall lyx
killall gdb
killall xterm
kill $AUTOLYX
sleep 0.3
killall autolyx -9
killall test.py -9
killall lyx -9
killall gdb -9
killall xterm -9
kill $AUTOLYX -9
