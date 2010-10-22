# Wrapper around Sweave that sets up some things for LyX
# argument 1 is the file name
# argument 2 is the iconv name for the encoding of the file

ls.args <- commandArgs(trailingOnly=T)

# check whether Sweave.sty is seen by LaTeX
ls.sweavesty <- system("kpsewhich Sweave.sty", intern=T, ignore.stderr=T)
ls.sp <- (length(ls.sweavesty) == 0)

# set default encoding to the one of the file
options(encoding=ls.args[2])

#run sweave
Sweave(ls.args[1], stylepath=ls.sp)
