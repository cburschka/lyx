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


current_format = 1

###########################################################
#
# Conversion chain

no_match = (False, [])

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


conversions = [
	[ # this will be a long list of conversions for format 0
    remove_obsolete
	] # end conversions for format 0
]
