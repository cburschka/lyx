# Standard textclass definition file. Taken from initial LyX source code
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Transposed by Pascal André <andre@via.ecp.fr>
# Heavily modifed and enhanced by several developers.

# This file contains some special LyX environments that are used by
# all environment classes. Some of these are a bit peculiar and might
# be removed later.

# Lyx-Code style definition
Style LyX-Code
  Margin		Static
  LatexType		Environment
  LatexName		lyxcode
  NextNoIndent		1
  LeftMargin		MMM
  RightMargin		MMM
  TopSep		0.5
  BottomSep		0.5
  Align			Left
  AlignPossible		Block, Left, Right, Center
  LabelType		No_Label
  FreeSpacing 	    	1

  # define the environment lyxcode
  Preamble
  \newenvironment{lyxcode}
    {\begin{list}{}{
      \setlength{\rightmargin}{\leftmargin}
      \raggedright
      \setlength{\itemsep}{0pt}
      \setlength{\parsep}{0pt}
      \normalfont\ttfamily}%
     \item[]}
    {\end{list}}
  EndPreamble

  # standard font definition
  Font 
    Family  		Typewriter
  EndFont

End

# Comment style definition
Style Comment
  Margin		Dynamic
  LatexType		Environment
  LatexName		comment
  NextNoIndent		1
  LeftMargin		MMM
  RightMargin           MMM
  Align			Block
  AlignPossible		Block, Left, Right, Center
  LabelType		Static
  LabelSep		:x
  Labelstring		"Comment:"

  LabelFont
	Shape		Italic
	Series		Bold
	Color		black
  EndFont

  TextFont
	Color		magenta
	Shape		Italic
  EndFont

  Preamble
  \usepackage{verbatim}
  EndPreamble

End

# Address style definition
Style Address
  Margin		Static
  LatexType		Command
  LatexName		lyxaddress
  ParSkip		0.4
  BottomSep		1.5
  ParSep		1.5
  Align			Left
  AlignPossible		Block, Left, Right, Center
  LabelType		No_Label

  # define the macro \lyxaddress
  Preamble
  \newcommand{\lyxaddress}[1]{
    \par {\raggedright #1 
    \vspace{1.4em}
    \noindent\par}
  }
  EndPreamble

End

# Right Address style
Style Right_Address
  Margin		Right_Address_Box
  LatexType		Command
  LatexName		lyxrightaddress
  ParSkip		0.4
  BottomSep		1.5
  ParSep		1.5
  Align			Left
  AlignPossible		Left

  # define the macro \lyxrightaddress
  Preamble
  \newcommand{\lyxrightaddress}[1]{
    \par {\raggedleft \begin{tabular}{l}\ignorespaces
    #1
    \end{tabular}
    \vspace{1.4em}
    \par}
  }
  EndPreamble
  
End
