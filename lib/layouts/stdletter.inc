# Letter textclass definition file. 
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Heavily modifed and enhanced by several developers.

# General textclass parameters
Columns			1
Sides			1
PageStyle		Empty

# This is just to show how to declare the default font.
# The defaults are exactly those shown here.
DefaultFont
  Family		Roman
  Series		Medium
  Shape			Up
  Size			Normal
  Color			None
EndFont

# Standard style definition
Style Standard
  Margin  		Static
  LatexType 		Paragraph
  LatexName 		dummy
  ParIndent		MM
  ParSkip 		0.4
  Align 		Block
  AlignPossible 	Block, Left, Right, Center
  LabelType 		No_Label
End

# My Address style
Style My_Address
  Margin		Right_Address_Box
  LatexType		Command
  LatexName		address
  ParSkip		0.4
  BottomSep		1.5
  ParSep		1.5
  Align			Left
  AlignPossible		Left
End

# Send_To_Address style definition
Style Send_To_Address
  Margin		Static
  LatexType		Command
  LatexName		letter
  KeepEmpty		1  
  ParSkip		0.4
  BottomSep		1.5
  ParSep		1.5
  Align			Left
  AlignPossible		Block, Left, Right, Center
  LabelType		No_Label
End

# Opening style definition
Style Opening
  Margin  		Static
  LatexType 		Command
  LatexName 		opening
  KeepEmpty		1  
  LeftMargin		Signature::xx
  LabelSep		xx
  TopSep		0.4
  BottomSep		0.4
  ParSep		0.4
  Align 		Block
  AlignPossible 	Block, Left
  LabelType		Static
  LabelString		Opening:

  # label font definition
  LabelFont 
    Series		Bold
    Shape		Italic
    Latex		Latex
  EndFont
End

# Signature style definition
Style Signature
  Margin  		Static
  LatexType 		Command
  LatexName 		signature
  LeftMargin		Signature::xx
  LabelSep		xx
  TopSep		0.4
  BottomSep		0.4
  ParSep		0.4
  Align 		Block
  AlignPossible 	Block, Left
  LabelType		Static
  LabelString		Signature:

  # label font definition
  LabelFont 
    Series		Bold
    Shape		Italic
    Latex		Latex
  EndFont
End

# Closing style definition
Style Closing
  Margin  		Static
  LatexType 		Command
  LatexName 		closing
  LeftMargin		Signature::xx
  LabelSep		xx
  TopSep		0.4
  BottomSep		0.4
  ParSep		0.4
  Align 		Block
  AlignPossible 	Block, Left
  LabelType		Static
  LabelString		Closing:

  # label font definition
  LabelFont 
    Series		Bold
    Shape		Italic
    Latex		Latex
  EndFont
End

# cc style definition
Style cc
  Margin  		Static
  LatexType 		Command
  LatexName 		cc
  LeftMargin		cc:xx
  LabelSep		xx
  TopSep		0.4
  BottomSep		0.4
  ParSep		0.4
  Align 		Block
  AlignPossible 	Block, Left
  LabelType		Static
  LabelString		cc:
End

# encl style definition
Style encl
  Margin  		Static
  LatexType 		Command
  LatexName 		encl
  LeftMargin		encl:xx
  LabelSep		xx
  TopSep		0.4
  BottomSep		0.4
  ParSep		0.4
  Align 		Block
  AlignPossible 	Block, Left
  LabelType		Static
  LabelString		encl:
  Spacing               Other 1.5
End

# Telephone style definition
Style Location
  Margin  		Static
  LatexType 		Command
  LatexName 		location
  LeftMargin		Location:xx
  LabelSep		xx
  TopSep		0.4
  BottomSep		0.4
  ParSep		0.4
  Align 		Block
  AlignPossible 	Block, Left
  LabelType		Static
  LabelString		Location:
  Spacing               Other 1.5
End

# Telephone style definition
Style Telephone
  Margin  		Static
  LatexType 		Command
  LatexName 		telephone
  LeftMargin		Telephone:xx
  LabelSep		xx
  TopSep		0.4
  BottomSep		0.4
  ParSep		0.4
  Align 		Block
  AlignPossible 	Block, Left
  LabelType		Static
  LabelString		Telephone:
  Spacing               Other 1.5
End

