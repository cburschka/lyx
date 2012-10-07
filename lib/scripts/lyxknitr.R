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
## and flexibility; see https://yihui.name/knitr

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
opts_knit$set(root.dir = getwd())

## run knit() to get .tex or .R
knit(.cmdargs[1], output = .cmdargs[2], tangle = 'tangle' %in% .cmdargs)
