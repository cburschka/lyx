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
    echo modern_xfig

    # Can we find fig2dev?
    type fig2dev > /dev/null || exit 1

    input=$1
    pdftex_t=$2
    pdftex=$3.pdf

    fig2dev -Lpdftex ${input} ${pdftex}
    fig2dev -Lpdftex_t -p${outbase} ${input} ${pdftex_t}

    exit 0;
}

# Older versions of xfig cannot do this, so we emulate the behaviour using
# pstex and pstex_t output.
legacy_xfig() {
    echo legacy_xfig

    # Can we find fig2dev, eps2eos or gs?
    type fig2dev > /dev/null || exit 1
    type eps2eps > /dev/null || exit 1
    type gs > /dev/null || exit 1

    input=$1
    pdftex_t=$2
    png=$3.png
    pstex=$3.pstex

    fig2dev -Lpstex ${input} ${pstex}
    fig2dev -Lpstex_t -p${outbase} ${input} ${pdftex_t}

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

# We expect two args, the names of the input and output files.
test $# -eq 2 || exit 1

input=$1
output=$2

# Strip the extension from ${output}
outbase=`echo ${output} | sed 's/[.][^.]*$//'`

# Ascertain whether fig2dev is "modern enough".
# Here "modern" means "fig2dev Version 3.2 Patchlevel 4"
version_info=`fig2dev -h | sed '/^fig2dev/! d'`
# If no line begins "fig2dev" then default to legacy_xfig
test "x${version_info}" = "x" && {
    legacy_xfig ${input} ${output} ${outbase}
}

version=`echo ${version_info} | cut -d' ' -f3`
patchlevel=`echo ${version_info} | cut -d' ' -f5`
# If we cannot extract the version of patchlevel info
# then default to legacy_xfig
test "x${version}" = "x" -o "x${patchlevel}" = "x" && {
    legacy_xfig ${input} ${output} ${outbase}
}
echo ${version} ${patchlevel} | grep '[0-9]!' -o && {
    legacy_xfig ${input} ${output} ${outbase}
}

# So, is it an old version?
test ${version} != "3.2" -o ${patchlevel} -lt 4 && {
    legacy_xfig ${input} ${output} ${outbase}
}
# I guess not ;-)

# Commented out for now to test legacy_xfig...
#modern_xfig ${input} ${output} ${outbase}
legacy_xfig ${input} ${output} ${outbase}

# The end
