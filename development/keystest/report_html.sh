#!/bin/bash
LT=development/keystest

GEOM=320x200
convert -normalize -scale $GEOM -quality $QUALITY $f $GEOM/$f

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

OUT=$LT/out/html
mkdir -p $OUT
rm $OUT/index*.html
ls $OUT/*.html

CPP_HTML_DIR_REL=cpp_html
CPP_HTML_DIR=$out/cpp_html

strings $LT/out/GDB.* > $LT/out/GDBs 
grep "#$UNIQUE_LINE " $LT/out/GDBs > $LT/out/list
cat $LT/out/list | sed 's/0x[^ )]*[ )]/.*/g'  | sort | uniq | tee $LT/out/listuniq

echo '<html>' >> $OUT/indexreport.html

tidy_keycode () {
while	read -r k
do
	if [ "$k" = Raiselyx ] 
	then
		echo -n '\[!Raiselyx]'
	elif  [ "$k" = Loop ] 
	then
		echo -n '\[!Loop]'
	else
		echo -n "$k" | sed s/^KK:\ //
	fi
done
}

html_keycode() {
	  cat "$f_base.KEYCODEpure" | tidy_keycode 
	  echo -n '<font color=gray>'
	  cat "$f_base.KEYCODEpure+" | tidy_keycode 
	  echo -n '</font><br/>'
}

gdb2html() {
cat $g | sed 's/&/&amp/g' | sed 's/</&lt/g'  | while read -r l
do
	c=`echo $l | grep -i -o "at [[:alnum:].]*:[0-9]*"`
	if [ -z "$c" ]
	then
		echo -- "$l" | sed s/--//
	else
		cpp=`echo "$c" | sed s/at\ // | sed s/:.*//g`
		lineno=`echo "$c" | sed s/.*://g`
		echo $cpp,$lineno 1>&2
		#if [ -e "$CPP_HTML_DIR/$cpp.html" ]
		if true
		then
			echo "$l" | sed "s/$c/<a href=$CPP_HTML_DIR_REL\/$cpp.html\#line$lineno>$c<\/a>/"
		else
			echo "$l"
		fi
	fi
done | sed 's/^/<br\/>/'
}

for file in `find $LT/out/ | grep replay/last_crash_sec`
do
 echo last_crash_sec file: $file
 SEC=`cat $file`
 echo SEC $SEC
 f_base=`echo $file | sed s/last_crash_sec/$SEC/g`
 NUM_KEYCODES=`wc -l < "$f_base.KEYCODEpure"`
 echo NUM_KEYCODES=$NUM_KEYCODES...
 if [ "$NUM_KEYCODES" -lt 80 ]  
 then
  echo f_base $f_base
  f=$f_base.GDB
  echo $f
  g=$f.short
  #if egrep '([Ii][Nn] .*[:[:alnum:]][:][0-9]*[^0-9]|#0 | signal SIG[^T])' -A9999  <$f >$g
  egrep '([Ii][Nn] .*[:[:alnum:]][:][0-9]*[^0-9]|#0 | signal SIG[^T])' -A9999  <$f >$g
  if true
  then
	IN_AT=`egrep -o '([Ii][Nn]|at) ([:lower:]* )?[:_[:alnum:]]*(::[:_[:alnum:]]*|:[0-9]*)' <$f | grep -v lyx_exit | grep -v [Aa]ssert | head -n 1  `
	SIGNAL=`grep -o ' signal SIG[[:alnum:]_]*[, ]' <$g | sed s/[^[:upper:]]//g | head -n 1`
	TITLE="$SIGNAL $IN_AT"
	TITLE_=`echo $TITLE|sed s/[^[:alnum:]]/_/g`
	INDEX="index.html"
	echo TITLE $TITLE
	echo INDEX $INDEX
	echo NEW $INDEX
	echo '<html>' >> $LT/out/$INDEX
	echo -n "<a href=\"$SEC.html\">$TITLE</a> " >> $OUT/indexreport.html
	html_keycode >> $OUT/indexreport.html
	echo >> $OUT/indexreport.html
	( echo '<html>'
	  echo "<h1>$TITLE</h1>"
	  html_keycode
	  echo "<a href=$SEC.KEYCODE>KEYCODES</a> "
	  echo "<a href=$SEC.GDB>GDB</a><br/>"
	  echo "<a href=$SEC.GDB.png><img src=$SEC.small.png/></a><br/><br/>"
	  gdb2html
	) > $OUT/$SEC.html
	echo '<a href="'"$SEC.html"'">'$SEC'</a><br/>' >> $OUT/$INDEX
	echo '<a href="'"$SEC.html"'">'$SEC'</a><br/>'
	if [ ! -e $OUT/$SEC.small.png ]
	then  
		convert -normalize -scale $GEOM $f.png $OUT/$SEC.small.png -quality 85
	fi
	chmod a+r $f $f.png $f_base.KEYCODE $f_base.html $OUT/indexreport.html
	ln $f $f.png $f_base.KEYCODE $f_base.html $OUT/
  fi
 fi
done
mv $OUT/indexreport.html $OUT/indexreport.html.bak
echo "<html>" >> $OUT/indexreport.html
echo "<h1>List of bugs found</h1>" >> $OUT/indexreport.html
sort -k 2 -t '>' < $OUT/indexreport.html.bak >> $OUT/indexreport.html

$OUT/
exit
