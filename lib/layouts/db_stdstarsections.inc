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
	CopyStyle             Part
	Margin                Static
	LatexName             bridgehead
	LatexType             Paragraph
	InnerTag	      "!-- --"
	LabelType             No_Label
	LatexParam	      renderas="part"
End


Style Chapter*
	CopyStyle             Chapter
	Margin                Static
	LatexType             Paragraph
	InnerTag	      "!-- --"
	LatexName             bridgehead
	LabelType             No_Label
	OptionalArgs          0
	LatexParam	      renderas="chapter"
End


Style Section*
	CopyStyle             Section
	Margin                Static
	LatexName             bridgehead
	LatexType             Paragraph
	InnerTag	      "!-- --"
	LabelType             No_Label
	OptionalArgs          0
	LatexParam	      renderas="sect1"
End


Style Subsection*
	CopyStyle             Subsection
	Margin                Static
	LatexName             bridgehead
	LatexType             Paragraph
	InnerTag	      "!-- --"
	LabelType             No_Label
	OptionalArgs          0
	LatexParam	      renderas="sect2"
End


Style Subsubsection*
	CopyStyle             Subsubsection
	Margin                Static
	LatexName             bridgehead
	LatexType             Paragraph
	InnerTag	      "!-- --"
	LabelType             No_Label
	OptionalArgs          0
	LatexParam	      renderas="sect3"
End


Style Paragraph*
	CopyStyle             Paragraph
	Margin                Static
	LatexName             bridgehead
	LatexType             Paragraph
	InnerTag	      "!-- --"
	LabelType             No_Label
	OptionalArgs          0
	LatexParam	      renderas="sect4"
End


Style Subparagraph*
	CopyStyle             Subparagraph
	Margin                Static
	LatexName             bridgehead
	LatexType             Paragraph
	InnerTag	      "!-- --"
	LabelType             No_Label
	OptionalArgs          0
	LatexParam	      renderas="sect5"
End
