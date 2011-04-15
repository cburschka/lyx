# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>

# This is the counterpart of stdlists.inc
# It is desirable, as far as possible, to have the same look and feel for
# related layouts in latex and docbook.

Format 35
Input stdlists.inc


Style Itemize
	LatexType             Item_Environment
	LatexName             itemizedlist
	ItemTag               listitem
End


Style Enumerate
	LatexType             Item_Environment
	LatexName             orderedlist
	ItemTag               listitem
End


Style Description
	LatexType             Item_environment
	LatexName             variablelist
	InnerTag              varlistentry
	LabelTag              term
	ItemTag               listitem
End

# maybe List is the same as simplelist (I need to check it).
NoStyle List

