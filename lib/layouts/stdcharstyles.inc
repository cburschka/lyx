# Textclass definition file for LaTeX.
# Author : Martin vermeer <martin.vermeer@hut.fi>
# Character Styles definition

Format 5
InsetLayout CharStyle:Noun
	LyxType               charstyle
	LabelString           noun
	LatexType             command
	LatexName             noun
	Font
	  Shape               Smallcaps
	EndFont
	Preamble
	\newcommand{\noun}[1]{\textsc{#1}}
	EndPreamble
End


InsetLayout CharStyle:Emph
	LyxType               charstyle
	LabelString           emph
	LatexType             command
	LatexName             emph
	Font
	  Shape               Italic
	EndFont
End


InsetLayout CharStyle:Strong
	LyxType               charstyle
	LabelString           strong
	LatexType             command
	LatexName             strong
	Font
	  Series              Bold
	EndFont
	Preamble
	\newcommand{\strong}[1]{\textbf{#1}}
	EndPreamble
End


InsetLayout CharStyle:Code
	LyxType               charstyle
	LabelString           code
	LatexType             command
	LatexName             code
	Font
	  Family              Typewriter
	EndFont
	Preamble
	\newcommand{\code}[1]{\texttt{#1}}
	EndPreamble
End


# Error fallback:
InsetLayout CharStyle
	LyxType               end
	LabelString           "UNDEFINED"
	Font
	  Color               error
	EndFont
End

