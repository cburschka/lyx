# Letter textclass definition file.
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Heavily modifed and enhanced by several developers.


Format 93
Columns                 1
Sides                   1
PageStyle               Empty


Style Standard
	Category              MainText
	Margin                Static
	LatexType             Paragraph
	LatexName             dummy
	ParIndent             MM
	ParSkip               0.4
	Align                 Block
	AlignPossible         Block, Left, Right, Center
	LabelType             No_Label
	DocBookTag            para
End


Style My_Address
	Category              "Postal Data"
	Margin                Right_Address_Box
	LatexType             Command
	LatexName             address
	ParSkip               0.4
	BottomSep             1.5
	ParSep                1.5
	Align                 Left
	DocBookTag            address
End


Style Send_To_Address
	Category              "Postal Data"
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
	DocBookTag            address
	DocBookAttr           role='send-to-address'
End


Style Opening
	Category              Letter
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
	  Color               textlabel3
	EndFont
	DocBookTag            para
	DocBookAttr           role='opening'
End


Style Signature
	Category              Closings
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
	  Color               textlabel3
	EndFont
	DocBookTag            para
	DocBookAttr           role='signature'
End


Style Closing
	CopyStyle             Signature
	LatexName             closing
	LabelString           "Closing:"
	DocBookTag            para
	DocBookAttr           role='opening'
End


Style cc
	CopyStyle             Signature
	LatexName             cc
	LabelString           "cc:"
	LabelFont
	  Series              Bold
	  Shape               Italic
	  Color               textlabel2
	EndFont
	DocBookTag            para
	DocBookAttr           role='carbon-copy'
End


Style encl
	CopyStyle             cc
	LatexName             encl
	LabelString           "encl:"
	Spacing               Other 1.5
	DocBookTag            para
	DocBookAttr           role='enclosed'
End


Style Location
	CopyStyle             cc
	LatexName             thispagestyle{firstpage}\location
	LabelString           "Location:"
	DocBookTag            otheraddr
	DocBookAttr           role='place'
	DocBookTagType        paragraph
End


Style Telephone
	CopyStyle             cc
	LatexName             thispagestyle{firstpage}\telephone
	LabelString           "Telephone:"
	DocBookTag            phone
	DocBookAttr           ""
	DocBookTagType        paragraph
End
