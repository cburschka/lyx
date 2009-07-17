#!/bin/bash
#development/keystest/lyx_make.sh [--update]
#This script runs LyX (and possibly updates), starts spamming it with hundreds of 
#keypresses, and logs all output, including backtraces to development/keystest/out/GDB .
#Use report.sh to generated the more useful bug reports in development/keystest/out/{or}*

LT=development/keystest

if ! [ -d $LT ]; then
 echo lyx_make.sh is supposed to be in the root of the lyx svn tree, eg. development/keystest/lyx_make.sh. 
fi

mkdir -p $LT/out
if which wmctrl xvkbd bash xterm python xclip
then 

 if [ a"$1" == a--update ]; then
  svn up
  ./autogen.sh && ./configure --enable-debug -- && nice -18 make && 
	if [ ! a"$2" == a"0"  ]
	then
		bash $LT/autolyx 
	fi
 else
  DBG=`src/lyx --version 2>&1 | grep C++.Compiler.flags|grep -- -g`;
  if [ -z "$DBG" ]; then echo Wrong build of LyX binary. ; exit; fi

  bash $LT/autolyx 
 fi

else
 echo NEEDS the following packages: 
 echo wmctrl xvkbd bash xterm python xclip
fi
