# Author : André Pönitz
# This include files contains a label definitions for a report-like numbering.

Style Part
	LabelType             Counter
	LabelCounter          part
	LabelString           "Part \Roman{part}"
End
	

Style Chapter
	LabelType             Counter
	LabelCounter          chapter
	LabelString           "Chapter \arabic{chapter}"
	LabelStringAppendix   "Appendix \Alph{chapter}"
	TocLevel              1
End


Style Section
	LabelType             Counter
	LabelCounter          section
	LabelString           "\arabic{chapter}.\arabic{section}"
	LabelStringAppendix   "\Alph{chapter}.\arabic{section}"
	TocLevel              2
End


Style Subsection
	LabelType             Counter
	LabelCounter          subsection
	LabelString           "@Section@.\arabic{subsection}"
	LabelStringAppendix   "@Section@.\arabic{subsection}"
	TocLevel              3
End


Style Subsubsection
	LabelType             Counter
	LabelCounter          subsubsection
	LabelString           "@Subsection@.\arabic{subsubsection}"
	LabelStringAppendix   "@Subsection@.\arabic{subsubsection}"
	TocLevel              4
End


Style Paragraph
	TocLevel              5
End


Style Subparagraph
	TocLevel              6
End
