. ./shared_variables.sh
OUT="$ROOT_OUTDIR"
NOW_SEC=`date +%s`
echo NOW_SEC $NOW_SEC 
echo recently modified files:
LATEST_FILE=`ls $ROOT_OUTDIR/* -td -1 | grep -v log  | head -n1`
echo $LATEST_FILE | (
 grep replay > /dev/null || (
	ls $ROOT_OUTDIR/* -lotd | head -n6
 )
)
ls $ROOT_OUTDIR/* -tdo -1 | grep replay |head -n4

LATEST_FILE=`ls $ROOT_OUTDIR/* -td -1 | grep replay  | head -n1`
if  [ $LATEST_FILE = "$ROOT_OUTDIR/toreplay" ]
then
	#echo foo
	LATEST_FILE=`ls $ROOT_OUTDIR/toreplay/* -td -1 | grep replay  | head -n1`	
fi

if  [ $LATEST_FILE = "$ROOT_OUTDIR/toreplay/replayed" ]
then
	echo foo
	LATEST_FILE=`ls $ROOT_OUTDIR/toreplay/replayed/* -td -1 | grep replay  | head -n1`	
else
	echo oof
fi

echo  LATEST_FILE $LATEST_FILE 
echo $LATEST_FILE | (
 grep replay > /dev/null && (
  if [ -e $LATEST_FILE/last_crash_sec ]
  then
	ls $LATEST_FILE/*re -lotd | head
	SEC=`cat $LATEST_FILE/last_crash_sec`
	echo $SEC $(($NOW_SEC-$SEC))
	ls -l $LATEST_FILE/$SEC.KEYCODEpure | head -n4
	echo `cat $LATEST_FILE/$SEC.KEYCODEpure | sed s/KK:\//g`
	cat $LATEST_FILE/$SEC.GDB | grep "VIOLATED" #-A 15 -B 5
	cat $LATEST_FILE/$SEC.GDB | grep "signal SIG" #-A 15 -B 5
	cat $LATEST_FILE/$SEC.GDB | grep "lyx::" | head -n5 #-A 15 -B 5
  else
	ls $LATEST_FILE -lot | head
	cat `echo $LATEST_FILE | sed s/KEYCODEpure.replay/GDB/` | grep "signal SIG" -A 19
  fi
	cat $LATEST_FILE/log | grep Bore | tail -n2
 ) || (
	ls $ROOT_OUTDIR/* -lotd | head
 )
)


grep -F "autolyx:
Trace
reproduced
X_PID
x-session" $ROOT_OUTDIR/log | grep -v kill | grep -v Terminated | tail -n 9
#exit
echo autolyx crashes ---------
grep autolyx: $ROOT_OUTDIR/log | grep -v kill | grep -v Terminated | grep -v grep #-A 5
echo python crashes ---------
grep -i -a Trace $ROOT_OUTDIR/log -A 7 | tail -n8
#echo misc ----
#grep reproduced $ROOT_OUTDIR/log | tail -n5
