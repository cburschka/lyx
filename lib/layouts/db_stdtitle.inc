# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>

# This file is the counterpart of stdstile.inc
# It is desireable, as far as possible, to have the same look and feeling for
# related layouts in latex and docbook.

Input stdtitle.inc

# Title style definition
Style Title
  LatexType		Command
  LatexName		artheader
  LatexParam		"2|"
End

# Author style definition
Style Author
  LatexType		Environment
  LatexName		author
  KeepEmpty		1
End

# first name style definition
Style FirstName
  Margin  		Static
  LatexType 		Paragraph
  LatexName 		firstname
  ParIndent		MM
  ParSkip 		0.4
  Align 		Center
  AlignPossible 	Center
  LabelType 		No_Label
End

# surname style definition
Style Surname
  Margin  		Static
  LatexType 		Paragraph
  LatexName 		surname
  ParIndent		MM
  ParSkip 		0.4
  Align 		Center
  AlignPossible 	Center
  LabelType 		No_Label
End

# Date style definition
Style Date
  LatexType		Paragraph
  LatexName		date
End
