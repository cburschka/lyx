# Textclass definition file for agu-dtd.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>
# Modified Martin Vermeer <martin.vermeer@hut.fi> 
#
# This file is the counterpart of stdstyle.inc
# It is desirable, as far as possible, to have the same look and feel for
# related layouts in latex and docbook.

Input stdtitle.inc


Style Header
	Margin                First_dynamic
	LatexType             Command
	LatexName             header
	LabelSep              xx
	LabelType             Centered_Top_Environment
	LabelString           "<header>"
	CommandDepth          2
	KeepEmpty             1
	NextNoIndent          1
	LabelFont
	  Color               green
	EndFont
End


Style Special-section
	CopyStyle             Header
	LatexName             special-section
	LabelType             Static
	LabelString           "<special-section>"
	CommandDepth          3
	LabelFont
	  Color               blue
	EndFont
End


Style AGU-journal
	CopyStyle             Special-section
	LatexName             AGU-journal
	LabelString           "<AGU-journal>"
	CommandDepth          3
End


Style AGU-volume
	CopyStyle             Special-section
	LatexName             AGU-volume
	LabelString           "<AGU-volume>"
	CommandDepth          3
End


Style AGU-issue
	CopyStyle             Special-section
	LatexName             AGU-issue
	LabelString           "<AGU-issue>"
	CommandDepth          3
End


Style Copyright
	CopyStyle             Special-section
	LatexName             copyright
	LabelString           "<copyright>"
	CommandDepth          3
End


Style Indexterms
	CopyStyle             Special-section
	LatexName             index-terms
	LabelString           "<index-terms>"
	CommandDepth          3
End


Style Keywords
	CopyStyle             Special-section
	LatexName             keywords
	LabelString           "<keywords>"
	CommandDepth          3
End


Style Affiliation
	CopyStyle             Special-section
	LatexName             affiliation
	LabelString           "<affiliation>"
	CommandDepth          3
End


Style Supplementary
	CopyStyle             Special-section
	LatexName             sup-mat-ref
	LabelString           "<sup-mat-ref>"
	CommandDepth          3
End


Style Author
	CopyStyle             Special-section
	LatexName             author
	LabelString           "<author>"
	CommandDepth          3
End


Style Abstract
	LatexType             Environment
	LabelType             Centered_Top_Environment
	LabelString           "Abstract"
	CommandDepth          3
End


Style Received
	CopyStyle             Special-section
	LatexName             received
	LabelString           "<received>"
	CommandDepth          3
End


Style Revised
	CopyStyle             Special-section
	LatexName             revised
	LabelString           "<revised>"
	CommandDepth          3
End


Style Accepted
	CopyStyle             Special-section
	LatexName             accepted
	LabelString           "<accepted>"
	CommandDepth          3
End


Style Identline
	CopyStyle             Special-section
	LatexName             ident-line
	LabelString           "<ident-line>"
	CommandDepth          3
End


Style Runhead
	CopyStyle             Special-section
	LatexName             runhead
	LabelString           "<runhead>"
	CommandDepth          3
End


Style Date
	CopyStyle             Special-section
	LatexName             date
	LabelString           "<date>"
	CommandDepth          3
End


Style Title
	CopyStyle             Special-section
	LatexName             title
	LabelString           "<title>"
	CommandDepth          3
End


Style Citation
	CopyStyle             Special-section
	LatexName             citation
	LabelString           "<citation>"
	CommandDepth          3
End


