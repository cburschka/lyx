#!/bin/bash

listall () {
PID_LIST="$*"
while [ ! -z "$PID_LIST" ]
do
	#PID_LIST=`ps -o pid=  --ppid "$PID_LIST"| sed 's/^ *//g'` 
	PID_LIST=`ps -o pid=  --ppid "$PID_LIST"` 
	PID_LIST=`echo $PID_LIST` 
	#PID_LIST=`ps -o pid=  --ppid "$PID_LIST"` 
	echo $PID_LIST
done
}

kill_all_children () {
	kill `listall "$*"`
	sleep 0.1
	kill -9 `listall "$*"`
}

if [ "$1" = "kill"  ]
then
	shift
	kill_all_children "$*"
else
	listall "$*"
fi
