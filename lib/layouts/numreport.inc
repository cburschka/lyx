# Author : André Pönitz
# This include file contains label definitions for a report-like numbering.

Format 4
Input numarticle.inc

Style Part
	TocLevel              -1
End


Style Chapter
	LabelType             Counter
	LabelCounter          chapter
	LabelString           "Chapter \arabic{chapter}"
	LabelStringAppendix   "Appendix \Alph{chapter}"
	TocLevel              0
End


Style Section
	LabelString           "\arabic{chapter}.\arabic{section}"
	LabelStringAppendix   "\Alph{chapter}.\arabic{section}"
End

