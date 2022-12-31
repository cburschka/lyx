# Moved here by rikiheck. Original author cloudy.
# Edited sep '22 by Udi Fogiel <udifoglle@gmail.com> (more similar version to the amsthm one)

Format 99

# This is just a hacked version of the AMS proof environment
Style Proof
	Category              Reasoning
	Margin                First_Dynamic
	LatexType             Environment
	LatexName             proof
	NextNoIndent          1
	ResetArgs             1
	Argument 1
		LabelString       "Alternative Proof String"
		Tooltip           "Alternative proof string"
	EndArgument
	LabelSep              xx
	ParIndent             MMM
	ParSkip               0.4
	ItemSep               0.2
	TopSep                0.7
	BottomSep             0.7
	ParSep                0.3
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Static
	LabelString           "Proof."
	EndLabelType          Box
	Font
	    Shape             Up
	    Size              Normal
	EndFont
	LabelFont
	    Shape             Italic
	EndFont
	Preamble
		\ifcsname proof\endcsname\else
			\DeclareRobustCommand{\lyx@qed}{%
			  \ifmmode
			  \else \leavevmode\unskip\penalty9999 \hbox{}\nobreak\hfill
			  \fi
			  \quad\hbox{\lyx@qedsymbol}}
			\newcommand{\lyx@openbox}{\leavevmode
			  \hbox to.77778em{%
			  \hfil\vrule
			  \vbox to.675em{\hrule width.6em\vfil\hrule}%
			  \vrule\hfil}}
			\newcommand{\lyx@qedsymbol}{\lyx@openbox}
			\newenvironment{proof}[1][\proofname]{\par
			  \normalfont
			  \topsep6\p@\@plus6\p@ \trivlist
			  \item[\hskip\labelsep\itshape
				#1.]\ignorespaces
			}{%
			  \lyx@qed\endtrivlist
			}
		\fi
	EndPreamble
    # No LangPreamble or BabelPreamble, since babel knows \proofname already.
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='proof'
	DocBookTag            para
	DocBookGenerateTitle  true
End
