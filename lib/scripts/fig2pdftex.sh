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

# The external programs
FIG2DEV=fig2dev
# Used only by legacy_xfig
GS=gs

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


# This function is used only by legacy_xfig.
# It manipulates the Bounding Box info to enable gs to produce
# the appropriate PDF file from an EPS one.
# The generated PostScript commands are extracted from epstopdf, distributed
# with tetex.
clean_epsfile() {
    test $# -eq 1 || exit 1

    # No bounding box info
    grep '%%BoundingBox' $1 > /dev/null || return 1;

    bbox=`sed -n '/^%%BoundingBox/p' $1`
    llx=`echo ${bbox} | cut -d' ' -f2`
    lly=`echo ${bbox} | cut -d' ' -f3`
    urx=`echo ${bbox} | cut -d' ' -f4`
    ury=`echo ${bbox} | cut -d' ' -f5`

    width=`expr $urx - $llx`
    height=`expr $ury - $lly`
    xoffset=`expr 0 - $llx`
    yoffset=`expr 0 - $lly`

    temp=$1.??
    sed "/^%%BoundingBox/{
s/^\(%%BoundingBox:\).*/\1 0 0 ${width} ${height}\\
<< \/PageSize  [${width} ${height}] >> setpagedevice\\
gsave ${xoffset} ${yoffset} translate/
}" $1 > $temp

    mv -f $temp $1
}


# Older versions of xfig cannot do this, so we emulate the behaviour using
# pstex and pstex_t output.
legacy_xfig() {
    # Can we find fig2dev and epstopdf?
    find_exe ${FIG2DEV}
    find_exe ${GS}

    input=$1
    pdftex_t=$2
    pdf=$3.pdf
    pstex=$3.pstex

    ${FIG2DEV} -Lpstex ${input} ${pstex}
    ${FIG2DEV} -Lpstex_t -p${outbase} ${input} ${pdftex_t}

    # Convert the ${pstex} EPS file (free of "special" text) to PDF format
    # using gs
    clean_epsfile ${pstex}
    ${GS} -q -dNOPAUSE -dBATCH -dSAFER \
	-sDEVICE=pdfwrite -sOutputFile=${pdf} ${pstex}
    rm -f ${pstex}

    exit 0;
}

# The main logic of the script is below.
# All it does is ascertain which of the two functions above to call.

# We expect two args, the names of the input and output files.
test $# -eq 2 || exit 1

input=$1
output=$2

# Fail silently if the file doesn't exist
test -r $input || exit 0

# Strip the extension from ${output}
outbase=`echo ${output} | sed 's/[.][^.]*$//'`

# Ascertain whether fig2dev is "modern enough".
# If it is, then the help info will mention "pdftex_t" as one of the
# available outputs.
CONVERT_IT=modern_xfig
${FIG2DEV} -h | grep 'pdftex_t' > /dev/null || CONVERT_IT=legacy_xfig

${CONVERT_IT} ${input} ${output} ${outbase}

# The end
