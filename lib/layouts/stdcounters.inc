# Author : Lars Gullik Bjønnes <larsbj@lyx.org>

# This include file contains all the counters that are defined as standard
# in most LyX layouts.


Format 19
Counter part
	LabelString	      "\Roman{part}"
End

Counter chapter
End

Counter section
	Within               chapter
End

Counter subsection
	Within               section
End

Counter subsubsection
	Within               subsection
End

Counter paragraph
	Within               subsubsection
End

Counter subparagraph
	Within               paragraph
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
End

Counter footnote
End
