# Textclass definition file for AGU-Article.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>
#          Martin Vermeer <martin.vermeer@hut.fi>

# This file is the counterpart of stdclass.inc
# It is desirable, as far as possible, to have the same look and feel for
# related layouts in latex and docbook.

OutputType              docbook

Columns                 1
Sides                   1
SecNumDepth             3
TocDepth                3
DefaultStyle            Standard


Style Standard
	Margin                Static
	LatexType             Paragraph
	LatexName             p
	ParIndent             MM
	ParSkip               0.4
	Align                 Block
	AlignPossible         Block
	LabelType             No_Label
	LatexParam            "para#"
End


CharStyle Firstname
	LatexType             Command
	LatexName             firstname
	Font
	  Shape               SmallCaps
	EndFont
	LabelFont
	  Shape               Italic
	  Color               red
	EndFont
End

CharStyle Surname
	LatexType             Command
	LatexName             firstname
	Font
	  Shape               SmallCaps
	EndFont
	LabelFont
	  Shape               Italic
	  Color               red
	EndFont
End

CharStyle Filename
	LatexType             Command
	LatexName             firstname
	Font
	  Family              Typewriter
	EndFont
	LabelFont
	  Shape               Italic
	  Color               red
	EndFont
End

CharStyle Literal
	LatexType             Command
	LatexName             firstname
	Font
	  Family              Typewriter
	EndFont
	LabelFont
	  Shape               Italic
	  Color               red
	EndFont
End


Input agu_stdlists.inc
Input agu_stdsections.inc
Input db_stdstarsections.inc
Input db_stdtitle.inc
Input db_stdstruct.inc
Input db_lyxmacros.inc
Input db_stdlayouts.inc
Input stdcounters.inc
Input agu_stdcounters.inc
Input stdfloats.inc
