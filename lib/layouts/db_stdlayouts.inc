# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>

# This is the counterpart of stdlayouts.inc
# It is desireable, as far as possible, to have the same look and feeling for
# related layouts in latex and docbook.

Input stdlayouts.inc

# SGML style definition
Style SGML
  Margin                Static
  LatexType             Paragraph
  LatexName             "!-- --"
  NewLine               0
  ParIndent             MM
  ParSkip               0.4
  ParSep                0.5
  Align                 Left
  AlignPossible         Block
  LabelType             No_Label
  FreeSpacing           1
  PassThru		1
End

# Verbatim style definition
Style Literal
  LatexType		Environment
  LatexName		literallayout
  PassThru              1
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
  LatexName		title
End