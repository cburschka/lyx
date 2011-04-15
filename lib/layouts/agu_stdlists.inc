# Textclass definition file for AGU-Article.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>
#          Martin Vermeer <martin.vermeer@hut.fi>

# This is the counterpart of stdlists.inc
# It is desirable, as far as possible, to have the same look and feel for
# related layouts in latex and docbook.

Format 35
Input stdlists.inc


Style Itemize
	LatexType             Item_Environment
	LatexName             lst
	Latexparam            "type=<bulleted>"
	ItemTag               item
End


Style Enumerate
	LatexType             Item_Environment
	LatexName             lst
	Latexparam            "type=<ordered>"
	ItemTag               item
End


Style Description
	LatexType             Item_environment
	LatexName             lst
	Latexparam            "type=<unordered>"
	ItemTag               item
End

# maybe List is the same as simplelist (I need to check it).
NoStyle List

