#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file csv2lyx.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Hartmut Haase

# Full author contact details are available in file CREDITS

# This script reads a csv-table (file name.csv) and converts it into
# a LyX-table for versions 1.5.0 and higher (LyX table format 276).
# The original csv2lyx was witten by Antonio Gulino <antonio.gulino@tin.it>
# in Perl for LyX 1.x and modified for LyX table format 276 by the author.
#


import os, re, string, sys, unicodedata

def error(message):
    sys.stderr.write(message + '\n')
    sys.exit(1)

# processing command line options
if len(sys.argv) == 1 or sys.argv[1] == '--help':
    print '''Usage:
   csv2lyx [options] mycsvfile mytmptable.lyx

This script creates a LyX document containing a table
from a comma-separated-value file. The LyX file has format 276
and can be opened with LyX 1.5.0 and newer.

Options:
   -s separator    column separator, default is Tab
   --help          usage instructions

Remarks:
   If your .csv file contains special characters (e. g. umlauts,
   accented letters, etc.) make sure it is coded in UTF-8 (unicode).
   Else LyX will loose some cell contents.'''
    sys.exit(0)

# print len(sys.argv), sys.argv
separator = '\t'
infile = ""
if len(sys.argv) == 3:
	infile = sys.argv[1]
	outfile = sys.argv[2]
elif len(sys.argv) == 5:
	infile = sys.argv[3]
	outfile = sys.argv[4]
	if sys.argv[1] == '-s':
		separator = sys.argv[2]

if not os.path.exists(infile):
	error('File "%s" not found.' % infile)
# read input
finput = open(infile, 'r')
rowcontent = finput.readlines()
finput.close()
num_rows = len(rowcontent) # number of lines
# print 'num_rows ', num_rows
i = 0
num_cols = 1 # max columns
while i < num_rows:
	# print len(rowcontent[i]), '   ', rowcontent[i]
	num_cols = max(num_cols, rowcontent[i].count(separator) + 1)
	i += 1
# print num_cols

fout = open(outfile, 'w')
#####################
# write first part
####################
fout.write("""#csv2lyx created this file
\lyxformat 276
\\begin_document
\\begin_header
\\textclass article
\\inputencoding auto
\\font_roman default
\\font_sans default
\\font_typewriter default
\\font_default_family default
\\font_sc false
\\font_osf false
\\font_sf_scale 100
\\font_tt_scale 100
\\graphics default
\\paperfontsize default
\\papersize default
\\use_geometry false
\\use_amsmath 1
\\use_esint 0
\\cite_engine basic
\\use_bibtopic false
\\paperorientation portrait
\\secnumdepth 3
\\tocdepth 3
\\paragraph_separation indent
\\defskip medskip
\\papercolumns 1
\\papersides 1
\\paperpagestyle default
\\tracking_changes false
\\output_changes false
\\end_header

\\begin_body

\\begin_layout Standard
\\align left
\\begin_inset Tabular
""")
fout.write('<lyxtabular version="3" rows=\"' + str(num_rows) + '\" columns=\"' + str(num_cols) + '\">\n')
fout.write('<features>\n')
#####################
# write table
####################
i = 0
while i < num_cols:
	fout.write('<column alignment="left" valignment="top" width="0pt">\n')
	i += 1
j = 0
while j < num_rows:
	fout.write('<row>\n')
	row = str(rowcontent[j])
	row = string.split(row,separator)
	#print j, ': ' , row
############################
# write contents of one line
############################
	i = 0
	while i < num_cols:
		fout.write("""<cell alignment="left" valignment="top" usebox="none">
\\begin_inset Text

\\begin_layout Standard\n""")
		fout.write(row[i].strip('\n'))
		fout.write('\n\\end_layout\n\n\\end_inset\n</cell>\n')
		i += 1
	fout.write('</row>\n')
	j += 1
#####################
# write last part
####################
fout.write("""</lyxtabular>

\\end_inset


\\end_layout

\\end_body
\\end_document\n""")
fout.close()
