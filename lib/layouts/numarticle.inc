# Author : André Pönitz
# This include files contains label definitions for a article-like numbering.

Style Part
	LabelType             Counter
	LabelCounter          part
	LabelString           "Part \Roman{part}"
End


Style Section
	LabelType             Counter
	LabelCounter          section
	LabelString           "\arabic{section}"
	LabelStringAppendix   "\Alph{section}"
	TocLevel              1
End


Style Subsection
	LabelType             Counter
	LabelCounter          subsection
	LabelString           "@Section@.\arabic{subsection}"
	LabelStringAppendix   "@Section@.\arabic{subsection}"
	TocLevel              2
End


Style Subsubsection
	LabelType             Counter
	LabelCounter          subsubsection
	LabelString           "@Subsection@.\arabic{subsubsection}"
	LabelStringAppendix   "@Subsection@.\arabic{subsubsection}"
	TocLevel              3
End


Style Paragraph
	TocLevel              4
End


Style Subparagraph
	TocLevel              5
End
