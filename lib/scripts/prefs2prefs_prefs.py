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

conversions = [
	[ # this will be a long list of conversions for format 0
		
	] # end conversions for format 0
]
