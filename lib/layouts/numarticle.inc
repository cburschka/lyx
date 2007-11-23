# Author : André Pönitz
# This include file contains label definitions for an article-like numbering.

Format 4
Style Part
	LabelType             Counter
	LabelCounter          part
	LabelString           "Part \Roman{part}"
	TocLevel              0
End


# this will be removed later for real articles
Style Chapter
	LabelType             Counter
	LabelCounter          chapter
	TocLevel              0
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
	LabelType             Counter
	LabelCounter          paragraph
	LabelString           "@Subsubsection@.\arabic{paragraph}"
	LabelStringAppendix   "@Subsubsection@.\arabic{paragraph}"
	TocLevel              4
End


Style Subparagraph
	LabelType             Counter
	LabelCounter          subparagraph
	LabelString           "@Paragraph@.\arabic{subparagraph}"
	LabelStringAppendix   "@Paragraph@.\arabic{subparagraph}"
	TocLevel              5
End
