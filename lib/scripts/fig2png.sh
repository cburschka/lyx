#! /bin/sh
# converts an image from XFIG to PNG format
# We go the long route to ensure that the image is of the highest
# possible quality.

# We expect a single arg, the name of the input file.
test $# -eq 1 || exit 1

input=`basename $1`
base=`basename ${input} .fig`
test ${input} = ${base} && {
    echo Expecting an XFIG file as input
    exit 1
}

dir=`dirname $1`
base=${dir}/${base}

# Generate the fig2dev output
eps=${base}.eps
pstex_t=${base}.pstex_t

echo Entered FIG2PNG.SH

fig2dev -Lpstex ${input} ${eps}
fig2dev -Lpstex_t -p${base} ${input} ${pstex_t}

# Convert the EPS file (free of "special" text) to PNG format using gs
# gs is extremely fussy about the EPS files it converts, so ensure it is
# "clean" first.
clean_eps=${eps}.$$
eps2eps ${eps} ${clean_eps}

# Extract the width and height of the image using gs' bbox device.
# Ie, take output that includes a line "%%BoundingBox: 0 0 <width> <height>"
# and rewrite it as "-g<width>x<height>"
geometry=`gs -q -dSAFER -dNOPAUSE -dBATCH -sDEVICE=bbox ${clean_eps} 2>&1 | \
	sed '/^%%BoundingBox/! d' | cut -d' ' -f4,5 | \
	sed 's/^\([0-9]\{1,\}\) \([0-9]\{1,\}\)$/-g\1x\2/'`

# Generate the bitmap using the -g option to ensure the size is the same
# as the original.
# If we're using a version of gs that does not have a bbox device, then
# $GEOMETRY = "", so there are no unwanted side effects.
png=${base}.png
gs -q -dSAFER -dBATCH -dNOPAUSE ${geometry} -sDEVICE=png16m \
	-sOutputFile=${png} ${clean_eps}
rm -f ${clean_eps} ${eps}
