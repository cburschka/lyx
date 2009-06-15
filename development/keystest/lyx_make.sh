#!/bin/bash
#This script updates LyX, runs LyX, starts spamming it with hundreds of 
#keypresses, and logs all output, including backtraces to development/keystest/out/GDB .
#Use report.sh to generated the more useful bug reports in development/keystest/out/{or}*

LT=development/keystest

if ! [ -d $LT ]; then
 echo lyx_make.sh is supposed to be in the root of the lyx svn tree, eg. development/keystest/lyx_make.sh. 
fi

mkdir -p $LT/out
if which wmctrl xvkbd bash xterm python
then 
 svn up
 ./autogen.sh && ./configure --enable-debug -- && nice -18 make && (bash $LT/autolyx & sleep 9 ; xterm -e python $LT/test.py)
else
 echo NEEDS the following packages: 
 echo wmctrl xvkbd bash xterm python
fi
