#!/bin/bash
for i in "$@" ; do
	echo "#include <$i>"  > 1.cpp
	inc='-I. -I/suse/usr/src/lyx/trunk/boost -I/usr/include/qt4/QtCore -I/usr/include/qt4'
	l=`g++ $inc -DQT_NO_KEYWORDS -DQT_NO_STL -E 1.cpp | wc -l`
	printf "%-40s: %d\n" $i $l
done

