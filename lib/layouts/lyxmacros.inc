# Standard textclass definition file. Taken from initial LyX source code
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Transposed by Pascal André <andre@via.ecp.fr>
# Heavily modifed and enhanced by several developers.

# This file contains some special LyX environments that are used by
# all environment classes. Some of these are a bit peculiar and might
# be removed later.


Format 35
Style LyX-Code
	Category              MainText
	Margin                Static
	LatexType             Environment
	LatexName             lyxcode
	NextNoIndent          1
	LeftMargin            MMM
	RightMargin           MMM
	TopSep                0.5
	BottomSep             0.5
	Align                 Left
	AlignPossible         Block, Left, Right, Center
	LabelType             No_Label
	FreeSpacing           1
	Preamble
		\newenvironment{lyxcode}
			{\par\begin{list}{}{
				\setlength{\rightmargin}{\leftmargin}
				\setlength{\listparindent}{0pt}% needed for AMS classes
				\raggedright
				\setlength{\itemsep}{0pt}
				\setlength{\parsep}{0pt}
				\normalfont\ttfamily}%
			 \item[]}
			{\end{list}}
	EndPreamble
	Font
	  Family              Typewriter
	EndFont
End


Style Address
	Category	      FrontMatter
	Margin                Static
	LatexType             Command
	LatexName             lyxaddress
	ParSkip               0.4
	BottomSep             1.5
	ParSep                1.5
	Align                 Left
	AlignPossible         Block, Left, Right, Center
	LabelType             No_Label
	Preamble
		\newcommand{\lyxaddress}[1]{
			\par {\raggedright #1
			\vspace{1.4em}
			\noindent\par}
		}
	EndPreamble
End


Style Right_Address
	Category	      FrontMatter
	Margin                Right_Address_Box
	LatexType             Command
	LatexName             lyxrightaddress
	ParSkip               0.4
	BottomSep             1.5
	ParSep                1.5
	Align                 Left
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
