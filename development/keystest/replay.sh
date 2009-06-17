#!/bin/bash
# This script starts LyX, and restarts LyX if it is closed
# it logs all output, including backtraces to development/keystest/out/GDB

#rename other windows to avoid confusion.
DIRNAME0=`dirname "$0"`
OUTDIR="$DIRNAME0/out"
WAITSECS=20
INFILE="$1"
LINES_TO_INCLUDE=8

if [ ! -e "$INFILE" ]
then
	echo cannot find "$INFILE"
	exit
fi

killall lyx

wmctrl -N __renamed__ -r lyx
wmctrl -N __renamed__ -r lyx
wmctrl -N __renamed__ -r lyx
wmctrl -N __renamed__ -r lyx

while ! grep " signal SIG[^T]" "$INFILE.new_gdb"
do
  SEC=`date +%s`
  echo GDB---------------- >> "$INFILE.new_gdb.bak"
  cat "$INFILE.new_gdb" >> "$INFILE.new_gdb.bak"
  echo LINES_TO_INCLUDE $LINES_TO_INCLUDE
  #cat $NUMLINES
  tail -n $LINES_TO_INCLUDE < $INFILE > $INFILE.new_key
  NUMLINES=`wc -l "$INFILE.new_key"`
  echo NUMLINES $NUMLINES
  ( i=0
    echo Waiting $WAITSECS before starting replay
    sleep $WAITSECS 
    echo Starting replay
    wmctrl -R LyX && xvkbd -xsendevent -text '\Afn'
    cat "$INFILE.new_key" |
    while read -r l
    do
      wmctrl -R LyX && xvkbd -xsendevent -text "$l"
      #echo -- "$l"
      i=$(($i+1))
      echo $i/$NUMLINES
      sleep 0.1
    done
    echo FINISHED REPLAY
    sleep 4
    killall lyx
    sleep 2
    killall lyx -9
    echo FINISHED REPLAY and killed lyx
  ) &
  CHILD_PID="$!"
  echo "Starting Lyx"
  (echo "run
  bt" ; yes q) | gdb src/lyx 2>&1 | strings|  tee "$INFILE.new_gdb"
  kill $CHILD_PID
  sleep 2 kill -9 $CHILD_PID
  LINES_TO_INCLUDE=$(($LINES_TO_INCLUDE*2))
  
done
echo END
