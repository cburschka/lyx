#!/bin/bash
#LT=development/keystest
LT=`dirname $0`
. $LT/shared_variables.sh

GEOM=320x200
QUALITY=85
if [ -z "$KEYCODE_DIR" ]
then
	KEYCODE_DIR=$ROOT_OUTDIR
fi
#convert -normalize -scale $GEOM -quality $QUALITY $f $GEOM/$f

UNIQUE_LINE=1

OUT=$LT/$OUT_NAME/html
URL_OF_OUT=http://gmatht.homelinux.net/xp/$OUT_NAME/html/
mkdir -p $OUT
rm $OUT/index*.html
ls $OUT/*.html

CPP_HTML_DIR_REL=cpp_html
CPP_HTML_DIR=$OUT/cpp_html

LYX_VER=`src/lyx -version 2>&1 | head -n1 | sed s/^LyX\ // | sed s/\ .*// | sed s/1svn/0/ | sed s/2svn/1/ | sed s/3svn/2/ | sed s/4svn/3/ | sed s/5svn/4/ | sed s/6svn/5/ | sed s/7svn/6/ | sed s/8svn/7/ | sed s/9svn/8/`
BOILER_PLATE=`cd src ; svn info 2>&1 | grep Revision`

strings $LT/$OUT_NAME/GDB.* > $LT/$OUT_NAME/GDBs 
grep "#$UNIQUE_LINE " $LT/$OUT_NAME/GDBs > $LT/$OUT_NAME/list
cat $LT/$OUT_NAME/list | sed 's/0x[^ )]*[ )]/.*/g'  | sort | uniq | tee $LT/$OUT_NAME/listuniq

echo '<html>' >> $OUT/indexreport.html

list_keycode_files () {
#echo for f in  $OUT_NAME/*y/last_crash_sec $OUT_NAME/toreplay/replayed/*y/last_crash_sec
for f in  $OUT_NAME/*y/last_crash_sec $OUT_NAME/toreplay/replayed/*y/last_crash_sec $OUT_NAME/toreproduce/replayed/*y/last_crash_sec  $OUT_NAME/toreplay/*y/last_crash_sec $OUT_NAME/toreplay/final/*y/last_crash_sec
#for f in  $OUT_NAME/*y/*y/last_crash_sec $OUT_NAME/toreplay/replayed/*y/*y/last_crash_sec  $OUT_NAME/toreplay/*y/*y/last_crash_sec $OUT_NAME/toreplay/final/*y/last_crash_sec
do
        keycode_file=$(echo $f | sed s/last_crash_sec/$(cat $f).KEYCODEpure/)
	if test -e $keycode_file.replay/last_crash_sec
	then
		f=$keycode_file.replay/last_crash_sec
        	keycode_file=$(echo $f | sed s/last_crash_sec/$(cat $f).KEYCODEpure/)
	fi
        echo $keycode_file
done
}

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


trac_keycode() {
	  echo -n "'''"
	  cat "$f_base.KEYCODEpure" | tidy_keycode 
	  echo -n "'''"
	  #echo -n '""'
	  #echo -n '<font color=gray>'
	  cat "$f_base.KEYCODEpure+" | tidy_keycode 
	  #echo -n '</font>'

}

line2url() {
	echo "$@" | sed 's/&/%26/g' | sed 's/"/%22/g' | sed 's/#/%23/g' | sed 's/;/%3B/g'
}

text2url() {
	#echo "$@" | 
	while read -r L
	do
		#echo -n `echo "$L" | line2url`'%0A'
		echo -n `line2url "$L"`'%0A'
	done
}

report_bug_url () {
	#http://www.lyx.org/trac/newticket?description=bar%0Abar2&summary=foo%13oo&keywords=crash&version=1.6.5
 	#http://www.lyx.org/trac/newticket?summary=foo%13oo&description=bar%0Abar2&version=1.6.5&keywords=crash
	#ver=`echo $3 | sed s/1svn/0/ | sed s/2svn/1/ | sed s/3svn/2/ | sed s/4svn/3/ | sed s/5svn/4/ | sed s/6svn/5/ | sed s/7svn/6/ | sed s/8svn/7/ | sed s/9svn/8/`
	#echo 'http://www.lyx.org/trac/newticket?summary='`line2url "$1"`'&description='`text2url "$2"`'&version='"$LYX_VER"'&keywords=crash'
	DESC=`get_description | text2url`
	#DESC=`text2url "$DESC"`
	if echo $DESC | grep SIGSEGV > /dev/null
	then
		KEYWORDSTR="&keywords=crash"
	else
		if echo $DESC | grep SIGABRT > /dev/null
		then
			KEYWORDSTR="&keywords=assertion"
		elif echo $DESC | grep SIGSEGV > /dev/null
		then
			KEYWORDSTR="crash"
		fi
	fi
	echo 'http://www.lyx.org/trac/newticket?summary='`line2url "$TITLE"`"&description=$DESC&version=$LYX_VER$KEYWORDSTR"
	#echo 'http://www.lyx.org/trac/newticket?description='"$1"'&summary=foo%13oo&keywords=crash&version=1.6.
}

search_bug_url () {
	#http://www.lyx.org/trac/search?q=Foo%20bar
	echo 'http://www.lyx.org/trac/search?q='`line2url "$TITLE"`
}

gdb2html() {
echo g $g'<br/>'
head $g -n 100 | sed 's/&/\&amp;/g' | sed 's/</\&lt;/g' | sed 's/  /\&nbsp /g'  | while read -r l
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
		if [ -e "$CPP_HTML_DIR/$cpp.html" ]
		#if true
		then
			#echo "$l" | sed "s/$c/<a href=$CPP_HTML_DIR_REL\/$cpp.html\#line$lineno>$c<\/a>/"
			echo "$l" | sed "s?$c?<a href=$CPP_HTML_DIR_REL\/$cpp.html\#line$lineno>$c<\/a>?"
		else
			echo "$l"
		fi
	fi
done | sed 's/^/<br\/>/'
}

get_description() {
	echo "Keycodes: "
	trac_keycode
	ii=1
	echo
	echo To reproduce:
	cat "$f_base.KEYCODEpure" "$f_base.KEYCODEpure+" | while read -r L
	do
		echo "$ii) ... ($L)" 
		ii=$((ii+1))
	done

	echo
	echo Arch: `arch`
	svn_revision $f_base
	echo '{{{'
	cat $g
	echo '}}}'
	echo
	echo For more info see the "[[$URL_OF_OUT/$HTML_NAME Keytest Report]]"
}

make_cpp_html() {
#This is ugly. At the moment all bug reports in the same set reference the same files, which could lead to confusion if they are from differnt versions of lyx. Do not mix different versions until this is fixed. However we may not want to "fix" this when it may be better to always use a new outdir for each version and just reproduce the old bugs so they don't get lost.
if ! test -e $ROOT_OUTDIR/html/cpp_html
then 
	(mkdir -p $ROOT_OUTDIR/html/cpp_html/ &&
	cd $ROOT_OUTDIR/html/cpp_html/ &&
	for f in `find ../../../src/ -iname '*.cpp' ; find ../../../src/ -iname '*.h'` ; do  g=`basename $f`; c2html -n < $f > $g.html ; echo $f  ; done)
fi
}

svn_revision() {
#| grep ^Revision | sed s/'Revision: '/r/
echo KEYCODE $1 ... `dirname $1` 1>&2
cat `dirname $1`/last_crash_sec.info.svn | grep ^Revision
echo _KEYCODE $KEYCODE ... 1>&2
}




echo beginning
make_cpp_html
#for file in `find $LT/$OUT_NAME/ -anewer $LT/$OUT_NAME/html | grep replay/last_crash_sec`
#for file in `find $KEYCODE_DIR | grep save/.*KEYCODEpure`
#for file in `find $KEYCODE_DIR -anewer oldfile | grep save/.*KEYCODEpure$ | head -n4`
#for file in `ls $KEYCODE_DIR/*/final/*KEYCODEpure`
#for file in `ls $KEYCODE_DIR/*/final/*/*KEYCODEpure`
list_keycode_files
echo END OF KEYCODE FILES
for file in `list_keycode_files`
do
 echo FILE $file
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
 if [ "$NUM_KEYCODES" -lt 20 ]  
 then
  echo f_base $f_base
  f=$f_base.GDB
  echo $f
  g=$f.short
  #egrep '([Ii][Nn] .*[:[:alnum:]][:][0-9]*[^0-9]|#0 | signal SIG[^T]| ASSERTION .* VIOLATED IN )' -A9999  <$f >$g
  egrep '([Ii][Nn] .*[:[:alnum:]][:][0-9]*[^0-9]|#0 | signal SIG[^T]| ASSERTION .* VIOLATED IN )' -A9999  <$f >$g
  if true
  then
	IN_AT=`egrep -o '([Ii][Nn]|at) ([:lower:]* )?[:_[:alnum:]]*(::[:_[:alnum:]]*|:[0-9]*)' <$f | grep -v lyx_exit | grep -v [Aa]ssert | head -n 1  `
	SIGNAL=`grep -o ' signal SIG[[:alnum:]_]*[, ]' <$g | sed s/[^[:upper:]]//g | head -n 1`
	TITLE="$SIGNAL $IN_AT"
	if grep ' ASSERTION .* VIOLATED IN ' $g
	then
		TITLE=`grep -o ' ASSERTION .* VIOLATED IN .*$' $g`
	fi
	TITLE_=`echo $TITLE|sed s/[^[:alnum:]]/_/g`
	INDEX="index.html"
	HTML_NAME=$SEC.html # name of html report for this bug
	echo TITLE $TITLE
	echo INDEX $INDEX
	echo NEW $INDEX
	echo '<html>' >> $LT/$OUT_NAME/$INDEX
	(echo -n "<br/><a href=\"$SEC.html\">$TITLE</a> " 
	html_keycode 
	#echo -n " [<a href=\"`search_bug_url`\">search</a>] [<a href=\"`report_bug_url`\">report</a>] <a href=\"$SEC.screenshot.html\">screenshots</a>" 
	echo -n " [<a href=\"`search_bug_url`\">search</a>] [<a href=\"`report_bug_url`\">report</a>]" 
	if ls $f_base.s*.png > /dev/null
	then
		echo -n " <a href=\"$SEC.screenshot.html\">screenshots</a>" 
	fi
	echo 
	#echo -n '<br/> '
	echo ) >> $OUT/indexreport.html
	echo -n '<br> '>> $LT/$OUT_NAME/$INDEX
	echo >> $LT/$OUT_NAME/$INDEX

	( echo '<html>'
	  echo "<title>$TITLE</title>"
	  html_keycode
	  echo "<br/><a href=$SEC.KEYCODE>KEYCODES</a> "
	  echo " <a href=$SEC.KEYCODEpure>pure</a> "
	  echo "<a href=$SEC.GDB>GDB</a>"
 	  if ls $f_base.s*.png > /dev/null
          then
                 echo "<a href=\"$SEC.screenshot.html\">screenshots</a>" 
	  else
		 echo '(no screenshots)'
 	  fi
	  echo "<br/>"
         
        #echo -n '<br/> '

	  #echo "<a href=$SEC.GDB.png><img src=$SEC.small.png/></a><br/><br/>"
	  #echo "<a href=$SEC.screenshot.html><img src=$SEC.small.png/></a><br/><br/>"
	  gdb2html
	  (echo ----
	  svn_revision $f_base
	  echo arch `arch`
	  dpkg -l | egrep '(libqt4-dbg|libc6 |libboost-dev )'
	  lsb_release -a) | sed 's/&/\&amp;/g' | sed 's/</\&lt;/g' | sed 's/  /\&nbsp /g' | sed 's/^/<br\/>/'
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
echo '<p>Please, do not "report" bugs without searching for them first. Also make sure to fill out the "To reproduce" section before pressing the "Create ticket" button</p>' >> $OUT/indexreport.html
sort -k 2 -t '>' < $OUT/indexreport.html.bak >> $OUT/indexreport.html

firefox $OUT/indexreport.html
#google-chrome $OUT/indexreport.html
