# Letter textclass definition file.
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Heavily modifed and enhanced by several developers.


Format 35
Columns                 1
Sides                   1
PageStyle               Empty


Style Standard
	Margin                Static
	LatexType             Paragraph
	LatexName             dummy
	ParIndent             MM
	ParSkip               0.4
	Align                 Block
	AlignPossible         Block, Left, Right, Center
	LabelType             No_Label
End


Style My_Address
	Margin                Right_Address_Box
	LatexType             Command
	LatexName             address
	ParSkip               0.4
	BottomSep             1.5
	ParSep                1.5
	Align                 Left
End


Style Send_To_Address
	Margin                Static
	LatexType             Command
	LatexName             letter
	KeepEmpty             1
	ParSkip               0.4
	BottomSep             1.5
	ParSep                1.5
	Align                 Left
	AlignPossible         Block, Left, Right, Center
	LabelType             No_Label
End


Style Opening
	Margin                Static
	LatexType             Command
	LatexName             opening
	KeepEmpty             1
	LeftMargin            Signature:xx
	LabelSep              xx
	TopSep                0.4
	BottomSep             0.4
	ParSep                0.4
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Static
	LabelString           "Opening:"
	LabelFont
	  Series              Bold
	  Shape               Italic
	  Color               red
	EndFont
End


Style Signature
	Margin                Static
	LatexType             Command
	LatexName             signature
	LeftMargin            Signature:xx
	LabelSep              xx
	TopSep                0.4
	BottomSep             0.4
	ParSep                0.4
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Static
	LabelString           "Signature:"
	LabelFont
	  Series              Bold
	  Shape               Italic
	  Color               red
	EndFont
End


Style Closing
	CopyStyle             Signature
	LatexName             closing
	LabelString           "Closing:"
End


Style cc
	CopyStyle             Signature
	LatexName             cc
	LabelString           "cc:"
	LabelFont
	  Series              Bold
	  Shape               Italic
	  Color               green
	EndFont
End


Style encl
	CopyStyle             cc
	LatexName             encl
	LabelString           "encl:"
	Spacing               Other 1.5
End


Style Location
	CopyStyle             cc
	LatexName             thispagestyle{firstpage}\location
	LabelString           "Location:"
End


Style Telephone
	CopyStyle             cc
	LatexName             thispagestyle{firstpage}\telephone
	LabelString           "Telephone:"
End
