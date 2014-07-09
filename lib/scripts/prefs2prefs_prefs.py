# -*- coding: utf-8 -*-

# file prefs2prefs-prefs.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Richard Heck

# Full author contact details are available in file CREDITS

# This file houses conversion information for the preferences file.

# The converter functions take a line as argument and return a list: 
# 	(Bool, NewLine), 
# where the Bool says if  we've modified anything and the NewLine is 
# the new line, if so, which will be used to replace the old line.

# Incremented to format 2, r39670 by jrioux
#   Support for multiple file extensions per format.
#   No conversion necessary.

# Incremented to format 3, r39705 by tommaso
#   Support for file formats that are natively (g)zipped.
#   We must add the flag zipped=native to formats that
#   were previously hardcoded in the C++ source: dia.

# Incremented to format 4, r40028 by vfr
#   Remove support for default paper size.

# Incremented to format 5, r40030 by vfr
#   Add a default length unit.
#   No conversion necessary.

# Incremented to format 6, r40515 by younes
#   Add use_qimage option.
#   No conversion necessary.

# Incremented to format 7, r40789 by gb
#   Add mime type to file format

# Incremented to format 8, 288c1e0f by rgh
#   Add "nice" flag for converters
#   No conversion necessary.

# Incremented to format 9, a18af620 by spitz
#  Remove default_language rc.

# Incremented to format 10, 4985015 by tommaso
#  Add close_buffer_with_last_view in preferences.
#  No conversion necessary.

# Incremented to format 11, by gb
#   Split pdf format into pdf and pdf6

# Incremented to format 12, by vfr
#   Add option to use the system's theme icons
#   No conversion necessary.

# Incremented to format 13, by bh
#   Rename mac_like_word_movement to mac_like_cursor_movement

# Incremented to format 14, by spitz
#   New RC default_otf_view_format
#   No conversion necessary.

# Incremented to format 15, by prannoy
#   Add fullscreen_statusbar
#   No conversion necessary.

# Incremented to format 16, by lasgouttes
#  Remove force_paint_single_char rc.

# Incremented to format 17, by lasgouttes
#  Remove rtl_support rc.

# NOTE: The format should also be updated in LYXRC.cpp and
# in configure.py.

import re

###########################################################
#
# Conversion chain

def get_format(line):
	entries = []
	i = 0
	while i < len(line):
		if line[i] == '"':
			beg = i + 1
			i = i + 1
			while i < len(line) and line[i] != '"':
				if line[i] == '\\' and i < len(line) - 1 and line[i+1] == '"':
					# convert \" to "
					i = i + 1
				i = i + 1
			end = i
			entries.append(line[beg:end].replace('\\"', '"'))
		elif line[i] == '#':
			return entries
		elif not line[i].isspace():
			beg = i
			while i < len(line) and not line[i].isspace():
				i = i + 1
			end = i
			entries.append(line[beg:end])
		i = i + 1
	return entries


def simple_renaming(line, old, new):
	i = line.lower().find(old.lower())
	if i == -1:
		return no_match
	line = line[:i] + new + line[i+len(old):]
	return (True, line)

no_match = (False, [])

######################################
### Format 1 conversions (for LyX 2.0)

def remove_obsolete(line):
	tags = ("\\use_tempdir", "\\spell_command", "\\personal_dictionary",
				"\\plaintext_roff_command", "\\use_alt_language", 
				"\\use_escape_chars", "\\use_input_encoding",
				"\\use_personal_dictionary", "\\use_pspell",
				"\\use_spell_lib")
	line = line.lower().lstrip()
	for tag in tags:
		if line.lower().startswith(tag):
			return (True, "")
	return no_match


def language_use_babel(line):
	if not line.lower().startswith("\language_use_babel"):
		return no_match
	re_lub = re.compile(r'^\\language_use_babel\s+"?(true|false)', re.IGNORECASE)
	m = re_lub.match(line)
	val = m.group(1)
	newval = '0'
	if val == 'false':
		newval = '3'
	newline = "\\language_package_selection " + newval
	return (True, newline)


def language_package(line):
	return simple_renaming(line, "\\language_package", "\\language_custom_package")


lfre = re.compile(r'^\\converter\s+"?(\w+)"?\s+"?(\w+)"?\s+"([^"]*?)"\s+"latex"', re.IGNORECASE)
def latex_flavor(line):
	if not line.lower().startswith("\\converter"):
		return no_match
	m = lfre.match(line)
	if not m:
		return no_match
	conv = m.group(1)
	fmat = m.group(2)
	args = m.group(3)
	conv2fl = {
		   "luatex":   "lualatex",
		   "pplatex":  "latex",
		   "xetex":    "xelatex",
		  }
	if conv in conv2fl.keys():
		flavor = conv2fl[conv]
	else:
		flavor = conv
	if flavor == "latex":
		return no_match
	return (True,
		"\\converter \"%s\" \"%s\" \"%s\" \"latex=%s\"" % (conv, fmat, args, flavor))


emre = re.compile(r'^\\format\s+(.*)\s+"(document[^"]*?)"', re.IGNORECASE)
def export_menu(line):
	if not line.lower().startswith("\\format"):
		return no_match
	m = emre.match(line)
	if not m:
		return no_match
	fmat = m.group(1)
	opts = m.group(2)
	return (True,
		"\\Format %s \"%s,menu=export\"" % (fmat, opts))

# End format 1 conversions (for LyX 2.0)
########################################

#################################
# Conversions from LyX 2.0 to 2.1
zipre = re.compile(r'^\\format\s+("?dia"?\s+.*)\s+"([^"]*?)"', re.IGNORECASE)
def zipped_native(line):
	if not line.lower().startswith("\\format"):
		return no_match
	m = zipre.match(line)
	if not m:
		return no_match
	fmat = m.group(1)
	opts = m.group(2)
	return (True,
		"\\Format %s \"%s,zipped=native\"" % (fmat, opts))

def remove_default_papersize(line):
	if not line.lower().startswith("\\default_papersize"):
		return no_match
	return (True, "")

def add_mime_types(line):
	if not line.lower().startswith("\\format"):
		return no_match
	entries = get_format(line)
	converted = line
	i = len(entries)
	while i < 7:
		converted = converted + '	""'
		i = i + 1
	formats = {'tgif':'application/x-tgif', \
		'fig':'application/x-xfig', \
		'dia':'application/x-dia-diagram', \
		'odg':'application/vnd.oasis.opendocument.graphics', \
		'svg':'image/svg+xml', \
		'bmp':'image/x-bmp', \
		'gif':'image/gif', \
		'jpg':'image/jpeg', \
		'pbm':'image/x-portable-bitmap', \
		'pgm':'image/x-portable-graymap', \
		'png':'image/x-png', \
		'ppm':'image/x-portable-pixmap', \
		'tiff':'image/tiff', \
		'xbm':'image/x-xbitmap', \
		'xpm':'image/x-xpixmap', \
		'docbook-xml':'application/docbook+xml', \
		'dot':'text/vnd.graphviz', \
		'ly':'text/x-lilypond', \
		'latex':'text/x-tex', \
		'text':'text/plain', \
		'gnumeric':'application/x-gnumeric', \
		'excel':'application/vnd.ms-excel', \
		'oocalc':'application/vnd.oasis.opendocument.spreadsheet', \
		'xhtml':'application/xhtml+xml', \
		'bib':'text/x-bibtex', \
		'eps':'image/x-eps', \
		'ps':'application/postscript', \
		'pdf':'application/pdf', \
		'dvi':'application/x-dvi', \
		'html':'text/html', \
		'odt':'application/vnd.oasis.opendocument.text', \
		'sxw':'application/vnd.sun.xml.writer', \
		'rtf':'application/rtf', \
		'doc':'application/msword', \
		'csv':'text/csv', \
		'lyx':'application/x-lyx', \
		'wmf':'image/x-wmf', \
		'emf':'image/x-emf'}
	if entries[1] in formats.keys():
		converted = converted + '	"' + formats[entries[1]] + '"'
	else:
		converted = converted + '       ""'
	return (True, converted)

re_converter = re.compile(r'^\\converter\s+', re.IGNORECASE)

def split_pdf_format(line):
	# strictly speaking, a new format would not require to bump the
	# version number, but the old pdf format was hardcoded at several
	# places in the C++ code, so an update seemed like a good idea.
	if line.lower().startswith("\\format"):
		entries = get_format(line)
		if entries[1] == 'pdf':
			if len(entries) < 6:
				viewer = ''
			else:
				viewer = entries[5]
			converted = line.replace('application/pdf', '') + '''
\Format pdf6       pdf    "PDF (graphics)"        "" "''' + viewer + '"	""	"vector"	"application/pdf"'
			return (True, converted)
	elif line.lower().startswith("\\viewer_alternatives") or \
	     line.lower().startswith("\\editor_alternatives"):
		entries = get_format(line)
		if entries[1] == 'pdf':
			converted = line + "\n" + entries[0] + ' pdf6 "' + entries[2] + '"'
			return (True, converted)
	elif re_converter.match(line):
		entries = get_format(line)
		# The only converter from pdf that is touched is pdf->eps:
		# All other converters are likely meant for further processing on export.
		# The only converter to pdf that stays untouched is dvi->pdf:
		# All other converters are likely meant for graphics.
		if len(entries) > 2 and \
		   ((entries[1] == 'pdf' and entries[2] == 'eps') or \
		   (entries[1] != 'ps'  and entries[2] == 'pdf')):
			if entries[1] == 'pdf':
				converted = entries[0] + ' pdf6 ' + entries[2]
			else:
				converted = entries[0] + ' ' + entries[1] + ' pdf6'
			i = 3
			while i < len(entries):
				converted = converted + ' "' + entries[i] + '"'
				i = i + 1
			return (True, converted)
	return no_match

def remove_default_language(line):
	if not line.lower().startswith("\\default_language"):
		return no_match
	return (True, "")

def mac_cursor_movement(line):
	return simple_renaming(line, "\\mac_like_word_movement", "\\mac_like_cursor_movement")

# End conversions for LyX 2.0 to 2.1
####################################


#################################
# Conversions from LyX 2.1 to 2.2

def remove_force_paint_single_char(line):
	if not line.lower().startswith("\\force_paint_single_char"):
		return no_match
	return (True, "")

def remove_rtl(line):
	if not line.lower().startswith("\\rtl "):
		return no_match
	return (True, "")

# End conversions for LyX 2.1 to 2.2
####################################

conversions = [
	[  1, [ # there were several conversions for format 1
		export_menu,
		latex_flavor,
		remove_obsolete,
		language_use_babel,
		language_package
	]],
	[ 2, []],
	[ 3, [ zipped_native ]],
	[ 4, [ remove_default_papersize ]],
	[ 5, []],
	[ 6, []],
	[ 7, [add_mime_types]],
	[ 8, []],
	[ 9, [ remove_default_language ]],
	[ 10, []],
	[ 11, [split_pdf_format]],
	[ 12, []],
	[ 13, [mac_cursor_movement]],
	[ 14, []],
	[ 15, []],
	[ 16, [remove_force_paint_single_char]],
	[ 17, [remove_rtl]]
]
