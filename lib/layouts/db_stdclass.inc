# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>

# This file is the counterpart of stdclass.inc
# It is desireable, as far as possible, to have the same look and feeling for
# related layouts in latex and docbook.

OutputType		docbook

# General textclass parameters
Columns			1
Sides			1
SecNumDepth		3
TocDepth		3

# This is just to show how to declare the default font.
# The defaults are exactly those shown here.
DefaultFont
  Family		Roman
  Series		Medium
  Shape			Up
  Size			Normal
  Color			None
EndFont

# Standard style definition
Style Standard
  Margin  		Static
  LatexType 		Paragraph
  LatexName 		para
  ParIndent		MM
  ParSkip 		0.4
  Align 		Block
  AlignPossible 	Block
  LabelType 		No_Label
End

Input db_stdlists.inc
Input db_stdsections.inc
Input db_stdstarsections.inc
Input db_stdtitle.inc
Input db_stdstruct.inc
Input db_lyxmacros.inc
Input db_stdlayouts.inc
