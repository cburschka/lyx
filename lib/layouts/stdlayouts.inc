# Standard textclass definition file. Taken from initial LyX source code
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Transposed by Pascal André <andre@via.ecp.fr>
# Heavily modifed and enhanced by several developers.

# This include files contains various standard environments for lists,
# quotations and such.

# LaTeX style definition
Style LaTeX
  Margin		Static
  LatexType		Paragraph
  LatexName		dummy
  NewLine		0
  ParIndent		MM
  ParSkip		0.4
  ParSep		0.5
  Align			Block
  AlignPossible		Block, Left, Right, Center
  LabelType		No_Label
  FreeSpacing 	    	1

  # standard font definition
  Font 
    Latex		Latex
  EndFont

End

# Quotation style definition
Style Quotation
  Margin		Static
  LatexType		Environment
  LatexName		quotation
  NextNoIndent		1
  LeftMargin		MMM
  RightMargin		MMM
  ParIndent		MM
  ParSkip		0
  ItemSep		0
  TopSep		0.5
  BottomSep		0.5
  ParSep		0
  Align			Block
  AlignPossible		Block, Left, Right, Center
End

# Quote style definition
Style Quote
  Margin		Static
  LatexType		Environment
  LatexName		quote
  NextNoIndent		1
  LeftMargin		MMM
  RightMargin		MMM
  TopSep		0.5
  ParSep		0.5
  BottomSep		0.5
  Align			Block
  AlignPossible		Block, Left, Right, Center
  LabelType		No_Label
End

# Verse style definition
Style Verse
# LaTeX type is first dynamic because of LaTeX implementation.
# In LyX this does not make sense.
  LatexType		Environment
  LatexName		verse
  Margin		First_Dynamic
  NextNoIndent		1
  LeftMargin		MMMM
  LabelIndent		MMMM
  ParSkip		0.4
  TopSep		0.7
  BottomSep		0.7
  ParSep		0.5
  Align			Block
  AlignPossible		Block, Left
  LabelType		No_Label
End

# Caption style definition
Style Caption
  Margin		First_Dynamic
  LatexType		Command
  LatexName		caption
  NeedProtect		1
  LabelSep		xx
  ParSkip		0.4
  TopSep		0.5
  Align			Center
  AlignPossible		Center
  LabelType		Sensitive
  LabelString		Caption

  # label font definition
  LabelFont 
    Series		Bold
  EndFont
End
