# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>

# This is the counterpart of lyxmacros.inc
# It is desireable, as far as possible, to have the same look and feeling for
# related layouts in latex and docbook.

Input lyxmacros.inc

# Code style definition
Style Code
  CopyStyle		LyX-Code
  LatexType		Environment
  LatexName		programlisting
  LatexParam		CDATA
  PassThru		1
End

# Comment style definition
Style Comment
  LatexType		Paragraph
  LatexName		remark
End

Style LyX-Code
  ObsoletedBy		Code
End

NoStyle LyX-Code

NoStyle Address

NoStyle Right_Address