#!/bin/bash
#This script runs LyX, starts spamming it with hundreds of 
#keypresses, and logs all output, including backtraces to $LT/out/GDB
#Use report.sh to generated the more useful bug reports in $LT/out/{or}*

LT=development/keystest

bash $LT/autolyx
