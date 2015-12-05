# AGUplus textclass definition file.
# Author: Martin Vermeer <martin.vermeer@hut.fi>
#
# Necessary LaTeX files to be found on
#
# ftp://ftp.agu.org/journals/latex/journals


Format 58
Input stdclass.inc

SecNumDepth             4

Preamble
	\usepackage{times}
	\sectionnumbers
EndPreamble


NoStyle Chapter
NoCounter chapter
NoStyle Chapter*

NoStyle Subparagraph
NoCounter subparagraph
NoStyle Subparagraph*


Style Section
	Font
	  Size                Large
	EndFont
	TocLevel 1
End


Style Section*
	Font
	  Size                Large
	EndFont
End


Style Subsection
	Font
	  Size                Normal
	EndFont
	TocLevel 2
End


Style Subsection*
	Font
	  Size                Normal
	EndFont
End


Style Paragraph
	LatexName             subsubsection
	TocLevel 4
End


Style Affiliation
	CopyStyle             Author
	LatexName             affil
End


Style Paragraph*
	CopyStyle             Paragraph
	Margin                Static
	LatexName             subsubsubsection*
	LabelType             No_Label
	ResetArgs             1
End


Style Left_Header
	Margin                Static
	LatexType             Command
	LatexName             lefthead
	NextNoIndent          1
	LeftMargin            "Left Header:xx"
	LabelSep              xxx
	ParSkip               0.4
	TopSep                1.3
	BottomSep             0.7
	ParSep                0.7
	Align                 Left
	LabelType             Static
	LabelString           "Left Header:"
	InTitle               1
	LabelFont
	  Series              Bold
	  Shape               Italic
	  Color               red
	EndFont
End


Style Right_Header
	CopyStyle             Left_Header
	LatexName             righthead
	LeftMargin            "Right Header:xx"
	LabelString           "Right Header:"
End


Style Received
	CopyStyle             Left_Header
	LatexName             received
	LeftMargin            Received:xx
	LabelString           "Received:"
End


Style Revised
	CopyStyle             Left_Header
	LatexName             revised
	LeftMargin            Revised:xx
	LabelString           "Revised:"
End


Style Accepted
	CopyStyle             Left_Header
	LatexName             accepted
	LeftMargin            Accepted:xx
	LabelString           "Accepted:"
End


Style CCC
	CopyStyle             Left_Header
	LatexName             ccc
	LeftMargin            "CCC code:xx"
	LabelString           "CCC code:"
End


# cpright
# journalid
# articleid


Style PaperId
	CopyStyle             Left_Header
	LatexName             paperid
	LeftMargin            "Paper Id:xx"
	LabelString           "Paper Id:"
End


Style AuthorAddr
	CopyStyle             Left_Header
	LatexName             authoraddr
	LeftMargin            "Author Address:xx"
	LabelString           "Author Address:"
End


Style SlugComment
	CopyStyle             Left_Header
	LatexName             slugcomment
	LeftMargin            "Slug Comment:xx"
	LabelString           "Slug Comment:"
End


Style Bibliography
	TopSep                4
	LabelString           "Bibliography"
	LabelFont
	  Series              Bold
	  Size                Huge
	EndFont
End


Float
	Type                  plate
	GuiName               Plate
	Placement             htbp
	Extension             lof
	NumberWithin          none
	Style                 plain
	IsPredefined          true
	UsesFloatPkg          false
End


Float
	GuiName               Planotable
	Type                  planotable
	Placement             htbp
	Extension             lot
	NumberWithin          none
	Style                 Ruled
	IsPredefined          true
	UsesFloatPkg          false
End


InsetLayout Caption:Table
	LabelString          table
	LaTeXType            command
	LatexName            tablecaption
	NeedProtect          1
	MultiPar             false
	HTMLStyle
		div.float-caption {
			text-align: center;
			border: 2px solid black;
			padding: 1ex;
			margin: 1ex;
		}
	EndHTMLStyle
	HTMLAttr "class='float-caption float-caption-table'"
End


Counter plate
	Within                none
End


Counter planotable
	Within                none
End
