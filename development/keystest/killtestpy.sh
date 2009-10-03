TESTPY=`ps gaux | grep test.py | grep -v grep | sed 's/[^ ]* //' | sed s/0.0.*//g`
kill $TESTPY
sleep 0.1
kill $TESTPY -9
