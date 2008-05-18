#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file csv2lyx.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Hartmut Haase
# author José Matos

# Full author contact details are available in file CREDITS

# This script reads a csv-table (file name.csv) and converts it into
# a LyX-table for versions 1.5.0 and higher (LyX table format 276).
# It uses Python's csv module for parsing.
# The original csv2lyx was witten by Antonio Gulino <antonio.gulino@tin.it>
# in Perl for LyX 1.x and modified for LyX table format 276 by the author.
#
import csv, unicodedata
import os, sys
import optparse

def error(message):
    sys.stderr.write(message + '\n')
    sys.exit(1)

# processing command line options
# delegate this to standard module optparse
args = {}
args["usage"] = "Usage: csv2lyx [options] mycsvfile mytmptable.lyx"

args["description"] = """This script creates a LyX document containing a table
from a comma-separated-value file. The LyX file has format 276
and can be opened with LyX 1.5.0 and newer.
"""
parser = optparse.OptionParser(**args)

parser.set_defaults(excel = 'n', column_sep = 'n', guess_sep = False)
parser.add_option("-e", "--excel",
                  help="""'character'  Excel type, default is 'n'
   		       'e': Excel-generated CSV file
   		       't': Excel-generated TAB-delimited CSV file""")
parser.add_option("-s", "--separator", dest="column_sep",
                  help= "column separator, default is ','")
parser.add_option("-g", "--guess-sep", action="store_true",
                  help = "guess the columns separator")

group = optparse.OptionGroup(parser, "Remarks", """If your .csv file contains special characters (e. g. umlauts,
   accented letters, etc.) make sure it is coded in UTF-8 (unicode).
   Else LyX will loose some cell contents. If your .csv file was not written according to the "Common Format and MIME Type for Comma-Separated Values (CSV) Files" (http://tools.ietf.org/html/rfc4180) there may be unexpected results.""")
parser.add_option_group(group)

(options, args) = parser.parse_args()

# validate input
if len(args) == 1:
    infile = args[0]
    fout = sys.stdout
elif len(args) ==2:
    infile = args[0]
    fout = open(args[1], 'w')
else:
    parser.print_help()
    sys.exit(1)

if not os.path.exists(infile):
	error('File "%s" not found.' % infile)

dialects = {'n' : None, 'e' : 'excel', 't' : 'excel-tab'}
if options.excel not in dialects:
    parser.print_help()
    sys.exit(1)
dialect= dialects[options.excel]

# when no special column separator is given, try to detect it:
if options.column_sep == 'n':
    options.guess_sep = 'True'
print options.column_sep, options.guess_sep
if options.guess_sep:
    guesser = csv.Sniffer()
    input_file = "".join(open(infile,'rb').readlines())
    try:
        dialect = guesser.sniff(input_file)
        print 'found:', dialect.delimiter
        reader = csv.reader(open(infile, "rb"), dialect= dialect)
    except:
        print 'error, using ,'
        reader = csv.reader(open(infile, "rb"), dialect= dialect, delimiter=',')
else:
    reader = csv.reader(open(infile, "rb"), dialect= dialect, delimiter=options.column_sep)

# read input
num_cols = 1 # max columns
rows = []

for row in reader:
    num_cols = max(num_cols, len(row))
    rows.append(row)

num_rows = reader.line_num # number of lines

# create a LyX file
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
	num_cols_2 = len(rows[j]) # columns in current row
	#print j, ': ' , rows[j]
############################
# write contents of one line
############################
	i = 0
	while i < num_cols_2:
		fout.write("""<cell alignment="left" valignment="top" usebox="none">
\\begin_inset Text

\\begin_layout Standard\n""")
		#print rows[j][i]
		fout.write(rows[j][i])
		fout.write('\n\\end_layout\n\n\\end_inset\n</cell>\n')
		i += 1
# If row has less columns than num_cols
	if num_cols_2 < num_cols:
		while i < num_cols:
			fout.write("""<cell alignment="left" valignment="top" usebox="none">
\\begin_inset Text

\\begin_layout Standard\n""")
			fout.write(' ')
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
# close the LyX file
fout.close()
