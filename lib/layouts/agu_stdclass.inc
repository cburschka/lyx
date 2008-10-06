# Textclass definition file for AGU-Article.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>
#          Martin Vermeer <martin.vermeer@hut.fi>

# This file is the counterpart of stdclass.inc
# It is desirable, as far as possible, to have the same look and feel for
# related layouts in latex and docbook.

Format 10
OutputType              docbook

Columns                 1
Sides                   1
SecNumDepth             3
TocDepth                3
DefaultStyle            Standard

ClassOptions
	Header              "PUBLIC &quot;-//AGU//DTD article American Geophysical Union DTD version 3.42//EN&quot; &quot;AGU-Article-3.42.dtd&quot; "
End

Style Standard
	Margin                Static
	LatexType             Paragraph
	LatexName             p
	ParIndent             MM
	ParSkip               0.4
	Align                 Block
	LabelType             No_Label
	LatexParam            "id=<para#>"
End


InsetLayout Element:Firstname
	LyXType element
	LabelString Firstname
	LatexType             Command
	LatexName             firstname
	Font
	  Shape               SmallCaps
	EndFont
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
End

InsetLayout Element:Fname
	LyXType element
	LabelString Fname
	LatexType             Command
	LatexName             fname
	Font
	  Shape               SmallCaps
	EndFont
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
End


InsetLayout Element:Surname
	LyXType element
	LabelString Surname
	LatexType             Command
	LatexName             surname
	Font
	  Shape               SmallCaps
	EndFont
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
End


InsetLayout Element:Filename
	LyXType element
	LabelString Filename
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


InsetLayout Element:Literal
	LyXType element
	LabelString Literal
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


InsetLayout Element:Emph
	LyXType element
	LabelString Emph
	LatexType             Command
	LatexName             emph
	LatexParam            "type=<italic>"
	Font
	  Shape               Italic
	EndFont
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
End


InsetLayout Element:Abbrev
	LyXType element
	LabelString Abbrev
	LatexType             Command
	LatexName             abbrev
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Element:Citation-number
	LyXType element
	LabelString Citation-number
	LatexType             Command
	LatexName             citation-number
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Element:Volume
	LyXType element
	LabelString Volume
	LatexType             Command
	LatexName             volume
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Element:Day
	LyXType element
	LabelString Day
	LatexType             Command
	LatexName             day
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Element:Month
	LyXType element
	LabelString Month
	LatexType             Command
	LatexName             month
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Element:Year
	LyXType element
	LabelString Year
	LatexType             Command
	LatexName             year
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Element:Issue-number
	LyXType element
	LabelString Issue-number
	LatexType             Command
	LatexName             issue-number
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Element:Issue-day
	LyXType element
	LabelString Issue-day
	LatexType             Command
	LatexName             issue-day
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Element:Issue-months
	LyXType element
	LabelString Issue-months
	LatexType             Command
	LatexName             issue-months
	LabelFont
	  Color               blue
	EndFont
End


Input stdinsets.inc
Input agu_stdlists.inc
Input agu_stdsections.inc
Input db_stdstarsections.inc
Input agu_stdtitle.inc
Input db_stdstruct.inc
Input db_lyxmacros.inc
Input db_stdlayouts.inc
Input stdcounters.inc
Input agu_stdcounters.inc
Input stdfloats.inc
