#! /bin/sh

# file fig2pdf.sh
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# author Angus Leeming
#
# Full author contact details are available in file CREDITS


# This script converts an XFIG image to something that pdflatex can process
# into high quality PDF.

# Usage: sh fig2pdf.sh ${base}.xfig
# to generate ${base}.pdftex_t
# Thereafter, you need only '\input{${base}.pdftex_t}' in your latex document.

# These are the external programs we use
FIG2DEV=fig2dev
# If using "legacy_xfig" only
EPSTOPDF=epstopdf

find_exe() {
    test $# -eq 1 || exit 1

    type $1 > /dev/null || {
	echo "Unable to find \"$1\". Please install."
	exit 1
    }
}

# modern_xfig() and legacy_xfig() are the functions that do all the work.

# Modern versions of xfig can output the image without "special" text as
# a PDF file ${base}.pdf and place the text in a LaTeX file
# ${base}.pdftex_t for typesetting by pdflatex itself.
modern_xfig() {
    # Can we find fig2dev?
    find_exe ${FIG2DEV}

    input=$1
    pdftex_t=$2
    pdftex=$3.pdf

    ${FIG2DEV} -Lpdftex -p1 ${input} ${pdftex}
    ${FIG2DEV} -Lpdftex_t -p${outbase} ${input} ${pdftex_t}

    exit 0;
}

# Older versions of xfig cannot do this, so we emulate the behaviour using
# pstex and pstex_t output.
legacy_xfig() {
    # Can we find fig2dev and epstopdf?
    find_exe ${FIG2DEV}
    find_exe ${EPSTOPDF}

    input=$1
    pdftex_t=$2
    pdf=$3.pdf
    pstex=$3.pstex

    ${FIG2DEV} -Lpstex ${input} ${pstex}
    ${FIG2DEV} -Lpstex_t -p${outbase} ${input} ${pdftex_t}

    # Convert the ${pstex} EPS file (free of "special" text) to PDF format
    # using epstopdf.
    ${EPSTOPDF} --outfile=${pdf} ${pstex}
    rm -f ${pstex}

    exit 0;
}

# The main logic of the script is below.
# All it does is ascertain which of the two functions above to call.

# We expect two args, the names of the input and output files.
test $# -eq 2 || exit 1

input=$1
output=$2

# Strip the extension from ${output}
outbase=`echo ${output} | sed 's/[.][^.]*$//'`

# Ascertain whether fig2dev is "modern enough".
# If it is, then the help info will mention "pdftex_t" as one of the
# available outputs.
FUNCTION=modern_xfig
${FIG2DEV} -h | grep 'pdftex_t' > /dev/null || FUNCTION=legacy_xfig

${FUNCTION} ${input} ${output} ${outbase}

# The end
