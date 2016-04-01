# Author : André Pönitz
# This include files contains label definitions for a article-like numbering.


Format 60
Style Section
	LabelType             Static
	LabelCounter          section
	LabelString           "\Roman{section}."
	LabelStringAppendix   "Appendix \Alph{section}:"
	TocLevel              1
End


Style Subsection
	LabelType             Static
	LabelCounter          subsection
	LabelString           "\Alph{subsection}."
	LabelStringAppendix   "\arabic{subsection}."
	TocLevel              2
End


Style Subsubsection
	LabelType             Static
	LabelCounter          subsubsection
	LabelString           "\arabic{subsubsection}."
	LabelStringAppendix   "\alph{subsubsection}."
	TocLevel              3
End


Style Paragraph
	LabelType             Static
	LabelCounter          paragraph
	LabelString           "\alph{paragraph}."
	LabelStringAppendix   "\alph{paragraph}."
	TocLevel              4
End
