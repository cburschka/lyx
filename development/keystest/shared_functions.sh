DIRNAME0=`dirname "$0"`
#ROOT_OUTDIR="$DIRNAME0/out"
OUTDIR="$ROOT_OUTDIR"
#OUTDIR="$DIRNAME0/out"
THIS_PID=$$

#kill(){
#	echo kill
#}
#killall (){
#	echo killall
#}

mkdirp () {
	mkdir -p "$1"
	chmod g+w "$1"
} 

kill_all_children() {
        kill `$DIRNAME0/list_all_children.sh $1`
        sleep 0.1
        kill -9 `$DIRNAME0/list_all_children.sh $1`
}


#BORED_AFTER_SECS=7200 #If we have spent more than 3600 secs (an hour) replaying a file, without learning anything new, go and start looking for more bugs instead
if [ -z $BORED_AFTER_SECS ]
then
	BORED_AFTER_SECS=3600 #If we have spent more than 3600 secs (an hour) replaying a file, without learning anything new, go and start looking for more bugs instead
fi

LAST_CORE=""

#############################
# This section of code is LyX Specific
#############################

if [ ! -e $DIRNAME0/.lyx ]
then
	echo WARNING $DIRNAME0/.lyx does not exist
	echo will need to regenerate .lyx every test
fi

#if [ ! -e lib/doc.orig ]
#then
#	mv lib/doc lib/doc.orig
#fi

#kill_all_children() {
#        kill `$DIRNAME0/list_all_children.sh $1`
#        sleep 0.1
#        kill -9 `$DIRNAME0/list_all_children.sh $1`
#}

#. $DIRNAME0/shared_functions.sh


ensure_cannot_print () {
if [ ! -z "$REPLAYFILE" ]
then
	return
fi
if lpq
then
 
	echo We can print, this is bad!
	echo use lpadmin to stop keytest from destroying a forest.
	full_exit
	sleep 999999 ; read
else
	echo "Phew, lpq reckons we aren't ready to print. This is a *good* thing!"
fi
}

extras_save () {
 return 
 for f in `ls lib/doc`
 do
	if [ lib/doc/$f -nt lib/doc.orig/$f -o ! -e lib/doc.orig/$f ]
	then 
		#echo making doc dir $OUTDIR/$SEC.doc
		mkdirp $OUTDIR/$SEC.doc
		cp -a lib/doc/$f $OUTDIR/$SEC.doc/
	fi
 done
}

extras_prepare () {
	return
	mkdirp lib/doc/
	rm lib/doc/*.lyx
	cp -p lib/doc.orig/*.lyx lib/doc/
}

get_crash_id () {
  name=`(cat $GDB | grep -o ' in lyx::[[:alnum:]:]*' ; cat $GDB | grep -o ' [ai][nt] [[:alnum:]:]*' ) | head -n3 | sed s/in// | sed 's/ //g'`
  echo $name | sed 's/ /__/g'
}

calc_confirm_file() {
	id=`get_crash_id`
	echo "$ROOT_OUTDIR/$id.reproduced"
}

get_pid () {
	sleep 3
	echo getting pidof "$1" 1>&2
	#PID=`ps "-u$USER" "$2" | grep "$1" | grep -v grep | sed 's/^ *//g'|  sed 's/ .*$//'`
	PID=`ps x | grep "$1" | grep -v grep | grep -v "gdb " | sed 's/^ *//g'|  sed 's/ .*$//'`
	echo "$PID" | ( grep " " > /dev/null && ( echo ERROR too many PIDs 1>&2 ; ps x ; full_exit ) )
	nPIDs=`echo PID "$PID" | wc -l`
	echo nPIDs $nPIDs 1>&2
	sleep 1
	echo -- if [ "$nPIDs" != "1" ] 1>&2
	if test "$nPIDs" != "1" #2> /tmp/testerr
	then 
		echo autolyx: Wrong number of PIDs "$nPIDs" "($1)" "($2)" 1>&2
		echo autolyx: PIDs "$PID" 1>&2
		ps x 1>&2
		echo -----
	fi
	echo "$PID"
	echo got pidof "$1" 1>&2
}
clean_up () {
  	KT_PID=`get_pid keytest.py x`
  	kill $KT_PID
	sleep 0.1
  	kill -9 $KT_PID
}

full_exit() {
	clean_up

	echo attempting to exit this entire script... normal exit may just exit one function

	kill $THIS_PID
	sleep 1
	echo We should not get this far
	sleep 1
	kill -9 $THIS_PID
	echo We really should not get this far 
	exit
}

run_gdb () {
  #Spawn a process to kill lyx if it runs too long
  if ! touch $GDB
  then
	echo cannot touch $GDB
	full_exit
  fi
  (sleep 300; echo KILLER ACTIVATIED ;killall gdb lyx ; sleep 1 ; killall -9 gdb lyx)&
  KILLER_PID=$!
  echo KILLING LYX, before starting new lyx
  killall lyx
  sleep 1 
  killall lyx -9
  sleep 1
  echo Starting GDB
  #shell svn info $SRC_DIR/
  (echo "
  run
  bt
  #shell kill $CHILD_PID
  shell wmctrl -l
  shell sleep 1
  #shell kill -9 $CHILD_PID
" ; yes q) | HOME="$NEWHOME" gdb $EXE_TO_TEST 2>&1 | strings|  tee $GDB
  echo "end run_gdb ($KILLER_PID)"
  kill $KILLER_PID
  sleep 0.1
  kill -9 $KILLER_PID

  #### gcore $GDB.core
  #shell wmctrl -r __renamed__ -b add,shaded
  #shell wmctrl -r term -b add,shaded
  #shell wmctrl -r term -b add,shaded
  #shell wmctrl -R lyx' 
  #
  #shell import -window root '$GDB.png'
  #shell import -window root '$GDB..png'
  #exit
}


###########################



try_replay () {
	id=`get_crash_id`
	echo CRASH_ID 
	export CONFIRM_FILE=`calc_confirm_file`
	if [ ! -e "$CONFIRM_FILE" ]
	then
		echo $CONFIRM_FILE does not exist
		echo This bug appears not to have been reproduced before
		echo Will try to reproduce now
		echo
	        echo WANT_CRASH_ID=$WANT_CRASH_ID
		WANT_CRASH_ID="$id" do_replay
	        echo _WANT_CRASH_ID=$WANT_CRASH_ID
		echo 
		echo Finished attempt at replay
	else
		echo $CONFIRM_FILE exists
		echo This bugs has already been reproduced
		echo Will not attempt to reproduce again
	fi
}

do_replay() {
	(REPLAYFILE="$KEYCODEpure" TAIL_LINES=25 MAX_TAIL_LINES=10000 bash "$0")&
	TEST_PID="$!"
	echo Backgrounded $TEST_PID
	echo waiting for $TEST_PID to finish
	wait "$TEST_PID" 
}

test_replayed () {
	test -e "$f.replay/last_crash_sec" -o -e "$f.replay/Irreproducible" 
}

move_to_replayed () {
	mkdirp $REPLAY_DIR/replayed
	mv $f* $REPLAY_DIR/replayed
}

do_queued_replay() {
	if test_replayed
	then
		move_to_replayed
	else
		#./development/keytest/killtest
		killall lyx
		sleep 1
		echo killall -9 lyx
		killall -9 lyx
		KEYCODEpure="$f" do_replay
		#if test_replayed 
		#then 
		move_to_replayed
		#fi
	fi
}

do_queued_replays() {
REPLAY_DIR=$OUTDIR/toreplay
echo doing queued_replays
echo replays `ls $REPLAY_DIR/*KEYCODEpure`
for f in `ls $REPLAY_DIR/*KEYCODEpure`
do
	do_queued_replay
done
echo done queued_replays
(
  REPLAY_DIR=$OUTDIR/toreproduce
  export BORED_AFTER_SECS=0
  echo doing queued_reproduce
  echo reproduce`ls $REPLAY_DIR/*KEYCODEpure`
  for f in `ls $REPLAY_DIR/*KEYCODEpure`
  do
        do_queued_replay
  done
  echo done queued_reproduce
)
}

interesting_crash () {
(grep " signal SIG[^TK]" $GDB || grep KILL_FREEZE $KEYCODE) &&
   ( test -z "$WANT_CRASH_ID" || test "$WANT_CRASH_ID" = `get_crash_id` )
}

#get_pid() {
#	     ps a | grep $1 | grep -v grep | sed 's/^ *//g'|  sed 's/ .*$//'
#}

get_version_info() {
	(cd $SRC_DIR ; svn info) 2>&1 |tee "$1".svn
	$EXE_TO_TEST -version 2>&1 "$1".version
}

do_one_test() {
  GDB=$OUTDIR/$SEC.GDB
  KEYCODE=$OUTDIR/$SEC.KEYCODE
  KEYCODEpure=$OUTDIR/$SEC.KEYCODEpure
  NEWHOME=~/kt.dir/$SEC.dir
  mkdirp $NEWHOME
  NEWHOME=`cd $NEWHOME; pwd`
  echo NEWHOME $NEWHOME
  mkdirp "$NEWHOME"
  cp -rv $DIRNAME0/.lyx "$NEWHOME"/
  echo killall -9 lyx latex pdflatex
  killall -9 lyx latex pdflatex
  ( sleep 9 &&
     ps a | grep lyx 
	echo -- 1 || full_exit
     LYX_PID=""
     i=0
     echo -- while [ -z "$LYX_PID" -a 200 -gt $i ]
     while [ -z "$LYX_PID" -a 200 -gt $i ]
     do
	     #export LYX_PID=`ps a | grep /src/lyx | grep -v grep | sed 's/^ *//g'|  sed 's/ .*$//'`
	     export LYX_PID=`get_pid "$EXE_TO_TEST$" `
	     echo LYXPID "$LYX_PID" || full_exit
	     sleep 0.1
	     i=$(($i+1))
     done 
     echo `ps a | grep $EXE_TO_TEST`
	echo -- 2
     echo `ps a | grep $EXE_TO_TEST | grep -v grep`
	echo -- 3
     echo `ps a | grep $EXE_TO_TEST | grep -v grep | sed 's/ [a-z].*$//'`
	echo -- 4
     echo LYX_PID=$LYX_PID
     echo XA_PRIMARY | xclip -selection XA_PRIMARY
     echo XA_SECONDARY | xclip -selection XA_SECONDARY
     echo XA_CLIPBOARD | xclip -selection XA_CLIPBOARD

     echo -- if [ ! -z "$LYX_PID" ]
     if [ ! -z "$LYX_PID" ]
     then
	 kill `ps a | grep keytest.py | grep -v grep | cut -c 1-5`
	 sleep 0.2
	 kill -9 `ps a | grep keytest.py | grep -v grep | cut -c 1-5`
	while ! wmctrl -r lyx -b add,maximized_vert,maximized_horz
	do
		echo trying to maximize lyx
		sleep 1
	done
	 echo BEGIN KEYTEST KEYTEST_OUTFILE="$KEYCODEpure" nice -19 python $DIRNAME0/keytest.py
         KEYTEST_OUTFILE="$KEYCODEpure" nice -19 python $DIRNAME0/keytest.py | tee $KEYCODE
	 #echo "$!" > $NEWHOME/keytest_py.pid
	 echo END_KEYTEST
     fi
     echo NO_KEYTEST
     echo killall lyx
     killall lyx
     sleep 0.1
     kill -9 "$LYX_PID"
     killall -9 lyx #sometimes LyX really doesn't want to die causing the script to freeze
     #killall lyx #sometimes LyX really doesn't want to die causing the script to freeze
     sleep 1
     #kill -9 "$LYX_PID" #sometimes LyX really doesn't want to die causing the script to freeze

     #sleep 1 
     #killall -9 lyx
     ) &
  CHILD_PID="$!"
  ls $EXE_TO_TEST ; sleep 1
   pwd
  
  #You may want to use the following to simulate SIGFPE
  #(sleep 90 && killall -8 lyx) &
  echo TTL $TAIL_LINES
  extras_prepare
  ensure_cannot_print
  run_gdb
#  (run_gdb) &
#  GDBTASK_PID="$!"
#  (sleep 600 ; kill "$!")	
#  echo WAITING FOR: wait $GDBTASK_PID
#  wait $GDBTASK_PID
#  echo NOLONGER waiting for: wait $GDBTASK_PID

  echo END gdb
  kill $CHILD_PID
  KT_PID=`get_pid keytest.py`
  echo KT_PID=$KT_PID
  kill $KT_PID
  sleep 0.3
  kill -9 $CHILD_PID
  kill -9 $KT_PID
  # Or use "exited normally":
  echo END gdb2
  # these tend to take up a huge amount of space:
  echo will erase "$NEWHOME"
  sleep 2
  rm -rf $NEWHOME
  #if (grep " signal SIG[^TK]" $GDB || grep KILL_FREEZE $KEYCODE)
  if interesting_crash
  then
    extras_save
    mkdirp $OUTDIR/save && (
	    ln $OUTDIR/$SEC.* $OUTDIR/save ||
	    cp $OUTDIR/$SEC.* $OUTDIR/save)
    LAST_CRASH_SEC=$SEC
    echo $LAST_CRASH_SEC > $OUTDIR/last_crash_sec
    get_version_info $OUTDIR/last_crash_sec.info
    if [ ! -z "$TAIL_LINES" ]
    then
    	LAST_EVENT="$SEC"
	echo Reproducible > $OUTDIR/Reproducible
    fi
    TAIL_LINES="" 
    if [ -z "$REPLAYFILE" ]
    then
	echo ATTEMPTING TO REPLAY
	try_replay
    else
    	export KEYTEST_INFILE=$KEYCODEpure
	NUM_KEYCODES=`wc -l < $KEYCODEpure`
	echo NUM_KEYCODES $NUM_KEYCODES, was $LAST_NUM_KEYCODES
	if [ "$NUM_KEYCODES" != "$LAST_NUM_KEYCODES" ]
	then
		LAST_EVENT="$SEC"
		LAST_NUM_KEYCODES=$NUM_KEYCODES
		echo "Hooray! we have eleminated some keycodes"
	fi
    fi
    if [ ! -z "$AND_THEN_QUIT" ]
    then
		RESULT=1
		echo RR 1
		return 1
    fi
    if [ ! -z "$LAST_CORE" ]
    then
      rm "$LAST_CORE"
    fi
    LAST_CORE="$GDB.core"
  else
    if ! test -z "$BAK"
    then
	  echo will erase '$BAK/*'="'$BAK/*'"
	  sleep 2
          rm -rf $BAK/*
    	  mv $OUTDIR/$SEC.* $BAK/
    else
          echo "BAK is null"
    fi
    if [ ! -z "$TAIL_LINES" ]
     then
        echo TTL3 $TAIL_LINES
	echo MAX_TAIL_LINES "$MAX_TAIL_LINES"
	TAIL_LINES=$(($TAIL_LINES*2))
        echo TTL4 $TAIL_LINES
	if [ "$TAIL_LINES" -ge "0$MAX_TAIL_LINES" -a ! -z "$MAX_TAIL_LINES" ]
	then
		echo Giving up because $TAIL_LINES '>' $MAX_TAIL_LINES
		echo Irreproducible > $OUTDIR/Irreproducible
		full_exit
	fi
    fi
    if [ ! -z "$AND_THEN_QUIT" ]
    then
		RESULT=0
		echo RR 0
		return 0
    fi

    echo TTL2 $TAIL_LINES
  fi
}

test_exist () {
	if [ ! -e "$1" ]
	then    
	        echo "$1" does not exist!
		full_exit 1
	fi
}

assert () {
	if ! "$@"
	then 
		echo "Assertion '$*' Failed!"
		full_exit 1
	fi
}
	


#####################################################
# MAIN
#####################################################

#Start basic sanity checks

autolyx_main () {

if [ ! -e "$EXE_TO_TEST" ]
then
	echo "$EXE_TO_TEST" does not exist
	echo Cannot proceed
	exit
fi

assert which xvkbd
assert which wmctrl

if ! wmctrl -l > /dev/null 
then
	echo autolyx: cannot run wmctrl -l
	exit
fi

test_exist "$EXE_TO_TEST"
test_exist "$DIRNAME0/keytest.py"

if ! test -z "`pylint -e $DIRNAME0/keytest.py`" 
then
	echo  "$DIRNAME0/keytest.py" has python errors
	exit
fi

ensure_cannot_print

#End basic sanity checks

if [ ! -z "$1" ]
then
	REPLAYFILE=$1
fi

if [ ! -z "$REPLAYFILE" ]
then
	echo REPLAYMODE
	OUTDIR="$REPLAYFILE.replay/"
	mkdirp $REPLAYFILE.replay/ || full_exit
	export KEYTEST_INFILE=$REPLAYFILE
	if [ ! -e "$REPLAYFILE" ]
	then
		echo "$REPLAYFILE" does not exist
		echo exiting
		full_exit 1
	fi
else
	do_queued_replays
	echo RANDOM MODE
fi

get_pid [0-9].x-session-manager"$" x
export X_PID=`get_pid [0-9].x-session-manager x`
echo X_PID $X_PID

export TAIL_LINES=$TAIL_LINES
echo TL $TAIL_LINES


BAK="$OUTDIR/backup"
mkdirp $BAK

	


#rename other windows to avoid confusion.
wmctrl -N __renamed__ -r lyx
wmctrl -N __renamed__ -r lyx
wmctrl -N __renamed__ -r lyx
wmctrl -N __renamed__ -r lyx
export PATH=`cd $DIRNAME0; pwd`/path:$PATH

if [ ! -z "$1" ]
then
  SEC=`date +%s`
  export MAX_DROP=0
  if [ ".$SCREENSHOT_OUT." = ".auto." ]
  then
	echo SCREENSHOT_OUT was $SCREENSHOT_OUT.
	export SCREENSHOT_OUT="$OUTDIR/$SEC.s"
	echo SCREENSHOT_OUT is $SCREENSHOT_OUT.
	#exit
  fi
  export RESULT=179
  do_one_test #| tee do_one_test.log
  RESULT="$?"
  echo Ressult $RESULT

  kill `list_all_children.sh $$`
  sleep 0.1
  kill -9 `list_all_children.sh $$`

  exit $RESULT
  
  #echo done ; sleep 1
  full_exit
fi



(
echo TTL $TAIL_LINES

LAST_EVENT=`date +%s` # Last time something interesting happened. If nothing interesting has happened for a while, we should quit.

ensure_cannot_print
echo X_PID $X_PID
export X_PID=`get_pid [0-9].x-session-manager"$" x`
echo PATH $PATH
while true
do
 echo Currently running autolyx PID=$$
 if [ ! -z "$TAIL_LINES" ] 
 then
  echo TAIL_LINES: "$TAIL_LINES"
  TAIL_FILE=$OUTDIR/tail_"$TAIL_LINES"
  tail -n "$TAIL_LINES" "$REPLAYFILE" > $TAIL_FILE
  KEYTEST_INFILE=$TAIL_FILE
  MAX_DROP=0
 else
  MAX_DROP=0.2
 fi
 export MAX_DROP
  SEC=`date +%s`
 if [ -z "$TAIL_LINES" -a ! -z "$REPLAYFILE" ] 
 then
	echo Boredom factor: $SEC-$LAST_EVENT'=' $(($SEC-$LAST_EVENT))
	if [ $(($SEC-$LAST_EVENT)) -gt $BORED_AFTER_SECS ]
	then
		echo
		echo Is is now $SEC seconds
		echo The last time we managed to eliminate a keycode was at $LAST_EVENT
		echo We get bored after $BORED_AFTER_SECS seconds
		echo Giving up now.
		echo
		echo $LAST_CRASH_SEC > $OUTDIR/Finished
		SEC=$LAST_CRASH_SEC #I used SEC in place of LAST_CRASH_SEC. Here is a quick fix.
		#make screenshots
		if [ `cat $OUTDIR/$SEC.KEYCODEpure | wc -l` -lt 40 ] # If many keycodes, dont bother trying to make screenshots
		then
			echo "Making screenschot of $OUTDIR/$SEC.KEYCODEpure"
			test -e $OUTDIR/$SEC.KEYCODEpure || echo "DOES NOT EXIST: $OUTDIR/$SEC.KEYCODEpure"
			(SCREENSHOT_OUT="auto" ./doNtimes.sh 9 ./reproduce.sh $OUTDIR/$SEC.KEYCODEpure ; echo $f )
		fi
		mkdirp $OUTDIR/final
		mkdirp $OUTDIR/final_cp
		#chmod g+w $OUTDIR/final
		ln $OUTDIR/$SEC* $OUTDIR/final || cp $OUTDIR/$SEC* $OUTDIR/final
		cp $OUTDIR/$SEC* $OUTDIR/final_cp/
		cp -rv $OUTDIR/$SEC.replay $OUTDIR/final_cp/
		CONFIRM_FILE=`calc_confirm_file`
		echo Reproducible > "$CONFIRM_FILE"
		
		
		full_exit
	fi
 else
	do_queued_replays
 fi
 do_one_test
done
kill_all_children $$
) 2>&1 |tee $OUTDIR/log
kill_all_children $$
}
