# Author : Lars Gullik Bjønnes <larsbj@lyx.org>

# This include file contains all the counters that are defined as standard
# in most LyX layouts.


Format 35
Counter part
	LabelString          "\Roman{part}"
	PrettyFormat         "Part \Roman{part}"
End

Counter chapter
	PrettyFormat         "Chapter ##"
End

Counter section
	Within               chapter
	PrettyFormat         "Section ##"
End

Counter subsection
	Within               section
	PrettyFormat         "Section ##"
End

Counter subsubsection
	Within               subsection
	PrettyFormat         "Section ##"
End

Counter paragraph
	Within               subsubsection
	PrettyFormat         "Paragraph ##"
End

Counter subparagraph
	Within               paragraph
	PrettyFormat         "Paragraph ##"
End

Counter enumi
	LabelString          "\arabic{enumi}."
End

Counter enumii
	Within               enumi
	LabelString          "(\alph{enumii})"
End

Counter enumiii
	Within               enumii
	LabelString          "\roman{enumiii}."
End

Counter enumiv
	Within               enumiii
	LabelString          "\Alph{enumiv}."
End

Counter bibitem
End

Counter listing
End

Counter equation
	PrettyFormat         "Equation ##"
End

Counter footnote
	PrettyFormat         "Footnote ##"
End
