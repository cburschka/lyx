## file lyxstangle.R
## This file is part of LyX, the document processor.
## Licence details can be found in the file COPYING.

## author Yihui Xie

## Full author contact details are available in file CREDITS

## Rscript $$s/scripts/lyxstangle.R $$i $$e $$r

.cmdargs = commandArgs(TRUE)

options(encoding = .cmdargs[2])

## run Stangle to extract R code
Stangle(.cmdargs[1])
