#!/bin/bash
# Lyx files converted from 2.1 to 2.2 contain parbeak and latexpar separators
# that would not appear if the lyx file was written in 2.2 from scratch. The
# script removes latexpar separators and transforms parbreak separators into
# plain separators. Then it displays a diff of the resulting pdf files (requires
# diffpdf) for manual control. All lyx files shipped with lyx should be updated
# in this way. See #10068.
#
# example usage:
# cd lib/doc
# ../development/tools/separator-convert.sh Math.lyx
#
BUILD=../../build
LYX=$BUILD/src/lyx
cp $1 $1.old
$LYX -E pdf2 $1.old.pdf $1
sed -i "s/^\\\\begin_inset Separator parbreak$/\\\\begin_inset Separator plain/" $1
sed -i "/^\\\\begin_inset Separator latexpar$/ { N; d; }" $1
$LYX -e lyx $1
$LYX -E pdf2 $1.pdf $1
diffpdf $1.old.pdf $1.pdf
