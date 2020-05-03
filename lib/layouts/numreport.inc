# Author : André Pönitz
# This include file contains label definitions for a report-like numbering.

Format 80

Counter chapter
	GuiName Chapter
	Within                ""
	LabelString	      "\arabic{chapter}"
	LabelStringAppendix   "\Alph{chapter}"
End

Counter section
	GuiName Section
	Within                chapter
	LabelString	      ""
End

Float
	Type                  table
	NumberWithin          chapter
End


Float
	Type                  figure
	NumberWithin          chapter
End


Float
	Type                  algorithm
	NumberWithin          chapter
End

Counter listing
	GuiName Listing
	Within		      chapter
End

Counter footnote
	GuiName Footnote
	Within		      chapter
	LabelString	      "\arabic{footnote}"
End
