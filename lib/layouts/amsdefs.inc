# Author : David L. Johnson <dlj0@lehigh.edu>
# Probably broken by Jean-Marc Lasgouttes <Jean-Marc.Lasgouttes@inria.fr>
# modified and modularized by Emmanuel GUREGHIAN <gureghia@boston.bertin.fr>

# Including the maths stuff
Input amsmaths.inc

# The AMS documentclasses use the package amsmath and provide the
# functionality of makeidx.sty.
ProvidesAmsmath		1
ProvidesMakeidx		1


# the environnements copied from the old amsart.layout are :
# - Proof, 
# - Bibliography
# - Title
# - Author
# - Date
# - Abstract (This is a title component for this style.)
# - Address
# - Email
# - Keywords
# - Subjectclass

# Proof style declaration
Style Proof
  Margin		First_Dynamic
  LatexType		Environment
  LatexName		proof
  NextNoIndent		1
  LabelSep		xx
  ParIndent             MMM  
  ParSkip		0.4
  ItemSep		0.2
  TopSep		0.7
  BottomSep		0.7
  ParSep		0.3
  Align			Block
  AlignPossible		Block, Left
  LabelType		Static
  LabelString		"Proof."
  
  # standard font definition
  Font
    Shape               Up
    Size                Normal
  EndFont

  # label font definition
  LabelFont
    Shape               Italic
  EndFont

  # We don't want the preamble from Theorem
  Preamble
  EndPreamble 
End  

### Now the title stuff. We copy do not use stdstruct.inc to keep
### things in the right order

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
    Shape		Smallcaps
    Series 		Bold
    Size		Larger
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
    Shape		Smallcaps
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
  LabelType		Static
  LabelString		"Date: "

  # standard font definition
  Font 
    Size		Large
  EndFont

  LabelFont
    Shape		Italic
    Size		Large
  EndFont
End

# Abstract style definition
Style Abstract 
  Margin		First_Dynamic
  LatexType		Environment
  InTitle		1
  LatexName		abstract
  LeftMargin		MMM
  LabelIndent		MMM
  RightMargin		MMM
  ParIndent		MM
  TopSep		0.7
  BottomSep		0.7
  Align			Block
  AlignPossible		Block
  LabelType		Static
  LabelString		Abstract.
  LabelSep		M

  # standard font definition
  Font 
    Size		Small
  EndFont

  # label font definition
  LabelFont
    Shape		Smallcaps
  EndFont
End 

# Address style definition
Style Address
  Margin		Dynamic
  LatexType		Command
  InTitle		1
  LatexName		address
  ParSkip		0.4
  BottomSep		1.5
  ParSep		1.5
  Align			Left
  AlignPossible		Block, Left, Right, Center
  LabelType		Static
  LabelString		"Address: "
  # label font definition
  LabelFont
    Shape		Italic
  EndFont
End  

# Current Address style definition
Style Current_Address
  Margin		Dynamic
  LatexType		Command
  InTitle		1
  LatexName		curraddr
  ParSkip		0.4
  BottomSep		1.5
  ParSep		1.5
  Align			Left
  AlignPossible		Block, Left, Right, Center
  LabelType		Static
  LabelString		"Current address: "
  # label font definition
  LabelFont
    Shape		Italic
  EndFont
End  

# Email style definition
Style Email
  Margin		First_Dynamic
  LatexType		Command
  InTitle		1
  LatexName		email
  ParSkip		0.4
  BottomSep		1.5
  ParSep		1.5
  Align			Left
  AlignPossible		Block, Left, Right, Center
  LabelType		Static
  labelString		"E-mail address: "
  LabelFont
    Shape		Italic
  EndFont
End

# URL style definition
Style URL
  Margin		First_Dynamic
  LatexType		Command
  InTitle		1
  LatexName		urladdr
  ParSkip		0.4
  BottomSep		1.5
  ParSep		1.5
  Align			Left
  AlignPossible		Block, Left, Right, Center
  LabelType		Static
  labelString		"URL: "
  LabelFont
    Shape		Italic
  EndFont
End

# Keywords style definition
Style Keywords
  Margin		Dynamic
  LatexType		Command
  InTitle		1
  LatexName		keywords
  ParSkip		0.4
  BottomSep		1.5
  ParSep		1.5
  Align			Left
  AlignPossible		Block, Left, Right, Center
  LabelType		Static
  LabelString		"Key words and phrases: "
  # label font definition
  LabelFont
    Shape		Italic
  EndFont
End  

# Thanks style definition
Style Thanks
  Margin		Dynamic
  LatexType		Command
  InTitle		1
  LatexName		thanks
  ParSkip		0.4
  BottomSep		1.5
  ParSep		1.5
  Align			Left
  AlignPossible		Block, Left, Right, Center
  LabelType		Static
  LabelString		"Thanks: "
  # label font definition
  LabelFont
    Shape		Italic
  EndFont
End  

# Dedicatory style definition
Style Dedicatory
  Margin		Dynamic
  LatexType		Command
  InTitle		1
  LatexName		dedicatory
  ParSkip		0.4
  BottomSep		1.5
  ParSep		1.5
  Align			Left
  AlignPossible		Block, Left, Right, Center
  LabelType		Static
  LabelString		"Dedication: "
  # label font definition
  LabelFont
    Shape		Italic
  EndFont
End  


# Translator style definition
Style Translator
  Margin		Dynamic
  LatexType		Command
  InTitle		1
  LatexName		translator
  ParSkip		0.4
  BottomSep		1.5
  ParSep		1.5
  Align			Left
  AlignPossible		Block, Left, Right, Center
  LabelType		Static
  LabelString		"Translator: "
  # label font definition
  LabelFont
    Shape		Italic
  EndFont
End  

# Subjectclass style definition
Style Subjectclass
  Margin		Dynamic
  LatexType		Command
  InTitle		1
  LatexName		subjclass
  ParSkip		0.4
  BottomSep		1.5
  ParSep		1.5
  Align			Left
  AlignPossible		Block, Left, Right, Center
  LabelType		Static
  LabelString		"1991 Mathematics Subject Classification: "
  # label font definition
  LabelFont
    Shape		Italic
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

  # standard font definition
  Font
    Size		Small
  EndFont

  # label font definition
  LabelFont
    Size		Normal
    Shape		Smallcaps
  EndFont
End

