# Standard textclass definition file. Taken from initial LyX source code
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Transposed by Pascal André <andre@via.ecp.fr>
# Heavily modifed and enhanced by several developers.

# This include files contains all numbered sectionning-related
# commands that are useful for article-like document classes, but not
# for letters.

# Part style definition
Style Part
  Margin		Dynamic
  LatexType		Command
  LatexName		part
  NeedProtect		1
  NextNoIndent          1
  Labelsep		xxx
  ParSkip               0.4
  TopSep                4
  BottomSep             4
  ParSep                0.8
  Align			Center         
  Alignpossible         Center
  LabelType             Static
  LabelString		"Part #"

  # standard font definition
  Font 
    Series              Bold
    Size                Huge
  EndFont

End
  

# Chapter style definition
Style Chapter
  Margin                Static
  LatexType             Command
  LatexName             chapter
  NeedProtect		1
  NextNoIndent          1
  ParSkip               0.4
  TopSep                4
  BottomSep             0.8
  ParSep                0.8
  Align                 Block
  AlignPossible         Block, Left
  LabelType             Counter_Chapter
  LabelString		"Chapter "
  LabelStringAppendix   "Appendix "

  # standard font definition
  Font 
    Series              Bold
    Size                Huge
  EndFont

End

# Section style definition
Style Section
  Margin		Dynamic
  LatexType		Command
  LatexName		section
  NeedProtect		1
  NextNoIndent		1
  LabelSep		xxx
  ParSkip		0.4
  TopSep		1.3
  BottomSep		0.7
  ParSep		0.7
  Align			Block
  AlignPossible		Block, Left
  LabelType		Counter_Section

  # standard font definition
  Font 
    Series 		Bold
    Size		Larger
  EndFont

End

# Subsection style definition
Style Subsection
  Margin		Dynamic
  LatexType		Command
  LatexName		subsection
  NeedProtect		1
  NextNoIndent		1
  LabelSep		xxx
  ParSkip		0.4
  TopSep		0.9
  BottomSep		0.5
  ParSep		0.5
  Align			Block
  AlignPossible		Block, Left
  LabelType		Counter_Subsection

  # standard font definition
  Font 
    Series 		Bold
    Size		Large
  EndFont

End

# Subsubsection style definition
Style Subsubsection
  Margin		Dynamic
  LatexType		Command
  LatexName		subsubsection
  NeedProtect		1
  NextNoIndent		1
  LabelSep		xxx
  ParSkip		0.4
  TopSep		0.7
  BottomSep		0.4
  ParSep		0.4
  Align			Block
  AlignPossible		Block, Left
  LabelType		Counter_SubSubSection

  # standard font definition
  Font 
    Series 		Bold
    Size		Normal
  EndFont

End

# Paragraph style definition
Style Paragraph
  Margin		Dynamic
  LatexType		Command
  LatexName		paragraph
  NeedProtect		1
  NextNoIndent		1
  LabelSep		xxx
  ParSkip		0.4
  TopSep		0.4
  BottomSep		0
  ParSep		0.4
  Align			Block
  AlignPossible		Block, Left
  LabelType		Counter_Paragraph

  # standard font definition
  Font 
    Series 		Bold
    Size		Normal
  EndFont

End

# Subparagraph style definition
Style Subparagraph
  Margin		Dynamic
  LatexType		Command
  LatexName		subparagraph
  NeedProtect		1
  NextNoIndent		0
  LeftMargin		MM
  LabelSep		xxx
  ParSkip		0.4
  TopSep		0.4
  BottomSep		0
  ParSep		0.4
  Align			Block
  AlignPossible		Block, Left
  LabelType		Counter_SubParagraph

  # standard font definition
  Font 
    Series 		Bold
    Size		Normal
  EndFont

End
