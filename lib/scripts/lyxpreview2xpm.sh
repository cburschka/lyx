#!/bin/bash

# converts a single inset's TeX representation to an .xpm

dir=`dirname $2`
base=${dir}/`basename $2 .xpm`

cp $1 ${base}.tex 
(cd ${dir} ; pdflatex --interaction batchmode ${base}.tex) 
dvips -x 2000 -R -E -o ${base}.eps ${base}.dvi 
convert ${base}.eps ${base}.xpm 
rm ${base}.tex ${base}.aux ${base}.dvi ${base}.log ${base}.eps || true

exit 0
