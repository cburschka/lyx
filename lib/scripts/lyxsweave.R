# Wrapper around Sweave that sets up some things for LyX
# argument 1 is the absolute name of the input file
# argument 2 is the absolute name of the output file
# argument 3 is the iconv name for the encoding of the file
# argument 4 is the original document directory

ls.args <- commandArgs(trailingOnly=T)

# check whether Sweave.sty is seen by LaTeX
ls.sweavesty <- system("kpsewhich Sweave.sty", intern=T, ignore.stderr=T)
ls.sp <- (length(ls.sweavesty) == 0)

# set default encoding to the one of the file; it will be reset to previous
# default by the sweave module
ls.enc <- getOption("encoding")
options(encoding=ls.args[3])

# pass document dir to sweave module
ls.dir <- ls.args[4]

# this is used to tell where temporary files should go
ls.pr <- sub("\\.tex$", "", ls.args[2])


# finally run sweave
Sweave(file=ls.args[1], output=ls.args[2], syntax="SweaveSyntaxNoweb", stylepath=ls.sp, prefix.string=ls.pr)
