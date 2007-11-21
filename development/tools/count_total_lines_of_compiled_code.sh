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
t=0
#for i in `find ../../src/frontends/qt4 -name '*.cpp'` ; do
#for i in `find ../../src/insets -name '*.cpp'` ; do
#for i in `find ../../src/mathed -name '*.cpp'` ; do
#for i in `find ../../src/support -name '*.cpp'` ; do
#for i in `find ../../src/graphics -name '*.cpp'` ; do
#for i in `find ../../src/graphics -name '*.cpp'` ; do
#for i in ../../src/*.cpp ; do
for i in `find ../.. -name '*.cpp'` ; do
	#echo $i
	#echo "g++ $inc -DQT_NO_STL -E $i"
	#g++ $inc -DQT_NO_STL -E $i > tmp/`basename $i`
	l=`g++ $inc -DQT_NO_STL -E $i | wc -l`
	f=`cat $i | wc -l`
	s=$[s + l]
	t=$[t + f]
	printf "%10d %10d  %-40s\n" $l $f $i
done
echo "Total: compiled: $s  real: $t  ratio:" $[s / t]

