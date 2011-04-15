# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>

# This is the counterpart of lyxmacros.inc
# It is desirable, as far as possible, to have the same look and feel for
# related layouts in latex and docbook.

Format 35
Input lyxmacros.inc

Style Code
	CopyStyle             LyX-Code
	LatexType             Environment
	LatexName             screen
	PassThru              1
	ParbreakIsNewline     1
End

Style LyX-Code
	ObsoletedBy           Code
End

NoStyle LyX-Code
NoStyle Address
NoStyle Right_Address
