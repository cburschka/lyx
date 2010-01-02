#!/bin/bash
KT=`dirname "$0"`

. "$KT/shared_functions.sh"

N=$1
shift

echo doNtimes .$N. "$@"

i=0
for intr in `yes | head -n$N`
do
	i=$(($i+1))
	echo 'TRY#' $i
	if ! "$@"
	then
		echo TRIES_REQUIRED: $i
		kill_all_children $$
		exit 1
	fi
done

kill_all_children $$
echo DONE $N TIMES

