#!/bin/bash
#LT=development/keystest
LT=`dirname $0`

GEOM=320x200
QUALITY=85
if [ -z "$KEYCODE_DIR" ]
then
	KEYCODE_DIR=out
fi
#convert -normalize -scale $GEOM -quality $QUALITY $f $GEOM/$f

UNIQUE_LINE=1

OUT=$LT/out/html4
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
		echo -n "$k" | sed 's/^KK: //'
	fi
done
}

html_keycode() {
	  cat "$f_base.KEYCODEpure" | tidy_keycode 
	  echo -n '<font color=gray>'
	  cat "$f_base.KEYCODEpure+" | tidy_keycode 
	  echo -n '</font>'

}

gdb2html() {
echo g $g
cat $g | sed 's/&/&amp/g' | sed 's/</&lt/g'  | while read -r l
do
	#c=`echo $l | grep -i -o "at [[:alnum:]./]*:[0-9]*"`
	#We may want to support slashes later, but we'd have to support browsing
	#Qt source for that to be useful for LyX
 
	c=`echo $l | grep -i -o "at [[:alnum:].]*:[0-9]*"`
	if [ -z "$c" ]
	then
		echo -- "$l" | sed s/--//
	else
		cpp=`echo "$c" | sed 's/at //' | sed 's/:.*//g'`
		#cpp=`basename $cpp`
		echo cpp $cpp
		lineno=`echo "$c" | sed 's/.*://g'`
		echo $cpp,$lineno 1>&2
		#if [ -e "$CPP_HTML_DIR/$cpp.html" ]
		if true
		then
			#echo "$l" | sed "s/$c/<a href=$CPP_HTML_DIR_REL\/$cpp.html\#line$lineno>$c<\/a>/"
			echo "$l" | sed "s?$c?<a href=$CPP_HTML_DIR_REL\/$cpp.html\#line$lineno>$c<\/a>?"
		else
			echo "$l"
		fi
	fi
done | sed 's/^/<br\/>/'
}

echo beginning
#for file in `find $LT/out/ -anewer $LT/out/html | grep replay/last_crash_sec`
#for file in `find $KEYCODE_DIR | grep save/.*KEYCODEpure`
for file in `find $KEYCODE_DIR -anewer oldfile | grep save/.*KEYCODEpure$`
do
 lcs_file=`echo $file | sed 's/save\/.*//g'`last_crash_sec
 echo last_crash_sec file: $lcs_file
 SEC2=`cat $lcs_file`
 SEC=`basename $file | sed s/[.].*$//g`
 echo SEC .$SEC. .$SEC2.
 #if [ ! $SEC -eq $SEC2 ]
 #then
	#break
 #fi
 echo SEC $SEC
 #f_base=`echo $file | sed s/last_crash_sec/$SEC/g`
 f_base=`dirname $file`'/'$SEC
 echo f_base $f_base
 NUM_KEYCODES=`wc -l < "$f_base.KEYCODEpure"`
 echo NUM_KEYCODES=$NUM_KEYCODES...
 if [ "$NUM_KEYCODES" -lt 80 ]  
 then
  echo f_base $f_base
  f=$f_base.GDB
  echo $f
  g=$f.short
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
	if ls $f_base.s*.png 
	then
		echo -n "<a href=\"$SEC.screenshot.html\">screenshots</a>" >> $OUT/indexreport.html
	fi
	echo -n '<br/> '>> $OUT/indexreport.html
	echo >> $OUT/indexreport.html
	echo -n '<br> '>> $LT/out/$INDEX
	echo >> $LT/out/$INDEX

	( echo '<html>'
	  echo "<h1>$TITLE</h1>"
	  html_keycode
	  echo "<a href=$SEC.KEYCODE>KEYCODES</a> "
	  echo "<a href=$SEC.GDB>GDB</a>"
	  #echo "<a href=$SEC.GDB.png><img src=$SEC.small.png/></a><br/><br/>"
	  echo "<a href=$SEC.screenshot.html><img src=$SEC.small.png/></a><br/><br/>"
	  gdb2html
	) > $OUT/$SEC.html
	echo '<a href="'"$SEC.html"'">'$SEC'</a><br/>' >> $OUT/$INDEX
	echo '<a href="'"$SEC.html"'">'$SEC'</a><br/>'
	if [ ! -e $OUT/$SEC.small.png ]
	then  
		convert -normalize -scale $GEOM $f.png $OUT/$SEC.small.png -quality 85
	fi
	chmod a+r $f $f.png $f_base.KEYCODE* $f_base.html $OUT/indexreport.html
	ln $f $f.png $f_base.KEYCODE* $f_base.html $f_base.s*.png  $OUT/
	if ls $f_base.s*.png 
	then
		$LT/make_screenshot_html.py $OUT $SEC > $OUT/$SEC.screenshot.html
	fi
  fi
 fi
done
mv $OUT/indexreport.html $OUT/indexreport.html.bak
echo "<html>" >> $OUT/indexreport.html
echo "<h1>List of bugs found</h1>" >> $OUT/indexreport.html
sort -k 2 -t '>' < $OUT/indexreport.html.bak >> $OUT/indexreport.html

firefox $OUT/indexreport.html
