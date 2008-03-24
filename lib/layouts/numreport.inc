# Author : André Pönitz
# This include file contains label definitions for a report-like numbering.

Format 7

Style Part
	TocLevel              -1
End

Counter
	Name                  chapter
	Within                ""
	LabelString	      "\arabic{chapter}"
	LabelStringAppendix   "\Alph{chapter}"
End

Counter
	Name                  section
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

Counter
	Name                  listing
	Within		      chapter
End

Counter
	Name                  footnote
	Within		      chapter
	LabelString	      "\arabic{footnote}"
End
