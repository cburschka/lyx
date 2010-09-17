#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file prefs2prefs-lfuns.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Richard Heck

# Full author contact details are available in file CREDITS

# This file houses conversion information for the bind and ui files,
# i.e., for files where we are converting lfuns.

# The converter functions take a line as argument and return a list: 
# 	(Bool, NewLine), 
# where the Bool says if  we've modified anything and the NewLine is 
# the new line, if so, which will be used to replace the old line.


import sys, re

current_format = 1

###########################################################
#
# Actual converter functions
#
# These accept a line as argument and should return a list:
#  (bool, newline)
# where the bool indicates whether we changed anything. In 
# that case, one normally returns: (False, []).


def simple_renaming(line, old, new):
	if line.find(old) == -1:
		return (False, [])
	line = line.replace(old, new)
	return (True, line)


def next_inset_modify(line):
	return simple_renaming(line, "next-inset-modify", "inset-modify")


def next_inset_toggle(line):
	return simple_renaming(line, "next-inset-toggle", "inset-toggle")


#
#
###########################################################


# Conversion chain

conversions = [
	[ # this will be a long list of conversions for format 0
		next_inset_toggle,
		next_inset_modify
	] # end conversions for format 0
]

