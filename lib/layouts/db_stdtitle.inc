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

# Authorgroup
Style Authorgroup
  CopyStyle             Author
  LatexType             Environment
  LatexName             authorgroup
  LatexParam            author
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
  Align			Left
  AlignPossible		Left,Center
End

# Revision History style definition
Style RevisionHistory
  Margin                Static
  LatexType             Command
  LatexName             revhistory
  LatexParam		"3|!-- --"
  LeftMargin            MMM
  RightMargin           MMM
  ParIndent             MM
  TopSep                0.7
  BottomSep             0
  ParSep                0
  Align                 Block
  AlignPossible         Block
  KeepEmpty		1
  LabelType             Centered_Top_Environment
  LabelString           "Revision History"
  LabelBottomSep        0.5
 
End

# Revision style definition
Style Revision
  Margin                Dynamic
  LatexType             Command
  LatexName             revision
  LatexParam		"4|revnumber"
  LabelSep              xxx
  ParSkip               0.4
  TopSep                0.9
  BottomSep             0.5
  ParSep                0.5
  Align                 Block
  AlignPossible         Block
 
End

# RevisionRemark style definition
Style RevisionRemark
  Margin                Static
  LatexType             Paragraph
  InTitle               1
  LatexName             revremark
  LabelSep              xxx
  ParIndent		MM
  ParSkip               0.0
  TopSep                0.0
  BottomSep             0.0
  ParSep                1
  Align                 Block
  AlignPossible         Block
  LabelType             No_Label
 
End
