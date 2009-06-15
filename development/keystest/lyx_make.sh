#!/bin/bash
#This script updates LyX, runs LyX, starts spamming it with hundreds of 
#keypresses, and logs all output, including backtraces to LT/out/GDB
#Use report.sh to generated the more useful bug reports in LT/out/{or}*

#LYXDIR=LT/lyx-1.6.x-test
cd ~/lyx-1.6.x-test
mkdir -p LT/out
if which wmctrl xvkbd bash xterm python
then 
 #cd "$LYXDIR" || echo CANNOT FIND LT/lyx-1.6.x-test
 #cd "$LYXDIR" || exit
 svn up Makefile.am autogen.sh boost/ config/ configure.ac lib/ lyx.1in m4/ rename.sh src/
 export CFLAGS="-g"
 export CXXFLAGS="$CFLAGS"
 ./autogen.sh && ./configure && nice -18 make && (bash LT/autolyx & sleep 9 ; xterm -e python LT/test.py)
else
 echo NEEDS the following packages: 
 echo wmctrl xvkbd bash xterm python
fi
