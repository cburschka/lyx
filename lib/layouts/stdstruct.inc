# Standard textclass definition file. Taken from initial LyX source code
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Transposed by Pascal André <andre@via.ecp.fr>
# Heavily modifed and enhanced by several developers.

# This include files contains all layouts that handle the structure of
# a document, like titles and such.

# Title style definition
Style Title
  Margin		Static
  LatexType		Command
  InTitle		1
  LatexName		title
  ParSkip		0.4
  ItemSep		0
  TopSep		0
  BottomSep		1
  ParSep		1
  Align			Center
  AlignPossible		Center
  LabelType		No_Label

  # standard font definition
  Font 
    Size		Largest
  EndFont

End

# Author style definition
Style Author
  Margin		Static
  LatexType		Command
  InTitle		1
  LatexName		author
  LabelSep		xxx
  ParSkip		0.4
  TopSep		1.3
  BottomSep		0.7
  ParSep		0.7
  Align			Center
  AlignPossible		Center
  LabelType		No_Label

  # standard font definition
  Font 
    Size		Large
  EndFont

End

# Date style definition
Style Date
  Margin		Dynamic
  LatexType		Command
  InTitle		1
  LatexName		date
  LabelSep		xxx
  ParSkip		0.4
  TopSep		0.9
  BottomSep		0.5
  ParSep		1
  Align			Center
  AlignPossible		Center
  LabelType		No_Label


  # standard font definition
  Font 
    Size		Large
  EndFont

End

# Abstract style definition
Style Abstract 
  Margin		Static
  LatexType		Environment
  LatexName		abstract
  NextNoIndent		1
  LeftMargin		MMM
  RightMargin		MMM
  ParIndent		MM
  ItemSep		0
  TopSep		0.7
  BottomSep		0.7
  ParSep		0
  Align			Block
  AlignPossible		Block
  LabelType		Centered_Top_Environment
  LabelString		Abstract
  LabelBottomSep    0.5

  # standard font definition
  Font 
    Size		Small
  EndFont

  # label font definition
  LabelFont 
    Series		Bold
    Size		Large
  EndFont
End 

# Bibliography style definition
Style Bibliography
  Margin		First_Dynamic
  LatexType		Item_Environment
  LatexName		thebibliography
  NextNoIndent		1
  LeftMargin		MM
  ParSkip		0.4
  ItemSep		0
  TopSep		0.7
  BottomSep		0.7
  ParSep		0.5
  Align			Block
  AlignPossible		Block, Left
  LabelType		Bibliography
  LabelString		References
  LabelBottomSep    0.7

  # label font definition
  LabelFont 
    Series		Bold
    Size		Larger
  EndFont
End
