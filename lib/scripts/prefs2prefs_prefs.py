#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file prefs2prefs-lfuns.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Richard Heck

# Full author contact details are available in file CREDITS

# This file houses conversion information for the preferences file.

# The converter functions take a line as argument and return a list: 
# 	(Bool, NewLine), 
# where the Bool says if  we've modified anything and the NewLine is 
# the new line, if so, which will be used to replace the old line.

import re


###########################################################
#
# Conversion chain

def simple_renaming(line, old, new):
	if line.find(old) == -1:
		return no_match
	line = line.replace(old, new)
	return (True, line)

no_match = (False, [])

########################
### Format 1 conversions

def remove_obsolete(line):
	tags = ("\\use_tempdir", "\\spell_command", "\\personal_dictionary",
				"\\plaintext_roff_command", "\\use_alt_language", 
				"\\use_escape_chars", "\\use_input_encoding",
				"\\use_personal_dictionary", "\\use_pspell",
				"\\use_spell_lib")
	line = line.lstrip()
	for tag in tags:
		if line.startswith(tag):
			return (True, "")
	return no_match

def language_use_babel(line):
	if not line.startswith("\language_use_babel"):
		return no_match
	re_lub = re.compile(r'^\\language_use_babel\s+"?(true|false)')
	m = re_lub.match(line)
	val = m.group(1)
	newval = '0'
	if val == 'false':
		newval = '3'
	newline = "\\language_package_selection " + newval
	return (True, newline)

def language_package(line):
	return simple_renaming(line, "\\language_package", "\\language_custom_package")

lfre = re.compile(r'^\\converter\s+"?(\w+)"?\s+"?(\w+)"?\s+"([^"]*?)"\s+"latex"')
def latex_flavor(line):
	if not line.startswith("\\converter"):
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

emre = re.compile(r'^\\[Ff]ormat\s+(.*)\s+"(document[^"]*?)"')
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

########################


conversions = [
	[ # this will be a long list of conversions for format 0
		export_menu,
		latex_flavor,
		remove_obsolete,
		language_use_babel,
		language_package
	] # end conversions for format 0
]
