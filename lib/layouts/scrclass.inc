# KOMA script textclass definition file.
# Base definitions for the KOMA script layouts.
# Bernd Rellermeyer <100.41728@germanynet.de>, 1998/7/23.

# General textclass parameters
SecNumDepth             2
TocDepth                2

# Standard style definition
Style Standard
  LatexName             dummy
  ParIndent             MM
  ParSkip               0.4
  AlignPossible         Block, Left, Right, Center
End

Input stdlists.inc

# Modify Description
Style Description
  LabelFont
    Family              Sans
  EndFont
End

# Delete List
NoStyle List

# Labeling style definition
Style Labeling
  Margin		Manual
  LatexType		List_Environment
  LatexName             labeling
  NextNoindent		1
  LabelSep		xxx
  ParSkip		0.4
  TopSep		0.7
  BottomSep		0.7
  ParSep		0.5
  Align			Block
  AlignPossible		Block, Left
  LabelType		Manual
  LabelString		00.00.0000
End

Input stdsections.inc

# Modify Part
Style Part
  Font
    Family              Sans
  EndFont
End

# Modify Chapter
Style Chapter
  LabelString           ""
  Font
    Family              Sans
    Size                Largest
  EndFont
End

# Modify Section
Style Section
  Font
    Family              Sans
  EndFont
End

# Modify Subsection
Style Subsection
  Font
    Family              Sans
  EndFont
End

# Modify Subsubsection
Style Subsubsection
  Font
    Family              Sans
  EndFont
End

# Modify Paragraph
Style Paragraph
  Font
    Family              Sans
  EndFont
End

# Modify Subparagraph
Style Subparagraph
  LeftMargin            ""
  Font
    Family              Sans
  EndFont
End

Input stdstarsections.inc

# Addchap Style Definition
Style Addchap
  CopyStyle             Chapter*
  LatexName             addchap
End

# Addsec Style Definition
Style Addsec
  CopyStyle             Section*
  LatexName             addsec
End

# Addchap* Style Definition
Style Addchap*
  CopyStyle             Chapter*
  LatexName             addchap*
End

# Addsec* Style Definition
Style Addsec*
  CopyStyle             Section*
  LatexName             addsec*
End

# Minisec style definition
Style Minisec
  LatexType             Command
  LatexName             minisec
  NeedProtect           1
  NextNoIndent          1
  LabelSep              xxx
  ParSkip               0.4
  TopSep                0.4
  ParSep                0.4
  AlignPossible         Block, Left

  Font 
    Family              Sans
    Series              Bold
  EndFont
End

Input stdtitle.inc

# Modify Title
Style Title
  Font
    Family              Sans
    Series              Bold
  EndFont
End

# Modify Author
Style Author
  Font
    Size                Larger
  EndFont
End

# Modify Date
Style Date
  Font
    Size                Larger
  EndFont
End

# Subject style definition
Style Subject
  CopyStyle             Author
  LatexName             subject
End

# Publishers style definition
Style Publishers
  CopyStyle             Author
  LatexName             publishers
End

# Dedication style definition
Style Dedication
  CopyStyle             Author
  LatexName             dedication
End

# Titlehead style definition
Style Titlehead
  CopyStyle             Author
  LatexName             titlehead
  Align                 Left
  AlignPossible         Left

  Font 
    Size                Normal
  EndFont
End

# Uppertitleback style definition
Style Uppertitleback
  CopyStyle             Titlehead
  LatexName             uppertitleback
End

# Lowertitleback style definition
Style Lowertitleback
  CopyStyle             Titlehead
  LatexName             lowertitleback
End

# Extratitle style definition
Style Extratitle
  CopyStyle             Titlehead
  LatexName             extratitle
End

Input stdstruct.inc

# Modify Abstract
Style Abstract
  LabelFont
    Family              Sans
  EndFont
End

# Modify Bibliography
Style Bibliography
  LabelFont
    Family              Sans
  EndFont
End

Input stdlayouts.inc
Input lyxmacros.inc
Input scrmacros.inc
