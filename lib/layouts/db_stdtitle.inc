# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>
# Modified Martin Vermeer <martin.vermeer@hut.fi> 
#
# This file is the counterpart of stdstyle.inc
# It is desirable, as far as possible, to have the same look and feel for
# related layouts in latex and docbook.

Input stdtitle.inc


Style Title
	LatexType             Command
	LatexName             articleinfo
	CommandDepth          2
	InnerTag              "title"
End


Style Author
	LatexType             Environment
	LatexName             author
	KeepEmpty             1
End


Style Authorgroup
	CopyStyle             Author
	LatexType             Environment
	LatexName             authorgroup
	LatexParam            author
End


Style FirstName
	Margin                Static
	LatexType             Paragraph
	LatexName             firstname
	ParIndent             MM
	ParSkip               0.4
	Align                 Center
	AlignPossible         Center
	LabelType             No_Label
End


Style Surname
	Margin                Static
	LatexType             Paragraph
	LatexName             surname
	ParIndent             MM
	ParSkip               0.4
	Align                 Center
	AlignPossible         Center
	LabelType             No_Label
End


Style Date
	LatexType             Paragraph
	LatexName             date
	Align                 Left
	AlignPossible         Left,Center
End


Style RevisionHistory
	Margin                Static
	LatexType             Command
	LatexName             revhistory
	LatexParam            "3|!-- --"
	LeftMargin            MMM
	RightMargin           MMM
	ParIndent             MM
	TopSep                0.7
	BottomSep             0
	ParSep                0
	Align                 Block
	AlignPossible         Block
	KeepEmpty             1
	LabelType             Centered_Top_Environment
	LabelString           "Revision History"
	LabelBottomSep        0.5
End


Style Revision
	Margin                Dynamic
	LatexType             Command
	LatexName             revision
	LatexParam            "4|revnumber"
	LabelSep              xxx
	ParSkip               0.4
	TopSep                0.9
	BottomSep             0.5
	ParSep                0.5
	Align                 Block
	AlignPossible         Block
End


Style RevisionRemark
	Margin                Static
	LatexType             Paragraph
	InTitle               1
	LatexName             revremark
	LabelSep              xxx
	ParIndent             MM
	ParSkip               0.0
	TopSep                0.0
	BottomSep             0.0
	ParSep                1
	Align                 Block
	AlignPossible         Block
	LabelType             No_Label
End
