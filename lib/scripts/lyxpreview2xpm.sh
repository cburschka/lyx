#!/bin/bash

# converts a single inset's TeX representation to an .xpm

dir=`dirname $2`
base=${dir}/`basename $2 .xpm`

cp $1 ${base}.tex 
#(cd ${dir} ; latex ${base}) 
(cd ${dir} ; pdflatex --interaction batchmode ${base}) 
dvips -x 1750 -R -o ${base}.ps ${base}.dvi 
convert -crop 0x0 ${base}.ps ${base}.xpm 
rm ${base}.tex ${base}.aux ${base}.dvi ${base}.log ${base}.eps || true

exit 0
