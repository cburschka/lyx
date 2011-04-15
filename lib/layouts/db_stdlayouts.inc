# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>

# This is the counterpart of stdlayouts.inc
# It is desirable, as far as possible, to have the same look and feel for
# related layouts in latex and docbook.

Format 35
Input stdlayouts.inc

Style Literal
	LatexType             Environment
	LatexName             literallayout
	PassThru              1
	ParbreakIsNewline     1
End


Style Quotation
	LatexType             Environment
	LatexName             blockquote
	InnerTag              para
End

# Quote should be an inline element
NoStyle Quote
NoStyle Verse



Style SGML
	ObsoletedBy	      Standard
End
