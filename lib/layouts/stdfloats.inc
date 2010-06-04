# Author : Lars Gullik Bjønnes <larsbj@lyx.org>

# This include file contains all the floats that are defined as standard
# in most LyX layouts.


Format 27

Float
	Type                  table
	GuiName               Table
	Placement             tbp
	Extension             lot
	NumberWithin          none
	Style                 plain
	ListName              "List of Tables"
	NeedsFloatPkg         false
	ListCommand           listoftables
	RefPrefix             tab
End


Float
	Type                  figure
	GuiName               Figure
	Placement             tbp
	Extension             lof
	NumberWithin          none
	Style                 plain
	ListName              "List of Figures"
	NeedsFloatPkg         false
	ListCommand           listoffigures
	RefPrefix             fig
End


Float
	Type                  algorithm
	GuiName               Algorithm
	Placement             tbp
	Extension             loa
	NumberWithin          none
	Style                 ruled
	ListName              "List of Algorithms"
	NeedsFloatPkg         true
	RefPrefix             alg
End
