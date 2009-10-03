#!/bin/bash
KT=`dirname $0`
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
		exit 1
	fi
done

echo DONE $N TIMES
