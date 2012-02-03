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

.cmdargs <- commandArgs(trailingOnly=TRUE)
.doc.enc <- .cmdargs[3]

# check whether Sweave.sty is seen by LaTeX. if it is not, we will
# copy it alongside the .tex file (in general in the temporary
# directory). This means that an exported LaTeX file will not work,
# but this is a problem of installation of R on the user's machine.
# The advantage compared to the use of stylepath, is that the exported
# .tex file will be portable to another machine. (JMarc)
if (!length(system("kpsewhich Sweave.sty", intern=TRUE, ignore.stderr=TRUE))) {
  .texmf.path <- file.path(R.home("share"), "texmf")
  if (!file.exists(.sweave.sty <- file.path(.texmf.path, "Sweave.sty"))) {
    .sweave.sty <- file.path(.texmf.path, "tex", "latex", "Sweave.sty")
  } 
  file.copy(.sweave.sty, dirname(.cmdargs[2]), overwrite=TRUE)
  rm(list = c('.sweave.sty', '.texmf.path'))
}


# Change current directory to the document directory, so that R can find 
# data files.
setwd(.cmdargs[4])

# this is passed as a prefix.string to tell where temporary files should go
# the output file without extension and without '.'
tmpout <- gsub(".", "-", sub("\\.tex$", "", basename(.cmdargs[2])), fixed = TRUE)
# replace
.prefix.str <- paste(dirname(.cmdargs[2]), tmpout, sep="/")
rm(tmpout)

# avoid the default Rplots.pdf
options(device = function(...) {
  pdf(file = tempfile())
})

# finally run sweave

# The Sweave version provided with R >= 0.13.1 has proper handling for
# encodings and our workaround for previous versions does not work
# anymore. Therefore, the invocation has to be different.
if (is.null(formals(Sweave)$encoding)) {
  # set default encoding for input and output files; .orig.enc is used in
  # the sweave module preamble to reset the encoding to what it was.
  .orig.enc <- getOption("encoding")
  options(encoding=.doc.enc)
  Sweave(file=.cmdargs[1], output=.cmdargs[2], syntax="SweaveSyntaxNoweb", 
         prefix.string=.prefix.str)
} else {
  Sweave(file=.cmdargs[1], output=.cmdargs[2], syntax="SweaveSyntaxNoweb", 
         prefix.string=.prefix.str, encoding=.doc.enc)
}

# remove absolute path from \includegraphics
options(encoding=.doc.enc) # encoding may have been changed in the preamble chunk
ls.doc <- readLines(.cmdargs[2])
ls.cmd <- paste('\\includegraphics{', dirname(.cmdargs[2]), "/", sep = "")
ls.idx <- grep(ls.cmd, ls.doc, fixed = TRUE)
if (length(ls.idx)) {
   ls.doc[ls.idx] <- sub(ls.cmd, "\\includegraphics{", ls.doc[ls.idx], fixed = TRUE)
   writeLines(ls.doc, .cmdargs[2])
}
