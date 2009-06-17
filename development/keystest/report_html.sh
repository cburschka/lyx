#!/bin/bash
LT=development/keystest

# echo 'grep "#1 " $LT/out/GDB | sed 's/0x[^ )]*[ )]/.*/g'  | sort | uniq' >> report.sh

UNIQUE_LINE=1
SRC=
if [ -e out/log ]
then	
   LT=.
fi

while [ ! -e $LT/out/log ]
do
  cd ..
  if [ `pwd` = '/' ]
  then
     exit
  fi
done

OUT=$LT/out
rm $OUT/index*.html
#rm $OUT/indexreport.html
ls $OUT/*.html

strings $LT/out/GDB.* > $LT/out/GDBs 
grep "#$UNIQUE_LINE " $LT/out/GDBs > $LT/out/list
#cat $LT/out/list | grep -o ' in [[:alnum:]:]* ' | sort | uniq| tee $LT/out/listuniq
#cat $LT/out/list | grep -o ' in [[:alnum:]:]* ' | sort | uniq| tee $LT/out/listuniq
cat $LT/out/list | sed 's/0x[^ )]*[ )]/.*/g'  | sort | uniq | tee $LT/out/listuniq

NUM_REPORTS=`wc -l < $LT/out/list`
echo NUM_REPORTS $NUM_REPORTS

echo > $LT/out/overview

echo '<html>' >> $OUT/indexreport.html

#cat $LT/out/listuniq | while read l 
for f in $LT/out/*GDB
do
  echo $f
  g=$f.short
  if egrep '([Ii][Nn] .*[:[:alnum:]][:][0-9]*[^0-9]|#0 | signal SIG)' -A9999  <$f >$g
  then
	#egrep '(([Ii][Nn]|at) .*[:[:alnum:]][:][0-9]*[^0-9]|#0 | signal SIG)' -A9999  <$f
	SEC=`echo $f | sed s/[^[:digit:]]//g`
	#IN_AT=`egrep -o '([Ii][Nn]|at) ([:lower:]* )[:_[:alnum:]]*(::[:_[:alnum:]]*|:[0-9]*)' <$f | head -n 1  `
	IN_AT=`egrep -o '([Ii][Nn]|at) ([:lower:]* )?[:_[:alnum:]]*(::[:_[:alnum:]]*|:[0-9]*)' <$f | head -n 1  `
	SIGNAL=`grep -o ' signal SIG[[:alnum:]_]*[, ]' <$g | sed s/[^[:upper:]]//g | head -n 1`
	TITLE="$SIGNAL $IN_AT"
	TITLE_=`echo $TITLE|sed s/[^[:alnum:]]/_/g`
	INDEX="index_$TITLE_.html"
	echo TITLE $TITLE
	echo INDEX $INDEX
	if [ ! -e  $LT/out/$INDEX ]
	then	
		echo NEW $INDEX
		echo '<html>' >> $LT/out/$INDEX
		echo -n '<a href="'"$INDEX"'">'"$TITLE</a>" >> $OUT/indexreport.html
		echo '[<a href="'"$SEC.html"'">'1'</a>]<br/>' >> $OUT/indexreport.html
	else
		echo exists $INDEX
	fi
	( echo '<html>'
	  echo "<h1>$TITLE</h1>"
	  echo "<img src=$SEC.GDB.png>$TITLE</img>"
#	  head -n 20 $g | txt2html -pi | sed 's/^/<br\/>/' | sed 's/<br\/>$//g'
	  #head -n 20 $g | txt2html -pi | sed 's/^#/#<br\/>/' 
#	  cat $g | txt2html -pi | sed 's/^#/#<br\/>/' 
	  cat $g | sed 's/&/&amp/g' | sed 's/</&lt/g' | sed 's/^/<br\/>/'
	) > $OUT/$SEC.html
	echo '<a href="'"$SEC.html"'">'$SEC'</a><br/>' >> $OUT/$INDEX
	echo '<a href="'"$SEC.html"'">'$SEC'</a><br/>' 
  fi
done
exit
