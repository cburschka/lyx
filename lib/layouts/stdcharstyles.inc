# Textclass definition file for LaTeX.
# Author : Martin vermeer <amrtin.vermeer@hut.fi>
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


# Error fallback:
InsetLayout CharStyle
	LyxType               end
	LabelString           "UNDEFINED"
	Font
	  Color               error
	EndFont
End

