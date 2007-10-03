#!/bin/bash

qt=/usr/include/qt4
build=../../../build

inc="-I$qt -I$qt/QtCore -I$qt/QtGui"
inc="$inc -I$build/src"
inc="$inc -I$build/src/frontends/qt4"
inc="$inc -I../../boost"
inc="$inc -I../../src"
inc="$inc -I../../src/frontends"
inc="$inc -I../../src/frontends/controllers"
inc="$inc -I../../src/frontends/qt4"

s=0
for i in `find ../.. -name *.cpp` ; do
	#echo $i
	#echo "g++ $inc -DQT_NO_STL -E $i"
	#g++ $inc -DQT_NO_STL -E $i > tmp/`basename $i`
	l=`g++ $inc -DQT_NO_STL -E $i | wc -l`
	s=$[s + l]
	printf "%10d  %-40s\n" $l $i
done
echo "Total: $s"

