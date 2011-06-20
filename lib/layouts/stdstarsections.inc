# Standard textclass definition file. Taken from initial LyX source code
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Transposed by Pascal André <andre@via.ecp.fr>
# Heavily modifed and enhanced by several developers.

# This include files contains all non numbered versions of the
# sectioning-related commands that are useful for article-like
# document classes, but not for letters. It should be Input after
# modifying the style of the regular sectioning layouts.


Format 35
Style Part*
	CopyStyle	Part
	Category	Unnumbered
	Margin		Static
	LatexName	part*
	LabelType	No_Label
	LabelCounter	""
	OptionalArgs	0
End


Style Chapter*
	CopyStyle	Chapter
	Category	Unnumbered
	Margin		Static
	LatexName	chapter*
	LabelType	No_Label
	LabelCounter	""
	OptionalArgs	0
End


Style Section*
	CopyStyle	Section
	Category	Unnumbered
	Margin		Static
	LatexName	section*
	LabelType	No_Label
	LabelCounter	""
	OptionalArgs	0
End


Style Subsection*
	CopyStyle	Subsection
	Category	Unnumbered
	Margin	Static
	LatexName	subsection*
	LabelType	No_Label
	LabelCounter	""
	OptionalArgs	0
End


Style Subsubsection*
	CopyStyle	Subsubsection
	Category	Unnumbered
	Margin		Static
	LatexName	subsubsection*
	LabelType	No_Label
	LabelCounter	""
	OptionalArgs	0
End


Style Paragraph*
	CopyStyle	Paragraph
	Category	Unnumbered
	Margin		Static
	LatexName	paragraph*
	LabelType	No_Label
	LabelCounter	""
	OptionalArgs	0
End


Style Subparagraph*
	CopyStyle	Subparagraph
	Category	Unnumbered
	Margin		Static
	LatexName	subparagraph*
	LabelType	No_Label
	LabelCounter	""
	OptionalArgs	0
End
