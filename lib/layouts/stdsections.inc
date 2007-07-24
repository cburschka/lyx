# Standard textclass definition file. Taken from initial LyX source code
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Transposed by Pascal André <andre@via.ecp.fr>
# Heavily modifed and enhanced by several developers.

# This include files contains all numbered sectionning-related
# commands that are useful for article-like document classes, but not
# for letters.

Format 4
Style Part
	Margin                Dynamic
	LatexType             Command
	LatexName             part
	NeedProtect           1
	NextNoIndent          1
	Labelsep              xxx
	ParSkip               0.4
	TopSep                4
	BottomSep             4
	ParSep                0.8
	Align                 Center
	Alignpossible         Center
	Font
	  Series              Bold
	  Size                Huge
	EndFont
End


Style Chapter
	Margin                Static
	LatexType             Command
	LatexName             chapter
	NeedProtect           1
	NextNoIndent          1
	ParSkip               0.4
	TopSep                4
	BottomSep             0.8
	ParSep                0.8
	Align                 Block
	AlignPossible         Block, Left
	OptionalArgs          1
	Font
	  Series              Bold
	  Size                Huge
	EndFont
End


Style Section
	Margin                Dynamic
	LatexType             Command
	LatexName             section
	NeedProtect           1
	NextNoIndent          1
	LabelSep              xxx
	ParSkip               0.4
	TopSep                1.3
	BottomSep             0.7
	ParSep                0.7
	Align                 Block
	AlignPossible         Block, Left
	OptionalArgs          1
	Font
	  Series              Bold
	  Size                Larger
	EndFont
End


Style Subsection
	CopyStyle             Section
	LatexName             subsection
	TopSep                0.9
	BottomSep             0.5
	ParSep                0.5
	Font
	  Series              Bold
	  Size                Large
	EndFont
End


Style Subsubsection
	CopyStyle             Subsection
	LatexName             subsubsection
	TopSep                0.7
	BottomSep             0.4
	Font
	  Series              Bold
	  Size                Normal
	EndFont
End


Style Paragraph
	CopyStyle             Subsubsection
	LatexName             paragraph
	TopSep                0.4
	BottomSep             0
End


Style Subparagraph
	CopyStyle             Paragraph
	LatexName             subparagraph
	NextNoIndent          0
	LeftMargin            MM
End

Input numarticle.inc
