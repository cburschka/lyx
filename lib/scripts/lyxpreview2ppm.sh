#!/bin/sh

# This script takes a LaTeX file and generates PPM files, one per page.
# The idea is to use it with preview.sty to create small bitmap previews of
# things like math equations.

# The script takes two arguments, the name of the file to be converted and
# the resolution of the generated image, to be passed to gs.
if [ $# -ne 2 ]; then
	exit 1
fi

# A couple of helper functions
FIND_EXECUTABLE=""
FIND_IT () {
	which ${FIND_EXECUTABLE} > /dev/null
	STATUS=$?
	if [ ${STATUS} -ne 0 ]; then
		echo "Unable to find \"${FIND_EXECUTABLE}\". Please install."
		exit 1
	fi
}

CHECK_STATUS () {
	if [ ${STATUS} -ne 0 ]; then
		echo "${EXECUTABLE} failed."
		# Remove everything except the original .tex file.
		FILES=`ls ${BASE}* | sed -e "/${BASE}.tex/d"`
		rm -f ${FILES}
		exit ${STATUS}
	fi
}

# We use latex, dvips and gs, so check that they're all there.
FIND_EXECUTABLE=latex; FIND_IT
FIND_EXECUTABLE=dvips; FIND_IT
FIND_EXECUTABLE=gs;    FIND_IT

# Initialise some variables.
TEXFILE=$1
RESOLUTION=$2

DIR=`dirname ${TEXFILE}`
BASE=`basename ${TEXFILE} .tex`
DVIFILE=${BASE}.dvi
PSFILE=${BASE}.ps
METRICS=${BASE}.metrics

# Perform the conversion.
cd ${DIR}
latex -interaction=batchmode ${TEXFILE}

STATUS=$?
EXECUTABLE="latex ${TEXFILE}"; CHECK_STATUS

dvips -o ${PSFILE} ${DVIFILE}

STATUS=$?
EXECUTABLE="dvips ${DVIFILE}"; CHECK_STATUS

# Older versions of gs have problems with a large degree of anti-aliasing
# at high resolutions
ALPHA=4
if [ ${RESOLUTION} -gt 150 ]; then
	ALPHA=2
fi

gs -q -dNOPAUSE -dBATCH -dSAFER -sDEVICE=pnm -sOutputFile=${BASE}%03d.ppm \
    -dGraphicsAlphaBit=${ALPHA} -dTextAlphaBits=${ALPHA} -r${RESOLUTION} \
    ${PSFILE}

STATUS=$?
EXECUTABLE="gs ${PSFILE}"; CHECK_STATUS

# Attempt to generate a file ${METRICS} that contains only the tightpage
# bounding box info, extract from ${PSFILE}

# 1. Create a file containing the sed instructions
SEDSCRIPT=bbox.sed
cat - > ${SEDSCRIPT} <<EOF
# Delete everything that's enclosed between %%BeginDocument and %%EndDocument
/^\%\%BeginDocument/,/^\%\%EndDocument/d

# Extract the tightpage bounding box info.
# Given this snippet:
# %%Page: 1 1
# 1 0 bop
# -32890 -32890 32890 32890 492688 0 744653
# The sed command gives this:
# %%Page 1: -32890 -32890 32890 32890 492688 0 744653

/^\%\%Page:/{
  s/\: \(.*\) .*$/ \1: /;N;N
  s/\n[^\n]*\n//p
}

# Delete everything (so only the stuff that's printed, above, goes into the
# metrics file).
d
EOF

# 2. Run sed!
sed -f ${SEDSCRIPT} < ${PSFILE} > ${METRICS}
STATUS=$?
rm -f ${SEDSCRIPT}
EXECUTABLE="extracting metrics"; CHECK_STATUS

# All was successful, so remove everything except the ppm files and the
# metrics file.
FILES=`ls ${BASE}* | sed -e "/${BASE}.metrics/d" -e "/${BASE}.*.ppm/d"`
rm -f ${FILES}
