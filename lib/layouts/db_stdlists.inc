# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>

# This is the counterpart of stdlists.inc
# It is desireable, as far as possible, to have the same look and feeling for
# related layouts in latex and docbook.

Input stdlists.inc


Style Itemize
	LatexType             Item_Environment
	LatexName             itemizedlist
End


Style Enumerate
	LatexType             Item_Environment
	LatexName             orderedlist
End


Style Description
	LatexType             Item_environment
	LatexName             variablelist
End

# maybe List is the same as simplelist (I need to check it).
NoStyle List

