#! /bin/sh

# file lyxpreview2bitmap.sh
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# author Angus Leeming
# with much advice from David Kastrup, david.kastrup@t-online.de.
#
# Full author contact details are available in file CREDITS

# This script takes a LaTeX file and generates bitmap image files,
# one per page.

# The idea is to use it with preview.sty from the preview-latex project
# (http://preview-latex.sourceforge.net/) to create small bitmap
# previews of things like math equations.

# preview.sty can be obtained from
# CTAN/macros/latex/contrib/supported/preview.

# This script takes three arguments:
# TEXFILE:       the name of the .tex file to be converted.
# SCALEFACTOR:   a scale factor, used to ascertain the resolution of the
#                generated image which is then passed to gs.
# OUTPUTFORMAT:  the format of the output bitmap image files.
#                Two formats are recognised: "ppm" and "png".

# If successful, this script will leave in dir ${DIR}:
# a (possibly large) number of image files with names like
#         ${BASE}\([0-9]*\).${SUFFIX} where SUFFIX is ppm or png.
# a file containing info needed by LyX to position the images correctly
# on the screen.
#         ${BASE}.metrics
# All other files ${BASE}* will be deleted.

# A quick note on the choice of OUTPUTFORMAT:

# In general files in PPM format are 10-100 times larger than the
# equivalent files in PNG format. Larger files results in longer
# reading and writing times as well as greater disk usage.

# However, whilst the Qt image loader can load files in PNG format
# without difficulty, the xforms image loader cannot. They must first
# be converted to a loadable format (eg PPM!). Thus, previews will take
# longer to appear if the xforms loader is used to load snippets in
# PNG format.

# You can always experiment by adding a line to your
# ${LYXUSERDIR}/preferences file
#        \converter lyxpreview ${FORMAT} "lyxpreview2bitmap.sh" ""
# where ${FORMAT} is either ppm or png.

# These programs are used by the script.
# Adjust their names to suit your setup.
test -n "$LATEX" || LATEX=latex
DVIPNG=/home/angus/preview-latex/devel/dvipng/dvipng
DVIPS=dvips
GS=gs
PNMCROP=pnmcrop
readonly LATEX DVIPS GS PNMCROP

# Some helper functions.
FIND_IT ()
{
	test $# -eq 1 || exit 1

	type $1 > /dev/null || {
		echo "Unable to find \"$1\". Please install."
		exit 1
	}
}

BAIL_OUT ()
{
	test $# -eq 1 && echo $1

	# Remove everything except the original .tex file.
	FILES=`ls ${BASE}* | sed -e "/${BASE}\.tex/d"`
	rm -f ${FILES} texput.log
	echo "Leaving ${BASE}.tex in ${DIR}"
	exit 1
}

REQUIRED_VERSION ()
{
	test $# -eq 1 || exit 1

	echo "We require preview.sty version 0.73 or newer. You're using"
	grep 'Package: preview' $1
}

CHECK_FOR_PROGRAMS () {
	test $# -eq 1 || exit 1
	if [ $1 = "ppm" ]; then
		# We use latex, dvips and gs, so check that they're all there.
		FIND_IT ${LATEX}
		FIND_IT ${DVIPS}
		FIND_IT ${GS}
	elif [ "$3" = "png" ]; then
		# We use latex and dvipng
		FIND_IT ${LATEX}
		FIND_IT ${DVIPNG}
	fi
}

GENERATE_PPM_FILES () {
	test $# -eq 2 || exit 1

	BASE=$1
	RESOLUTION=$2

	# DVI -> PostScript
	PSFILE=${1}.ps

	${DVIPS} -o ${PSFILE} ${DVIFILE} || {
		BAIL_OUT "Failed: ${DVIPS} -o ${PSFILE} ${DVIFILE}"
	}

	# PostScript -> Bitmap files
	# Older versions of gs have problems with a large degree of
	# anti-aliasing at high resolutions

	# test expects integer arguments.
	# ${RESOLUTION} may be a float. Truncate it.
	INT_RESOLUTION=`echo "${RESOLUTION} / 1" | bc`

	ALPHA=4
	if [ ${INT_RESOLUTION} -gt 150 ]; then
		ALPHA=2
	fi

	GSDEVICE=pnmraw
	GSSUFFIX=ppm

	${GS} -q -dNOPAUSE -dBATCH -dSAFER \
		-sDEVICE=${GSDEVICE} -sOutputFile=${BASE}%d.${GSSUFFIX} \
		-dGraphicsAlphaBit=${ALPHA} -dTextAlphaBits=${ALPHA} \
		-r${RESOLUTION} ${PSFILE} ||
	{
		BAIL_OUT "Failed: ${GS} ${PSFILE}"
	}

	# All has been successful, so remove everything except the bitmap files
	# and the metrics file.
	FILES=`ls ${BASE}* | sed -e "/${BASE}.metrics/d" \
			 -e "/${BASE}\([0-9]*\).${GSSUFFIX}/d"`
#	rm -f ${FILES} texput.log

	# The bitmap files can have large amounts of whitespace to the left and
	# right. This can be cropped if so desired.
	CROP=1
	type ${PNMCROP} > /dev/null || CROP=0

	if [ ${CROP} -eq 1 ]; then
		for FILE in ${BASE}*.${GSSUFFIX}
		do
			if ${PNMCROP} -left ${FILE} 2> /dev/null |\
			   ${PNMCROP} -right  2> /dev/null > ${BASE}.tmp; then
				mv ${BASE}.tmp ${FILE}
			else
				rm -f ${BASE}.tmp
			fi
		done
		rm -f ${BASE}.tmp
	fi
}

GENERATE_PNG_FILES () {
	test $# -eq 2 || exit 1

	BASE=$1
	RESOLUTION=$2

	${DVIPNG} -D${RESOLUTION} -o${BASE}%d.png ${DVIFILE}
}

echo $*

# Preliminary check.
if [ $# -ne 3 ]; then
	exit 1
fi

# Extract the params from the argument list.
DIR=`dirname $1`
BASE=`basename $1 .tex`

SCALEFACTOR=$2
OUTPUTFORMAT=$3

if [ "${OUTPUTFORMAT}" != "ppm" -a "${OUTPUTFORMAT}" != "png" ]; then
	BAIL_OUT "Unrecognised output format ${OUTPUTFORMAT}. \
	Expected either \"ppm\" or \"png\"."
fi

CHECK_FOR_PROGRAMS ${OUTPUTFORMAT}

# Initialise some variables.
TEXFILE=${BASE}.tex
LOGFILE=${BASE}.log
DVIFILE=${BASE}.dvi
METRICSFILE=${BASE}.metrics
readonly TEXFILE LOGFILE DVIFILE METRICSFILE

# LaTeX -> DVI.
cd ${DIR}
${LATEX} ${TEXFILE} ||
{
	BAIL_OUT "Failed: ${LATEX} ${TEXFILE}"
}

# Parse ${LOGFILE} to obtain bounding box info to output to
# ${METRICSFILE}.
# This extracts lines starting "Preview: Tightpage" and
# "Preview: Snippet".
grep -E 'Preview: [ST]' ${LOGFILE} > ${METRICSFILE} ||
{
	REQUIRED_VERSION ${LOGFILE}
	BAIL_OUT "Failed: grep -E 'Preview: [ST]' ${LOGFILE}"
}

# Parse ${LOGFILE} to obtain ${RESOLUTION} for the gs process to follow.
# 1. Extract font size from a line like "Preview: Fontsize 20.74pt"
# Use grep for speed and because it gives an error if the line is
# not found.
LINE=`grep 'Preview: Fontsize' ${LOGFILE}` ||
{
	REQUIRED_VERSION ${LOGFILE}
	BAIL_OUT "Failed: grep 'Preview: Fontsize' ${LOGFILE}"
}
# The sed script strips out everything that won't form a decimal number
# from the line. It bails out after the first match has been made in
# case there are multiple lines "Preview: Fontsize". (There shouldn't
# be.)
# Note: use "" quotes in the echo to preserve newlines.
LATEXFONT=`echo "${LINE}" | sed 's/[^0-9\.]//g; 1q'`

# 2. Extract magnification from a line like
# "Preview: Magnification 2074"
# If no such line found, default to MAGNIFICATION=1000.
LINE=`grep 'Preview: Magnification' ${LOGFILE}`
if LINE=`grep 'Preview: Magnification' ${LOGFILE}`; then
	# Strip out everything that won't form an /integer/.
	MAGNIFICATION=`echo "${LINE}" | sed 's/[^0-9]//g; 1q'`
else
	MAGNIFICATION=1000
fi

# 3. Compute resolution.
# "bc" allows floating-point arithmetic, unlike "expr" or "dc".
RESOLUTION=`echo "scale=2; \
	${SCALEFACTOR} * (10/${LATEXFONT}) * (1000/${MAGNIFICATION})" \
	| bc`

# 4. Generate the bitmap files and clean-up.
if [ "${OUTPUTFORMAT}" = "ppm" ]; then
	GENERATE_PPM_FILES ${BASE} ${RESOLUTION}
else
	GENERATE_PNG_FILES ${BASE} ${RESOLUTION}
fi

echo "Previews generated!"
