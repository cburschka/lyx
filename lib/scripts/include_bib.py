#!/usr/bin/env python
# -*- coding: utf-8 -*-

# file include_bib.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# authors Richard Heck and [SchAirport]

# Full author contact details are available in file CREDITS

# This script is intended to include a BibTeX-generated biblography 
# in a LaTeX file, as publishers often want. It can be run manually
# on an exported LaTeX file, though it needs to be compiled first,
# so the bbl file will exist.
#
# It should also be possible to create a LyX converter to run this
# automatically. To set it up, create a format "ltxbbl"; make sure to 
# check it as a document format. Then create a LaTeX-->ltxbbl converter, 
# with the command:
#   python -tt $$s/scripts/include_bib.py $$i $$0
# and give it the "needaux" flag. You'll then have it in the export menu.
# We do not activate this converter by default, because there are problems
# when one tries to use multiple bibliographies.
#
# Please report any problems on the devel list.

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
  return out
    

def usage():
  print r'''
Usage: python include_bib.py file.tex [outfile.tex]
  Includes the contents of file.bbl, which must exist in the
  same directory as file.tex, in place of the \bibliography
  command, and creates the new file outfile.tex. If no name
  for that file is given, we create: file-bbl.tex.
'''  

if __name__ == "__main__":
  args = len(sys.argv)
  if args <= 1 or args > 3:
    usage()
    sys.exit(0)

  # we might should make sure this is a tex file....
  infile = sys.argv[1]
  if infile[-4:] != ".tex":
    print "Error: " + infile + " is not a TeX file"
    usage()
    sys.exit(1)

  if args == 3:
    outfile = sys.argv[2]
  else:
    outfile = infile[:-4] + "-bbl.tex"

  newfile = InsertBib(infile, outfile)
  print "Wrote " + outfile
