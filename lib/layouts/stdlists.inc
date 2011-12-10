# Standard textclass definition file. Taken from initial LyX source code
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Transposed by Pascal André <andre@via.ecp.fr>
# Heavily modifed and enhanced by several developers.

# This include files contains various standard environments for lists.


Format 35

Input stdlyxlist.inc

Style Itemize
	Category              List
	Margin                Static
	LatexType             Item_Environment
	LatexName             itemize
	NextNoIndent          1
	LeftMargin            MMN
	LabelSep              xx
	ItemSep               0.2
	TopSep                0.7
	BottomSep             0.7
	ParSep                0.3
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Itemize
	HTMLTag               ul
	HTMLItem              li
	HTMLLabel             NONE
End


Style Enumerate
	Category              List
	Margin                Static
	LatexType             Item_Environment
	LatexName             enumerate
	NextNoIndent          1
	LeftMargin            MMN
	LabelSep              xx
	ParSkip               0.0
	ItemSep               0.2
	TopSep                0.7
	BottomSep             0.7
	ParSep                0.3
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Enumerate
	LabelCounter          "enum"
	HTMLTag               ol
	HTMLItem              li
	HTMLLabel             NONE
	RefPrefix             enu
End

Style Description
	Category              List
	Margin                First_Dynamic
	LatexType             Item_environment
	LatexName             description
	NextNoIndent          1
	LeftMargin            MM
	LabelSep              xxx
	ParSkip               0.4
	ItemSep               0.2
	TopSep                0.7
	BottomSep             0.7
	ParSep                0.3
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Manual
	LabelFont
	  Series              Bold
	EndFont
	HTMLTag               dl
	HTMLAttr              class='description'
	HTMLItem              dd
	HTMLLabel             dt
	HTMLLabelFirst        1
	HTMLStyle
		dl.description dt { font-weight: bold; }
	EndHTMLStyle
End


Style List
  Obsoletedby           Labeling
End
