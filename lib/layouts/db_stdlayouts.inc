# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>

# This is the counterpart of stdlayouts.inc
# It is desireable, as far as possible, to have the same look and feeling for
# related layouts in latex and docbook.

Input stdlayouts.inc

# SGML style definition
Style SGML
  CopyStyle		LaTeX
  LatexName		"!-- --"
End

NoStyle LaTeX

# Verbatim style definition
Style Literal
  LatexType		Environment
  LatexName		literallayout
  Font
    Latex               Latex
  EndFont
End

# Quote style definition
Style Quotation
  LatexType 		Environment
  LatexName 		blockquote
  LatexParam		para
End

# Quote should be an inline element
NoStyle Quote

NoStyle Verse

Style Caption
  LatexType		Paragraph
  LatexName		caption
End