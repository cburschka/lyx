# Author : Lars Gullik Bjønnes <larsbj@lyx.org>

# This include file contains all the counters that are defined as standard
# in most LyX layouts.


Format 9
Counter 
	Name                  part
	LabelString	      "\Roman{part}"
End

Counter
	Name                 chapter
End

Counter
	Name                 section
	Within               chapter
End

Counter
	Name                 subsection
	Within               section
End

Counter
	Name                 subsubsection
	Within               subsection
End

Counter
	Name                 paragraph
	Within               subsubsection
End

Counter
	Name                 subparagraph
	Within               paragraph
End

Counter
	Name                 enumi
End

Counter
	Name                 enumii
	Within               enumi
End

Counter
	Name                 enumiii
	Within               enumii
End

Counter
	Name                 enumiv
	Within               enumiii
End

Counter
	Name                 bibitem
End

Counter
	Name                 listing
End

Counter
	Name                 equation
End

Counter
	Name                 footnote
End
