
# Standard textclass definition file. Taken from initial LyX source code
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Transposed by Pascal André <andre@via.ecp.fr>
# Heavily modifed and enhanced by several developers.

# This include files contains various standard environments for lists.


Format 8
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
	LabelString           "*"
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
	LabelString           "MM"
	LabelFont
	  Series              Bold
	EndFont
End


Style List
	Category              List
	Margin                Manual
	LatexType             List_Environment
	LatexName             lyxlist
	NextNoindent          1
	LabelSep              xxx
	ParSkip               0.4
	TopSep                0.7
	BottomSep             0.7
	ParSep                0.5
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Manual
	LabelString           "00.00.0000"

	#define the environment lyxlist
	Preamble
		\newenvironment{lyxlist}[1]
			{\begin{list}{}
				{\settowidth{\labelwidth}{#1}
				 \setlength{\leftmargin}{\labelwidth}
				 \addtolength{\leftmargin}{\labelsep}
				 \renewcommand{\makelabel}[1]{##1\hfil}}}
			{\end{list}}
	EndPreamble
End


#
# New environments (not active yet)
#
#Environment Itemize
#       LaTeXHeader "\begin{itemize}"
#       LaTeXFooter "\end{itemize}"
#  LabelString          "*"
#  Margin               Static
#  LatexType            Item_Environment
#  NextNoIndent         1
#  LeftMargin           MMN
#  LabelSep             xx
#  ItemSep              0.2
#  TopSep               0.7
#  BottomSep            0.7
#  ParSep               0.3
#  Align                Block
#  AlignPossible        Block, Left
#  LabelType            Static
#End

#Environment Enumerate
#  Margin               Static
#  LatexType            Item_Environment
#  LatexName            enumerate
#  NextNoIndent         1
#  LeftMargin           MMN
#  LabelSep             xx
#  ParSkip              0.0
#  ItemSep              0.2
#  TopSep               0.7
#  BottomSep            0.7
#  ParSep               0.3
#  Align                Block
#  AlignPossible        Block, Left
#  LabelType            Enumerate
#End

