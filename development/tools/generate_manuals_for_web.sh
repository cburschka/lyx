#!/bin/bash

# file manuals_for_web.sh
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# \author Tommaso Cucinotta
# \author Pavel Sanda
#
# Full author contact details are available in file CREDITS

# Usage:
# manuals_for_web.sh (just launch from the LyX git root folder)
#
# optional env vars that customize behavior:
#   $LYX - abs path to LyX executable
#   $OUT - final folder where all converted manuals are placed
#   $TOC - final index.html that links all converted manuals
#   $TMP - temporary folder where all the conversion is done

MAIN_DOCS=${MAIN_DOCS:-"Intro Tutorial UserGuide Math Additional Customization Shortcuts LFUNs"}
LYX=${LYX:-${PWD}/src/lyx}
OUT=${OUT:-$HOME/web/lyxdoc}
TOC=${TOC:-lyxdoc/index.html}
TMP=${TMP:-$(mktemp -d --tmpdir lyx-docs-XXXX)}
USERDIR=${USERDIR:-$(mktemp -d --tmpdir lyx-home-XXXX)}

echo LYX=$LYX
echo OUT=$OUT
echo TOC=$TOC
echo TMP=$TMP

echo "Building docs: $MAIN_DOCS"

mycpus=$(grep -c processor /proc/cpuinfo)
function pexec {
    while [ $(pidof lyx | wc -w) -ge $[$mycpus*15/10] -o $(pidof lyx | wc -w) -ge $[$mycpus*15/10] ]; do
        sleep $(echo "$RANDOM * 5 / 32767" | bc -l)
    done
    echo -e "\e[39mRunning $@ ..."
    eval "$@ > /dev/null 2>&1 && echo -e ... \\\\e[32mDONE\\\\e[39m with $@ || echo -e ... \\\\e[31mERROR\\\\e[39m on $@" &
}

if [ ! -d lib/doc -o ! -f lib/doc/Intro.lyx ]; then
    echo "Run me from the LyX top dir"
    exit 1
fi

if [ ! -f $LYX ]; then
    echo "Cannot find the LyX executable at: $LYX"
    exit 1
fi

mkdir -p $TMP/lyxdoc

cp -a lib/doc/* $TMP
cd $TMP

echo ""
echo "Manuals being processed in $TMP"
echo "Index of built manuals available at: file://$TMP/$TOC"
echo ""

cat > $TOC <<EOF
<html>
<body>
<h1>LyX manuals</h1>

<table>
<tr><th>Manual</th><th>Browse Link(s)</th><th>PDF Download(s)</th></tr>
EOF

for m in $MAIN_DOCS; do
    echo "<tr><td>$m</td><td>" >> $TOC
    find . -name $m.lyx | while read f; do
	if [ ! -f lyxdoc/${f%%.lyx}.xhtml ]; then
	    pexec $LYX -userdir $USERDIR -E xhtml lyxdoc/${f%%.lyx}.xhtml $f;
	else
	    echo "Skipping already existing lyxdoc/${f%%.lyx}.xhtml"
	fi
	if echo $f | grep '/[a-zA-Z_]\+/' > /dev/null 2>&1; then
	    lang=$(echo $f | sed -e 's#.*/\([a-zA-Z_]\+\)/.*#\1#')
	else
	    lang=en
	fi
	echo "<a href=\"${f%%.lyx}.xhtml\">[$lang]</a>" >> $TOC
    done
    echo "</td><td>" >> $TOC
    find . -name $m.lyx | while read f; do
	if [ ! -f lyxdoc/${f%%.lyx}.pdf ]; then
	    pexec $LYX -userdir $USERDIR -E pdf lyxdoc/${f%%.lyx}.pdf $f;
	else
	    echo "Skipping already existing lyxdoc/${f%%.lyx}.pdf"
	fi
	if echo $f | grep '/[a-zA-Z_]\+/' > /dev/null 2>&1; then
	    lang=$(echo $f | sed -e 's#.*/\([a-zA-Z_]\+\)/.*#\1#')
	else
	    lang=en
	fi
	echo "<a href=\"${f%%.lyx}.pdf\">[$lang]</a>" >> $TOC
    done
    echo "</tr>" >> $TOC
done
echo "</table>" >> $TOC

echo "Waiting for child processes to complete..."
wait

echo ""
echo "Manuals processed in $TMP"
echo "Index of built manuals available at: file://$TMP/$TOC"
echo ""
