## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; either version 2 of the
## License, or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
## General Public License for more details.

## author Yihui Xie

## knitr is an alternative package to Sweave, and has more features
## and flexibility; see https://yihui.github.com/knitr

## Rscript $$s/scripts/lyxknitr.R $$p$$i $$p$$o $$e $$r
## $$p the path of the output (temp dir)
## $$i the file name of the input Rnw
## $$o the tex output
## $$r path to the original input file (the lyx document)
## $$e encoding (e.g. 'UTF-8')

library(knitr)

.cmdargs = commandArgs(TRUE)

.orig.enc = getOption("encoding")
options(encoding = .cmdargs[3])

## the working directory is the same with the original .lyx file; you
## can put your data files there and functions like read.table() can
## work correctly without specifying the full path
setwd(.cmdargs[4])

## copy the Rnw file to the current working directory if it does not exist
.tmp.file = tempfile(); .rnw.file = basename(.cmdargs[1])
.rnw.exists = file.exists(.rnw.file)
if (.rnw.exists) file.rename(.rnw.file, .tmp.file)
file.copy(.cmdargs[1], '.')
## run knit() to get .tex or .R
knit(.rnw.file, tangle = 'tangle' %in% .cmdargs)

setwd(.cmdargs[4])
## remove the copied .Rnw if it did not exist, otherwise move the original one back
if (.rnw.exists) file.rename(.tmp.file, .rnw.file) else unlink(.rnw.file)
file.rename(basename(.cmdargs[2]), .cmdargs[2])  # move .tex to the temp dir
rm(.tmp.file, .rnw.file, .rnw.exists)  # clean up these variables

