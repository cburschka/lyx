# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>

# This file is the counterpart of stdclass.inc
# It is desirable, as far as possible, to have the same look and feel for
# related layouts in latex and docbook.

OutputType              docbook

Columns                 1
Sides                   1
SecNumDepth             3
TocDepth                3
DefaultStyle            Standard

ClassOptions
	Header             "<!DOCTYPE #  PUBLIC &quot;-//OASIS//DTD DocBook V4.1//EN&quot;"
End

Style Standard
	Margin                Static
	LatexType             Paragraph
	LatexName             para
	ParIndent             MM
	ParSkip               0.4
	Align                 Block
	AlignPossible         Block
	LabelType             No_Label
End


CharStyle Filename
	LatexType             Command
	LatexName             filename
	Font
	  Family              Typewriter
	EndFont
	LabelFont
	  Family              Roman
	  Color               blue
	EndFont
End

CharStyle Firstname
	LatexType             Command
	LatexName             firstname
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
End

CharStyle Surname
	LatexType             Command
	LatexName             surname
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
End

CharStyle Literal
	LatexType             Command
	LatexName             literal
	Font
	  Family              Typewriter
	EndFont
	LabelFont
	  Family              Roman
	  Color               blue
	EndFont
End


Input db_stdlists.inc
Input db_stdsections.inc
Input db_stdstarsections.inc
Input db_stdtitle.inc
Input db_stdstruct.inc
Input db_lyxmacros.inc
Input db_stdlayouts.inc
Input stdcounters.inc
Input db_stdcounters.inc
Input stdfloats.inc
