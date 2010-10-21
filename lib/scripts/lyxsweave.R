# Wrapper around Sweave that sets up some things for LyX
# argument 1 is the file name

args <- commandArgs(trailingOnly=T)

# check whether Sweave.sty is seen by LaTeX
sweavesty <- system("kpsewhich Sweave.sty", intern=T, ignore.stderr=T)
sp <- (length(sweavesty) == 0)

Sweave(args[1], stylepath=sp)
