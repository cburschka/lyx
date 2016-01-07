#!/bin/bash
for i in "$@" ; do
	echo "#include <$i>"  > 1.cpp
	inc='-I. -I/suse/usr/src/lyx/trunk/boost -I/usr/include/qt4/QtCore -I/usr/include/qt4'
	g++ $inc -DQT_NO_STL -E 1.cpp | grep '^#' | cut -d '"' -f 2 | sort | uniq \
		| grep -v '^[<#]' | grep -v '^1.cpp$'
	#printf "%-40s: %d\n" $i $l
done

