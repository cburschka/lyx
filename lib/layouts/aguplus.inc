# AGUplus textclass definition file. 
# Author: Martin Vermeer <martin.vermeer@hut.fi>
#
# Necessary LaTeX files to be found on
#
# ftp://ftp.agu.org/journals/latex/journals
#
# Input general definitions
Input stdclass.inc

SecNumDepth	4
MaxCounter	Counter_Section

# There are no chapters in an article.
NoStyle Chapter
NoStyle Chapter*

NoStyle Subparagraph
NoStyle Subparagraph*

# Redefine these a bit:

Style Section
  Font
    Size	Large
  EndFont
End

Style Section*
  Font
    Size	Large
  EndFont
End

Style Subsection
  Font
    Size	Normal
  EndFont
End

Style Subsection*
  Font
    Size	Normal
  EndFont
End

# Subsubsubsection style (re-)definition
Style Paragraph
  LatexName     subsubsection
End

# SubSubsubsection* style (re-)definition
Style Paragraph*
  CopyStyle     Paragraph
  Margin        Static
  LatexName     subsubsubsection*
  LabelType     No_Label
  OptionalArgs      0
End

Preamble
\usepackage{times}
\sectionnumbers
EndPreamble

# Headers style definition
Style Left_Header
  Margin        Static
  LatexType     Command
  LatexName     lefthead
  NextNoIndent	1
  LeftMargin	"Left Header:xx"
  LabelSep      xxx
  ParSkip       0.4
  TopSep        1.3
  BottomSep     0.7
  ParSep        0.7
  Align         Left
  AlignPossible     Left
  LabelType     Static
  LabelString	"Left Header:"
  InTitle		1

  # standard font definition
  LabelFont
    Series      Bold
    Shape       Italic
    Color       red
  EndFont

End

Style Right_Header
  CopyStyle		"Left Header"
  LatexName     righthead
  LeftMargin	"Right Header:xx"
  LabelString	"Right Header:"
End

Style Received
  CopyStyle		"Left Header"
  LatexName     received
  LeftMargin	Received:xx
  LabelString	Recieved:
End

Style Revised
  CopyStyle		"Left Header"
  LatexName     revised
  LeftMargin	Revised:xx
  LabelString	Revised:
End

Style Accepted
  CopyStyle		"Left Header"
  LatexName     accepted
  LeftMargin	Accepted:xx
  LabelString	Accepted:
End

Style CCC
  CopyStyle		"Left Header"
  LatexName     ccc
  LeftMargin	"CCC code:xx"
  LabelString	"CCC code:"
End

# cpright

# journalid

# articleid

Style PaperId
  CopyStyle		"Left Header"
  LatexName     paperid
  LeftMargin	"Paper Id:xx"
  LabelString	"Paper Id:"
End

Style AuthorAddr
  CopyStyle		"Left Header"
  LatexName     authoraddr
  LeftMargin	"Author Address:xx"
  LabelString	"Author Address:"
End

Style SlugComment
  CopyStyle		"Left Header"
  LatexName     slugcomment
  LeftMargin	"Slug Comment:xx"
  LabelString	"Slug Comment:"
End


# a few changes to the bibliography
Style Bibliography
  TopSep		4
  LabelString		Bibliography

  # label font definition
  LabelFont 
    Series		Bold
    Size		Huge
  EndFont
End

# plate
Float
  Type			plate
  GuiName		Plate
  Placement		htbp
  Extension		lof
  NumberWithin  none
  Style			plain
  LatexBuiltin  true
End

# planotable
Float
  GuiName		Planotable
  Type			planotable
  Placement		htbp
  Extension		lot
  NumberWithin  none
  Style			Ruled
  LatexBuiltin  true
End


# Table Caption style definition
Style Table_Caption
  Margin		First_Dynamic
  LatexType		Command
  LatexName	tablecaption
  NeedProtect	1
  LabelSep		xx
  ParSkip		0.4
  TopSep		0.5
  Align         Center
  AlignPossible Center
  LabelType     Sensitive
  LabelString   TableCaption

  # label font definition
  LabelFont
    Series		Bold
  EndFont
End

Counter
  Name		plate
  Within	none
End

Counter
  Name		planotable
  Within	none
End
