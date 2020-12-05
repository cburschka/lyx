# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>

# This file is the counterpart of stdclass.inc
# It is desirable, as far as possible, to have the same look and feel for
# related layouts in latex and docbook.

Format 89

Columns                 1
Sides                   1
SecNumDepth             3
TocDepth                3
DefaultStyle            Standard

Style Standard
	Category              MainText
	Margin                Static
	LatexType             Paragraph
	LatexName             para
	ParIndent             MM
	ParSkip               0.4
	Align                 Block
	LabelType             No_Label
	DocBookTag            para
End

Input stdinsets.inc
Input db_stdcharstyles.inc
# Input db_stdlists.inc
# Input db_stdsections.inc
Input db_stdstarsections.inc
Input db_stdtitle.inc
Input db_stdstruct.inc
# Input db_lyxmacros.inc
# Input db_stdlayouts.inc
Input stdcounters.inc
# Input db_stdcounters.inc
Input stdfloats.inc
