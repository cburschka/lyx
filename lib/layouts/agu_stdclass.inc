# Textclass definition file for AGU-Article.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>
#          Martin Vermeer <martin.vermeer@hut.fi>

# This file is the counterpart of stdclass.inc
# It is desirable, as far as possible, to have the same look and feel for
# related layouts in latex and docbook.

Format 82
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
	Category              MainText
	Margin                Static
	LatexType             Paragraph
	LatexName             p
	ParIndent             MM
	ParSkip               0.4
	Align                 Block
	LabelType             No_Label
	LatexParam            "id=<para#>"
End


InsetLayout Flex:Firstname
	LabelString Firstname
	LatexType             Command
	DocBookTag            firstname
	Font
	  Shape               SmallCaps
	EndFont
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
	ResetsFont true
End

InsetLayout Flex:Fname
	LabelString Fname
	LatexType             Command
	DocBookTag            fname
	Font
	  Shape               SmallCaps
	EndFont
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
	ResetsFont true
End


InsetLayout Flex:Surname
	LabelString Surname
	LatexType             Command
	DocBookTag            surname
	Font
	  Shape               SmallCaps
	EndFont
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
	ResetsFont true
End


InsetLayout Flex:Filename
	LabelString Filename
	LatexType             Command
	DocBookTag            filename
	Font
	  Family              Typewriter
	EndFont
	LabelFont
	  Family              Roman
	  Color               blue
	EndFont
	ResetsFont true
End


InsetLayout Flex:Literal
	LabelString Literal
	LatexType             Command
	DocBookTag            literal
	Font
	  Family              Typewriter
	EndFont
	LabelFont
	  Family              Roman
	  Color               blue
	EndFont
	ResetsFont true
End


InsetLayout Flex:Emph
	LabelString Emph
	LatexType             Command
	DocBookTag            emph
	DocBookAttr           "type='italic'"
	Font
	  Shape               Italic
	EndFont
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
	ResetsFont true
End


InsetLayout Flex:Abbrev
	LabelString Abbrev
	LatexType             Command
	DocBookTag            abbrev
	LabelFont
	  Color               blue
	EndFont
	ResetsFont true
End


InsetLayout Flex:Citation-number
	LabelString Citation-number
	LatexType             Command
	DocBookTag            citation-number
	LabelFont
	  Color               blue
	EndFont
	ResetsFont true
End


InsetLayout Flex:Volume
	LabelString Volume
	LatexType             Command
	DocBookTag            volume
	LabelFont
	  Color               blue
	EndFont
	ResetsFont true
End


InsetLayout Flex:Day
	LabelString Day
	LatexType             Command
	DocBookTag            day
	LabelFont
	  Color               blue
	EndFont
	ResetsFont true
End


InsetLayout Flex:Month
	LabelString Month
	LatexType             Command
	DocBookTag            month
	LabelFont
	  Color               blue
	EndFont
	ResetsFont true
End


InsetLayout Flex:Year
	LabelString Year
	LatexType             Command
	DocBookTag            year
	LabelFont
	  Color               blue
	EndFont
	ResetsFont true
End


InsetLayout Flex:Issue-number
	LabelString Issue-number
	LatexType             Command
	DocBookTag            issue-number
	LabelFont
	  Color               blue
	EndFont
	ResetsFont true
End


InsetLayout Flex:Issue-day
	LabelString Issue-day
	LatexType             Command
	DocBookTag            issue-day
	LabelFont
	  Color               blue
	EndFont
	ResetsFont true
End


InsetLayout Flex:Issue-months
	LabelString Issue-months
	LatexType             Command
	DocBookTag            issue-months
	LabelFont
	  Color               blue
	EndFont
	ResetsFont true
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
