# Author : André Pönitz
# This include file contains label definitions for a report-like numbering.

Format 59

Counter chapter
	Within                ""
	LabelString	      "\arabic{chapter}"
	LabelStringAppendix   "\Alph{chapter}"
End

Counter section
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
	Within		      chapter
End

Counter footnote
	Within		      chapter
	LabelString	      "\arabic{footnote}"
End
