# file lyxsweave.R
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Jean-Marc Lasgouttes

# Full author contact details are available in file CREDITS

# Wrapper around Sweave that sets up some things for LyX
# argument 1 is the absolute name of the input file
# argument 2 is the absolute name of the output file
# argument 3 is the iconv name for the encoding of the file
# argument 4 is the original document directory

ls.args <- commandArgs(trailingOnly=TRUE)

# check whether Sweave.sty is seen by LaTeX. if it is not, we will
# pass the option stylepath=TRUE to sweave so that a full path is given
# to \usepackage.
ls.sweavesty <- system("kpsewhich Sweave.sty", intern=TRUE, ignore.stderr=TRUE)
ls.sp <- (length(ls.sweavesty) == 0)

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
Sweave(file=ls.args[1], output=ls.args[2], syntax="SweaveSyntaxNoweb", stylepath=ls.sp, prefix.string=ls.pr)
