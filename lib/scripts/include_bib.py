# file include_bib.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# authors Richard Heck and [SchAirport]

# Full author contact details are available in file CREDITS

import sys, os

class secbib:
  def __init__(self, start = -1, end = -1):
    self.start = start
    self.end   = end

class BibError(Exception):
  def __init__(self, msg):
    self.msg = msg

  def __str__(self):
    return self.msg


def InsertBib(fil, out):   
  ''' Inserts the contents of the .bbl file instead of the bibliography in a new .tex file '''

  texlist = open(fil, 'r').readlines()

  # multiple bibliographies
  biblist = []
  stylist = []
  
  for i, line in enumerate(texlist):
    if "\\bibliographystyle" in line:
      stylist.append(i)
    elif "\\bibliography" in line:
      biblist.append(i)
    elif "\\begin{btSect}" in line:
      raise BibError("Cannot export sectioned bibliographies")
  
  filenew = fil[:-4] + "-bibinc.tex" #The new .tex file

  if len(biblist) > 1:
    raise BibError("Cannot export multiple bibliographies.")
  if not biblist:
    raise BibError("No biliography found!")

  bibpos = biblist[0]
  newlist = texlist[0:bibpos]
  bblfile = fil[:-4] + ".bbl"
  bbllist = open(bblfile, 'r').readlines()
  newlist += bbllist
  newlist += texlist[bibpos + 1:]
    
  outfile = open(out, 'w')
  outfile.write("".join(newlist))
  outfile.close()
  return filenew
    

if __name__ == "__main__":
  newfile = InsertBib(sys.argv[1], sys.argv[2])
  print "Wrote " + newfile
