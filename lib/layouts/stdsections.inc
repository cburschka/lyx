# Standard textclass definition file. Taken from initial LyX source code
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Transposed by Pascal André <andre@via.ecp.fr>
# Heavily modifed and enhanced by several developers.

# This include files contains all numbered sectionning-related
# commands that are useful for article-like document classes, but not
# for letters.

Format 35

Style Part
	Category              Section
	Margin                Dynamic
	LabelString           "Part \thepart"
	LabelType             Counter
	TocLevel              0
	LabelCounter          part
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
	OptionalArgs          1
	Font
	  Series              Bold
	  Size                Huge
	EndFont
	HTMLTag               h1
End


Style Chapter
	Margin                Static
	Category              Section
	LabelString           "Chapter \thechapter"
	LabelStringAppendix   "Appendix \thechapter"
	LabelType             Counter
	LabelCounter          chapter
	TocLevel              0
	LatexType             Command
	LatexName             chapter
	NeedProtect           1
	NextNoIndent          1
	ParSkip               0.4
	TopSep                4
	BottomSep             0.8
	ParSep                0.8
	Align                 Block
	OptionalArgs          1
	Font
	  Series              Bold
	  Size                Huge
	EndFont
	HTMLTag               h1
End


Style Section
	Category              Section
	Margin                Dynamic
	LabelType             Counter
	LabelCounter          section
	TocLevel              1
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
	OptionalArgs          1
	Font
	  Series              Bold
	  Size                Larger
	EndFont
	HTMLTag               h2
End


Style Subsection
	CopyStyle             Section
	LabelCounter          subsection
	TocLevel              2
	LatexName             subsection
	TopSep                0.9
	BottomSep             0.5
	ParSep                0.5
	Font
	  Series              Bold
	  Size                Large
	EndFont
	HTMLTag               h3
End


Style Subsubsection
	CopyStyle             Subsection
	LabelCounter          subsubsection
	TocLevel              3
	LatexName             subsubsection
	TopSep                0.7
	BottomSep             0.4
	Font
	  Series              Bold
	  Size                Normal
	EndFont
	HTMLTag               h4
End


Style Paragraph
	CopyStyle             Subsubsection
	LabelCounter          paragraph
	TocLevel              4
	LatexName             paragraph
	TopSep                0.4
	BottomSep             0
End


Style Subparagraph
	CopyStyle             Paragraph
	LabelCounter          subparagraph
	TocLevel              5
	LatexName             subparagraph
	NextNoIndent          0
	LeftMargin            MM
End

Input stdrefprefix.inc
Input numarticle.inc
