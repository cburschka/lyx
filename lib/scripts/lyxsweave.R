# file lyxsweave.R
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Jean-Marc Lasgouttes
# author Yihui Xie

# Full author contact details are available in file CREDITS

# Wrapper around Sweave that sets up some things for LyX
# argument 1 is the absolute name of the input file
# argument 2 is the absolute name of the output file
# argument 3 is the iconv name for the encoding of the file
# argument 4 is the original document directory

ls.args <- commandArgs(trailingOnly=TRUE)

# check whether Sweave.sty is seen by LaTeX. if it is not, we will
# copy it alongside the .tex file (in general in the temporary
# directory). This means that an exported LaTeX file will not work,
# but this is a problem of installation of R on the user's machine.
# The advantage compared to the use of stylepath, is that the exported
# .tex file will be portable to another machine. (JMarc)
ls.sweavesty <- system("kpsewhich Sweave.sty", intern=TRUE, ignore.stderr=TRUE)
if (!length(ls.sweavesty)) {
   stypath <- file.path(R.home("share"), "texmf", "tex", "latex", "Sweave.sty")
   file.copy(stypath, dirname(ls.args[2]), overwrite=TRUE)
}

# set default encoding for input and output files; ls.enc is used in
# the sweave module preamble to reset the encoding to what it was.
ls.enc <- getOption("encoding")
options(encoding=ls.args[3])

# Change current directory to the document directory, so that R can find 
# data files.
setwd(ls.args[4])

# this is passed as a prefix.string to tell where temporary files should go
# the output file without extension and without '.'
tmpout <- gsub(".", "-", sub("\\.tex$", "", basename(ls.args[2])), fixed = TRUE)
# replace 
ls.pr <- paste(dirname(ls.args[2]), tmpout, sep="/")

# finally run sweave
Sweave(file=ls.args[1], output=ls.args[2], syntax="SweaveSyntaxNoweb", prefix.string=ls.pr)

# remove absolute path from \includegraphics
ls.doc = readLines(ls.args[2])
ls.cmd = paste('\\includegraphics{', dirname(ls.args[2]), "/", sep = "")
ls.idx = grep(ls.cmd, ls.doc, fixed = TRUE)
if (length(ls.idx)) {
   ls.doc[ls.idx] = sub(ls.cmd, "\\includegraphics{", ls.doc[ls.idx], fixed = TRUE)
   writeLines(ls.doc, ls.args[2])
}
