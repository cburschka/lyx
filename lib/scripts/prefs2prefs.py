# -*- coding: utf-8 -*-

# file prefs2prefs.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Richard Kimberly Heck

# Full author contact details are available in file CREDITS

# This is the main file for the user preferences conversion system.
# There are two subsidiary files:
#    prefs2prefs_lfuns.py
#    prefs2prefs_prefs.py
# The former is used to convert bind and ui files; the latter, to convert
# the preferences file. The converter functions are all in the subsidiary 
# files. 
# 
# The format of the existing files was format 0, as of 2.0.alpha6.

from __future__ import print_function
import os, re, string, sys
from getopt import getopt
import io

###########################################################
# Utility functions, borrowed from layout2layout.py

def trim_bom(line):
	" Remove byte order mark."
	if line[0:3] == u"\357\273\277":
		return line[3:]
	else:
		return line


def read(source):
	" Read input file and strip lineendings."
	lines = source.read().splitlines() or ['']
	lines[0] = trim_bom(lines[0])
	return lines


def write(output, lines):
	" Write output file with native lineendings."
	output.write(os.linesep.join(lines) + os.linesep)


# for use by find_format_lines
re_comment = re.compile(r'^#')
re_empty   = re.compile(r'^\s*$')

def find_format_line(lines):
	'''
	Returns (bool, int), where int is number of the line the `Format'
	specification is on, or else the number of the first non-blank,
	non-comment line. The bool tells whether we found a format line.
	'''
	for i in range(len(lines)):
		l = lines[i]
		if re_comment.search(l) or re_empty.search(l):
			continue
		m = re_format.search(l)
		if m:
			return (True, i)
		# we're done when we have hit a non-comment, non-empty line
		break
	return (False, i)


# for use by get_format
re_format  = re.compile(r'^Format\s+(\d+)\s*$')

def get_format(lines):
	" Gets format of current file and replaces the format line with a new one "
	(found, format_line) = find_format_line(lines)
	if not found:
		return 0
	line = lines[format_line]
	m = re_format.search(line)
	if not m:
		sys.stderr.write("Couldn't match format line!\n" + line + "\n")
		sys.exit(1)
	return int(m.group(1))


def update_format(lines):
	" Writes new format line "
	(found, format_line) = find_format_line(lines)
	if not found:
		lines[format_line:format_line] = ("Format 1", "")
		return

	line = lines[format_line]
	m = re_format.search(line)
	if not m:
		sys.stderr.write("Couldn't match format line!\n" + line + "\n")
		sys.exit(1)
	format = int(m.group(1))
	lines[format_line] = "Format " + str(format + 1)


def abort(msg):
	sys.stderr.write("\n%s\n" % (msg))
	sys.exit(10)

#
###########################################################

def usage():
	print ("%s [-l] [-p] infile outfile" % sys.argv[0])
	print ("or: %s [-l] [-p] <infile >outfile" % sys.argv[0])
	print ("  -l: convert LFUNs (bind and ui files)")
	print ("  -p: convert preferences")
	print ("Note that exactly one of -l and -p is required.")


def main(argv):
	try:
		(options, args) = getopt(sys.argv[1:], "lp")
	except:
		usage()
		abort("Unrecognized option")

	opened_files = False
	# Open files
	if len(args) == 0:
		source = sys.stdin
		output = sys.stdout
	elif len(args) == 2:
		source = io.open(args[0], 'r', encoding='utf_8', errors='surrogateescape')
		output = io.open(args[1], 'w', encoding='utf_8', newline='\n')
		opened_files = True
	else:
		usage()
		abort("Either zero or two arguments must be given.")

	conversions = False

	for (opt, param) in options:
		if opt == "-l":
			from prefs2prefs_lfuns import conversions
		elif opt == "-p":
			from prefs2prefs_prefs import conversions

	if not conversions:
		usage()
		abort("Neither -l nor -p given.")
	elif len(options) > 1:
		usage()
		abort("Only one of -l or -p should be given.")

	current_format = len(conversions)
	lines = read(source)
	format = get_format(lines)

	while format < current_format:
		target_format, convert = conversions[format]
		old_format = format

		# make sure the conversion list is sequential
		if int(old_format) + 1 != target_format:
			abort("Something is wrong with the conversion chain.")

		for c in convert:
			try:
				# first see if the routine will accept a list of lines
				c(lines)
			except:
				# if not, it wants individual lines
				for i in range(len(lines)):
					(update, newline) = c(lines[i])
					if update:
						lines[i] = newline

		update_format(lines)
		format = get_format(lines)

		# sanity check
		if int(old_format) + 1 != int(format):
			abort("Failed to convert to new format!")

	write(output, lines)

	# Close files
	if opened_files:
		source.close()
		output.close()

	return 0


if __name__ == "__main__":
	main(sys.argv)
