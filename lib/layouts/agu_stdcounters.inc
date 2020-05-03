# Author : José Matos <jamatos@lyx.org>
#          Martin Vermeer <martin.vermeer@hut.fi>

# This include file contains all the counters that are defined as standard
# in AGU-Article LyX layouts.

Format 80
Counter sect1
	GuiName "Section Level 1"
	Within               section
End

Counter sect2
	GuiName "Section Level 2"
	Within               sect1
End

Counter sect3
	GuiName "Section Level 3"
	Within               sect2
End

Counter sect4
	GuiName "Section Level 4"
	Within               sect3
End

Counter sect5
	GuiName "Section Level 5"
	Within               sect4
End

Counter para
	GuiName Paragraph
End
