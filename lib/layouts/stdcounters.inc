# Author : Lars Gullik Bjønnes <larsbj@lyx.org>

# This include file contains all the counters that are defined as standard
# in most LyX layouts.


Format 89
Counter part
	GuiName Part
	LabelString          "\Roman{part}"
	PrettyFormat         "Part ##"
End

Counter chapter
	GuiName Chapter
	PrettyFormat         "Chapter ##"
End

Counter section
	GuiName Section
	Within               chapter
	PrettyFormat         "Section ##"
End

Counter subsection
	GuiName Subsection
	Within               section
	PrettyFormat         "Section ##"
End

Counter subsubsection
	GuiName Subsubsection
	Within               subsection
	PrettyFormat         "Section ##"
End

Counter paragraph
	GuiName Paragraph
	Within               subsubsection
	PrettyFormat         "Paragraph ##"
End

Counter subparagraph
	GuiName Subparagraph
	Within               paragraph
	PrettyFormat         "Paragraph ##"
End

Counter enumi
	GuiName "Numbered List (Level 1)"
	LabelString          "\arabic{enumi}."
End

Counter enumii
	GuiName "Numbered List (Level 2)"
	Within               enumi
	LabelString          "(\alph{enumii})"
End

Counter enumiii
	GuiName "Numbered List (Level 3)"
	Within               enumii
	LabelString          "\roman{enumiii}."
End

Counter enumiv
	GuiName "Numbered List (Level 4)"
	Within               enumiii
	LabelString          "\Alph{enumiv}."
End

Counter bibitem
	GuiName "Bibliography Item"
End

Counter listing
	GuiName Listing
End

Counter equation
	GuiName Equation
	PrettyFormat         "Equation ##"
End

Counter footnote
	GuiName Footnote
	PrettyFormat         "Footnote ##"
End

Counter thanks
	GuiName "Footnote (Title)"
	PrettyFormat         "Footnote ##"
	LabelString	     "\fnsymbol{thanks}"
End
