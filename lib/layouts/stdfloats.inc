# Author : Lars Gullik Bjønnes <larsbj@lyx.org>

# This include file contains all the floats that are defined as standard
# in most LyX layouts.


Float
  Type                  table
  GuiName		Table
  Placement             tbp
  Extension             lot
  NumberWithin          none
  Style                 plain
  ListName              "List of Tables"
  LaTeXBuiltin          true
End
  
Float
  Type                  figure
  GuiName               Figure
  Placement             tbp
  Extension             lof
  NumberWithin          none
  Style                 plain
  ListName              "List of Figures"
  LaTeXBuiltin          true
End

Float
  Type                  algorithm
  GuiName               Algorithm
  Placement             tbp
  Extension             loa
  NumberWithin          none
  Style                 ruled
  ListName              "List of Algorithms"
  LaTeXBuiltin          false
End
