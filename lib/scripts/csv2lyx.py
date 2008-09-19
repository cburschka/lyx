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

header = """#csv2lyx created this file
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
<lyxtabular version="3" rows="%d" columns="%d">
<features>
"""

cell = """<cell alignment="left" valignment="top" usebox="none">
\\begin_inset Text

\\begin_layout Standard
%s
\\end_layout

\\end_inset
</cell>"""

footer = """</lyxtabular>

\\end_inset


\\end_layout

\\end_body
\\end_document
"""

# processing command line options
# delegate this to standard module optparse
args = {}
args["usage"] = "Usage: csv2lyx [options] csvfile [file.lyx]"

args["description"] = """This script creates a LyX document containing a table created from a
comma-separated-value (CSV) file. The resulting LyX file can be opened
with LyX 1.5.0 or any later version.
If no options are given csv2lyx will try to infer the CSV type of the csvfile,
"""
parser = optparse.OptionParser(**args)

parser.set_defaults(excel ='', column_sep = '')
parser.add_option("-e", "--excel", metavar ="CHAR",
                  help = """CHAR corresponds to a CSV type:
   		       'e': Excel-generated CSV file
   		       't': Excel-generated TAB-delimited CSV file""")
parser.add_option("-s", "--separator", dest = "column_sep",
                  help = """column separator
		   		       't' means Tab""")

group = optparse.OptionGroup(parser, "Remarks", """If your CSV file contains special characters (e. g. umlauts,
   accented letters, etc.) make sure it is coded in UTF-8 (unicode).
   Else LyX will loose some cell contents. If your CSV file was not written according to the "Common Format and MIME Type for Comma-Separated Values (CSV) Files" (http://tools.ietf.org/html/rfc4180) there may be unexpected results.""")
parser.add_option_group(group)

(options, args) = parser.parse_args()

# validate input
if len(args) == 1:
    infile = args[0]
    fout = sys.stdout
elif len(args) == 2:
    infile = args[0]
    fout = open(args[1], 'w')
else:
    parser.print_help()
    sys.exit(1)

if not os.path.exists(infile):
	error('File "%s" not found.' % infile)

dialects = {'' : None, 'e' : 'excel', 't' : 'excel-tab'}
if options.excel not in dialects:
    parser.print_help()
    sys.exit(1)
dialect = dialects[options.excel]

# Set Tab, if necessary
if options.column_sep == 't':
	options.column_sep = "\t"

# when no special column separator is given, try to detect it:
if options.column_sep and dialect :
    reader = csv.reader(open(infile, "rb"), dialect = dialect, delimiter = options.column_sep)
else:
    guesser = csv.Sniffer()
    input_file = "".join(open(infile,'rb').readlines())
    try:
        dialect = guesser.sniff(input_file)
        reader = csv.reader(open(infile, "rb"), dialect = dialect)
    except:
        # older versions (python < 2.5) of csv have problems (bugs)
        # that is why we try harder to get a result, this should work on most cases
        # as it assumes that the separator is a comma (the c in csv :-) )
        try:
            reader = csv.reader(open(infile, "rb"), dialect = dialect, delimiter = ',')
        except:
            reader = csv.reader(open(infile, "rb"), delimiter = ',')

# read input
num_cols = 1 # max columns
rows = []

for row in reader:
    num_cols = max(num_cols, len(row))
    rows.append(row)

num_rows = len(rows) # number of lines

# create a LyX file
#####################
# write first part
####################
fout.write(header % (num_rows, num_cols))

#####################
# write table
####################
for i in range(num_cols):
	fout.write('<column alignment="left" valignment="top" width="0pt">\n')

for j in range(num_rows):
    row = ['<row>']

    ############################
    # write contents of one line
    ############################
    for i in range(len(rows[j])):
        row.append( cell % rows[j][i].replace('\\','\\backslash\n'))

    # If row has less columns than num_cols fill with blank entries
    for i in range(len(rows[j]), num_cols):
        row.append(cell % " ")

    fout.write("\n".join(row) + '\n</row>\n')

#####################
# write last part
####################
fout.write(footer)
# close the LyX file
fout.close()
