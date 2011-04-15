# Author : André Pönitz
# This include files contains label definitions for a article-like numbering.


Format 35
Style Section
	LabelType             Counter
	LabelCounter          section
	LabelString           "\Roman{section}."
	LabelStringAppendix   "Appendix \Alph{section}:"
	TocLevel              1
End


Style Subsection
	LabelType             Counter
	LabelCounter          subsection
	LabelString           "\Alph{subsection}."
	LabelStringAppendix   "\arabic{subsection}."
	TocLevel              2
End


Style Subsubsection
	LabelType             Counter
	LabelCounter          subsubsection
	LabelString           "\arabic{subsubsection}."
	LabelStringAppendix   "\alph{subsubsection}."
	TocLevel              3
End


Style Paragraph
	LabelType             Counter
	LabelCounter          paragraph
	LabelString           "\alph{paragraph}."
	LabelStringAppendix   "\alph{paragraph}."
	TocLevel              4
End
