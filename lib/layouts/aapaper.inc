# This file contains additional style definitions for the
# A&A paper style not found in the standard include files.
# It is Input by aapaper.layout
#
# Author: Peter Sütterlin <pit@uni-sw.gwdg.de>


Format 35
Style Subtitle
	Margin                Static
	LatexType             Command
	InTitle               1
	LatexName             subtitle
	NextNoIndent          1
	ParSkip               0.4
	ItemSep               0
	TopSep                1.3
	BottomSep             1.3
	ParSep                1
	Align                 Left
	LabelType             No_Label
	Font
	  Size                Normal
	  Series              Bold
	EndFont
End


Style Address
	Margin                Static
	LatexType             Command
	InTitle               1
	LatexName             institute
	ParSkip               0.4
	BottomSep             0.4
	Align                 Left
	AlignPossible         Block, Left, Right, Center
	LabelType             No_Label
	Font
	  Family              Sans
	  Size                Small
	EndFont
End


Style Email
	Margin                Static
	LatexType             Command
	InTitle               1
	LatexName             mail
	Align                 Left
	LeftMargin            email:M
	LabelType             Static
	LabelString           "email:"
	LabelSep              M
	Font
	  Family              Sans
	  Size                Small
	EndFont
End


Style Offprint
	Margin                Static
	LatexType             Command
	InTitle               1
	LatexName             offprints
	Align                 Left
	LeftMargin            "Offprint Requests to:M"
	LabelType             Static
	LabelString           "Offprint Requests to:"
	LabelSep              M
	Font
	  Family              Sans
	  Size                Small
	EndFont
End


Style Acknowledgement
	Margin                Static
	LatexType             Environment
	LatexName             acknowledgements
	NextNoIndent          1
	ItemSep               0
	TopSep                0.2
	BottomSep             0
	ParSep                0
	Align                 Block
	LabelType             Top_Environment
	LabelString           "Acknowledgements"
	LabelSep              M
	Font
	  Size                Small
	EndFont
	LabelFont
	  Size                Small
	  Shape               Italic
	EndFont
End


Style Thesaurus
	LabelType             Static
	LatexType             Command
	InTitle               1
	LatexName             thesaurus
	NextNoIndent          1
	TopSep                0
	BottomSep             1
	LeftMargin            "Thesaurus not supported in recent A&A:M"
	LabelSep              M
	LabelType             Static
	LabelString           "Thesaurus not supported in recent A&A:"
	Font
	  Size                Small
	EndFont
	LabelFont
	  Color               Red
	EndFont
End
