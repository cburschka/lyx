#! /bin/sh
#
# \file lyxpreview2ppm.sh
# Copyright 2002 the LyX Team
# Read the file COPYING
#
# \author Angus Leeming, leeming@lyx.org
# with much help from David Kastrup, david.kastrup@t-online.de.
#
# This script takes a LaTeX file and generates PPM files, one per page.
# The idea is to use it with preview.sty from the preview-latex project
# (http://preview-latex.sourceforge.net/) to create small bitmap previews of
# things like math equations.

# preview.sty can be obtained from CTAN/macros/latex/contrib/supported/preview.

# This script, lyxpreview2ppm.sh, takes two arguments, the name of the file
# to be converted and a scale factor, used to ascertain the resolution of the
# generated image which is then passed to gs.

# If successful it will leave in dir ${DIR} a number of image files
# ${BASE}[0-9]\{3\}.ppm and a file ${BASE}.metrics containing info needed by
# LyX to position the images correctly on the screen. All other files ${BASE}*
# will be deleted.

# Three helper functions.
FIND_IT () {
	which ${EXECUTABLE} > /dev/null
	if [ $? -ne 0 ]; then
		echo "Unable to find \"${EXECUTABLE}\". Please install."
		exit 1
	fi
}

BAIL_OUT () {
	# Remove everything except the original .tex file.
	FILES=`ls ${BASE}* | sed -e "/${BASE}.tex/d"`
	rm -f ${FILES} texput.log
	exit 1
}

REQUIRED_VERSION () {
	echo "We require preview.sty version 0.73 or newer. You're using"
	grep 'Package: preview' ${LOGFILE}
}

# Preliminary check
if [ $# -ne 2 ]; then
	exit 1
fi

# We use latex, dvips and gs, so check that they're all there.
EXECUTABLE=latex; FIND_IT
EXECUTABLE=dvips; FIND_IT
EXECUTABLE=gs;    FIND_IT

# Initialise some variables.
DIR=`dirname $1`
BASE=`basename $1 .tex`

SCALEFACTOR=$2

TEXFILE=${BASE}.tex
LOGFILE=${BASE}.log
DVIFILE=${BASE}.dvi
PSFILE=${BASE}.ps
METRICSFILE=${BASE}.metrics

# LaTeX -> DVI.
cd ${DIR}
latex ${TEXFILE}
if [ $? -ne 0 ]; then
	echo "Failed: latex ${TEXFILE}"
	BAIL_OUT
fi

# Parse ${LOGFILE} to obtain bounding box info to output to ${METRICSFILE}.
# This extracts lines starting "Preview: Tightpage" and "Preview: Snippet".
grep -E 'Preview: [ST]' ${LOGFILE} > ${METRICSFILE}
if [ $? -ne 0 ]; then
	echo "Failed: grep -E 'Preview: [ST]' ${LOGFILE}"
	REQUIRED_VERSION
	BAIL_OUT
fi

# Parse ${LOGFILE} to obtain ${RESOLUTION} for the gs process to follow.
# 1. Extract font size from a line like "Preview: Fontsize 20.74pt"
# Use grep for speed and because it gives an error if the line is not found.
LINE=`grep 'Preview: Fontsize' ${LOGFILE}`
if [ $? -ne 0 ]; then
	echo "Failed: grep 'Preview: Fontsize' ${LOGFILE}"
	REQUIRED_VERSION
	BAIL_OUT
fi
# Use "" quotes in the echo to preserve newlines (technically IFS separators).
# The sed script strips out everything that won't form a decimal number from the
# line. It bails out after the first match has been made in case there are
# multiple lines "Preview: Fontsize". (There shouldn't be.)
LATEXFONT=`echo "${LINE}" | sed 's/[^0-9\.]//g; 1q'`

# 2. Extract magnification from a line like "Preview: Magnification 2074"
# If no such line found, default to MAGNIFICATION=1000.
LINE=`grep 'Preview: Magnification' ${LOGFILE}`
if [ $? -ne 0 ]; then
	MAGNIFICATION=1000
else
	# The sed script strips out everything that won't form an /integer/.
	MAGNIFICATION=`echo "${LINE}" | sed 's/[^0-9]//g; 1q'`
fi

# 3. Compute resolution.
# "bc" allows floating-point arithmetic, unlike "expr" or "dc".
RESOLUTION=`echo "scale=2; \
		${SCALEFACTOR} * (10/${LATEXFONT}) * (1000/${MAGNIFICATION})" \
		| bc`

# DVI -> PostScript
dvips -o ${PSFILE} ${DVIFILE}
if [ $? -ne 0 ]; then
	echo "Failed: dvips -o ${PSFILE} ${DVIFILE}"
	BAIL_OUT
fi

# PostScript -> Bitmap files
# Older versions of gs have problems with a large degree of anti-aliasing
# at high resolutions
ALPHA=4
if [ ${RESOLUTION} -gt 150 ]; then
	ALPHA=2
fi

gs -q -dNOPAUSE -dBATCH -dSAFER -sDEVICE=pnmraw -sOutputFile=${BASE}%03d.ppm \
    -dGraphicsAlphaBit=${ALPHA} -dTextAlphaBits=${ALPHA} -r${RESOLUTION} \
    ${PSFILE}

if [ $? -ne 0 ]; then
	echo "Failed: gs ${PSFILE}"
	BAIL_OUT
fi

# All has been successful, so remove everything except the bitmap files
# and the metrics file.
FILES=`ls ${BASE}* | sed -e "/${BASE}.metrics/d" -e "/${BASE}[0-9]\{3\}.ppm/d"`
rm -f ${FILES}
