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

# modern_xfig() and legacy_xfig() are the functions that do all the work.

# Modern versions of xfig can output the image without "special" text as
# a PDF file ${base}.pdf and place the text in a LaTeX file
# ${base}.pdftex_t for typesetting by pdflatex itself.
modern_xfig() {
    input=$1.fig
    pdftex=$1.pdf
    pdftex_t=$1.pdftex_t

    fig2dev -Lpdftex ${input} ${pdftex}
    fig2dev -Lpdftex_t -p$1 ${input} ${pdftex_t}

    exit 0;
}

# Older versions of xfig cannot do this, so we emulate the behaviour using
# pstex and pstex_t output.
legacy_xfig() {
    input=$1.fig
    pstex=$1.pstex
    png=$1.png
    pdftex_t=$1.pdftex_t

    fig2dev -Lpstex ${input} ${pstex}
    fig2dev -Lpstex_t -p$1 ${input} ${pdftex_t}

    # Convert the ${pstex} EPS file (free of "special" text) to PDF format
    # using gs.

    # gs is extremely fussy about the EPS files it converts, so ensure it is
    # "clean" first.
    clean=${pstex}.$$
    eps2eps ${pstex} ${clean}
    rm -f ${pstex}

    # Extract the width and height of the image using gs' bbox device.
    # Ie, take output that includes line "%%BoundingBox: 0 0 <width> <height>"
    # and rewrite it as "-g<width>x<height>"
    geometry=`gs -q -dSAFER -dNOPAUSE -dBATCH -sDEVICE=bbox ${clean} 2>&1 | \
	sed '/^%%BoundingBox/! d' | cut -d' ' -f4,5 | \
	sed 's/^\([0-9]\{1,\}\) \([0-9]\{1,\}\)$/-g\1x\2/'`

    # Generate a PNG file using the -g option to ensure the size is the same
    # as the original.
    # If we're using a version of gs that does not have a bbox device, then
    # ${geometry} = "", so there are no unwanted side effects.
    gs -q -dSAFER -dBATCH -dNOPAUSE ${geometry} -sDEVICE=png16m \
	-sOutputFile=${png} ${clean}
    rm -f ${clean}

    exit 0;
}

# The main logic of the script is below.
# All it does is ascertain which of the two functions above to call.

# We expect a single arg, the name of the input file.
test $# -eq 1 || exit 1

# Remove the .fig extension
input=`basename $1`
base=`echo ${input} | sed 's/\.fig$//'`

# Ascertain whether fig2dev is "modern enough".
# Here "modern" means "fig2dev Version 3.2 Patchlevel 4"
version_info=`fig2dev -h | sed '/^fig2dev/! d'`
# If no line begins "fig2dev" then default to legacy_xfig
test "x${version_info}" = "x" && legacy_xfig ${base}

version=`echo ${version_info} | cut -d' ' -f3`
patchlevel=`echo ${version_info} | cut -d' ' -f5`
# If we cannot extract the version of patchlevel info
# then default to legacy_xfig
test "x${version}" = "x" -o "x${patchlevel}" = "x" && legacy_xfig ${base}
echo ${version} ${patchlevel} | grep '[0-9]!' -o && legacy_xfig ${base}

# So, is it am old version?
test ${version} != "3.2" -o ${patchlevel} -lt 4 && legacy_xfig ${base}
# I guess not ;-)
modern_xfig ${base}

# The end
