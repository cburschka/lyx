# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>
# Modified Martin Vermeer <martin.vermeer@hut.fi>
#
# This file is the counterpart of stdstyle.inc
# It is desirable, as far as possible, to have the same look and feel for
# related layouts in latex and docbook.

Format 35
Input stdtitle.inc


Style Title
	LatexType             Command
	LatexName             articleinfo
	CommandDepth          2
	InnerTag              title
End


Style Author
	LatexType             Environment
	LatexName             author
End


Style Authorgroup
	CopyStyle             Author
	LatexType             Environment
	LatexName             authorgroup
	InnerTag              author
End


Style Date
	LatexType             Paragraph
	LatexName             date
	Align                 Center
	AlignPossible         Left,Center
End


Style RevisionHistory
	Margin                Static
	LatexType             Command
	LatexName             revhistory
	CommandDepth          3
	InnerTag              "!-- --"
	LeftMargin            MMM
	RightMargin           MMM
	ParIndent             MM
	TopSep                0.7
	BottomSep             0
	ParSep                0
	Align                 Block
	KeepEmpty             1
	LabelType             Centered_Top_Environment
	LabelString           "Revision History"
	LabelBottomSep        0.5
End


Style Revision
	Margin                Dynamic
	LatexType             Command
	LatexName             revision
	CommandDepth          4
	InnerTag              revnumber
	LabelSep              xxx
	ParSkip               0.4
	TopSep                0.9
	BottomSep             0.5
	ParSep                0.5
	Align                 Block
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
	LabelType             No_Label
End


Style FirstName
        ObsoletedBy   Standard
End

Style Surname
	ObsoletedBy   Standard
End
