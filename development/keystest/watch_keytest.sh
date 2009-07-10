
NOW_SEC=`date +%s`
echo NOW_SEC $NOW_SEC 
echo recently modified files:
LATEST_FILE=`ls out/* -td -1 | grep -v log  | head -n1`
echo $LATEST_FILE | (
 grep replay > /dev/null || (
	ls out/* -lotd | head
 )
)
ls out/* -tdo -1 | grep replay  

LATEST_FILE=`ls out/* -td -1 | grep replay  | head -n1`
echo  LATEST_FILE $LATEST_FILE 
echo $LATEST_FILE | (
 grep replay > /dev/null && (
  if [ -e $LATEST_FILE/last_crash_sec ]
  then
	ls $LATEST_FILE/*re -lotd | head
	SEC=`cat $LATEST_FILE/last_crash_sec`
	echo $SEC $(($NOW_SEC-$SEC))
	ls -l $LATEST_FILE/$SEC.KEYCODEpure
	echo `cat $LATEST_FILE/$SEC.KEYCODEpure | sed s/KK:\//g`
	cat $LATEST_FILE/$SEC.GDB | grep "signal SIG" -A 15
  else
	ls $LATEST_FILE -lot | head
	cat `echo $LATEST_FILE | sed s/KEYCODEpure.replay/GDB/` | grep "signal SIG" -A 9
  fi
	cat $LATEST_FILE/log | grep Bore | tail -n2
 ) || (
	ls out/* -lotd | head
 )
)


grep -F "autolyx:
Trace
reproduced
X_PID
x-session" out/log | grep -v kill | grep -v Terminated | tail -n 9
exit
echo autolyx crashes ---------
grep autolyx: out/log | grep -v kill | grep -v Terminated #-A 5
echo python crashes ---------
grep -i Trace out/log -A 5
echo misc ----
grep reproduced out/log | tail -n5
