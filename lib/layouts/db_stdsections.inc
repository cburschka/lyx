# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>

# This file is the counterpart of stdsections.inc
# It is desireable, as far as possible, to have the same look and feeling for
# related layouts in latex and docbook.

Input stdsections.inc

# Part style definition
Style Part
  LatexType		Command
  LatexName		part
  LatexParam		"0|"
End

# Chapter style definition
Style Chapter
  LatexType             Command
  LatexName             chapter
  LatexParam		"1|"
End

# Section style definition
Style Section
  LatexType		Command
  LatexName		sect1
  LatexParam		"2|"
End

# Subsection style definition
Style Subsection
  LatexType		Command
  LatexName		sect2
  LatexParam		"3|"
End

# Subsubsection style definition
Style Subsubsection
  LatexType		Command
  LatexName		sect3
  LatexParam		"4|"
End

# Paragraph style definition
Style Paragraph
  LatexType		Command
  LatexName		sect4
  LatexParam		"5|"
End

# Subparagraph style definition
Style Subparagraph
  LatexType		Command
  LatexName		sect5
  LatexParam		"6|"
End

