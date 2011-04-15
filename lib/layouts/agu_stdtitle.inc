# Textclass definition file for agu-dtd.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>
# Modified Martin Vermeer <martin.vermeer@hut.fi>
#
# This file is the counterpart of stdstyle.inc
# It is desirable, as far as possible, to have the same look and feel for
# related layouts in latex and docbook.

Format 35
Input stdtitle.inc


Style Header
	Margin                Static
	LatexType             Command
	LatexName             header
	LabelSep              xx
	LabelType             Centered_Top_Environment
	LabelString           "-- Header --"
	CommandDepth          2
	KeepEmpty             1
	NextNoIndent          1
	LabelFont
	  Size                Largest
	  Series              Bold
	  Color               blue
	EndFont
End


Style Special-section
	CopyStyle             Header
	Margin                Dynamic
	LatexName             special-section
	LabelType             Static
	LabelSep              M
	LabelString           "Special-section:"
	CommandDepth          3
	LabelFont
	  Size                Normal
	  Series              Medium
	EndFont
End



Style AGU-journal
	CopyStyle             Special-section
	LatexParam            "journal-code=<$1> electronic-only=<yes>"
	LatexName             AGU-journal
	LabelString           "AGU-journal:"
	CommandDepth          3
End


Style Citation-number
	CopyStyle             Special-section
	LatexName             citation-number
	LabelString           "Citation-number:"
	CommandDepth          3
End



Style AGU-volume
	CopyStyle             Special-section
	LatexName             AGU-volume
	LabelString           "AGU-volume:"
	CommandDepth          3
End


Style AGU-issue
	CopyStyle             Special-section
	LatexName             AGU-issue
	LabelString           "AGU-issue:"
	CommandDepth          3
End


Style Copyright
	CopyStyle             Special-section
	LatexName             copyright
	LabelString           "Copyright:"
	CommandDepth          3
End


Style Index-terms
	CopyStyle             Special-section
	LatexName             index-terms
	TopSep                1.0
	LabelString           "Index-terms..."
	CommandDepth          3
	LabelFont
	  Shape               Italic
      Size                Large
	EndFont
End


Style Index-term
	CopyStyle             Special-section
	LatexName             index-term
	LabelIndent           MM
	LabelString           "Index-term:"
	CommandDepth          4
End


Style Cross-term
	CopyStyle             Special-section
	LatexName             cross-term
	LabelIndent           MM
	LabelString           "Cross-term:"
	CommandDepth          4
End


Style Keywords
	CopyStyle             Special-section
	LatexName             keywords
	LabelString           "Keywords:"
	CommandDepth          3
End


Style Affiliation
	CopyStyle             Special-section
	LatexName             affiliation
	LabelString           "Affiliation:"
	CommandDepth          3
End


Style Supplementary
	CopyStyle             Special-section
	LatexName             sup-mat-ref
	TopSep                1.0
	LabelString           "Supplementary..."
	CommandDepth          3
	LabelFont
	  Shape               Italic
	  Size                Large
	EndFont
End


Style Supp-note
	CopyStyle             Special-section
	LatexName             sup-mat-note
	LabelIndent           MM
	LabelString           "Sup-mat-note:"
	CommandDepth          4
End


Style Cite-other
	CopyStyle             Special-section
	LatexName             cite-other
	LabelIndent           MM
	LabelString           "Cite-other:"
	CommandDepth          4
End


Style Author
	CopyStyle             Special-section
	Margin                Static
	LeftMargin            MMM
	LatexName             author
	TopSep                0.8
	BottomSep             0.8
	Align                 Center
	AlignPossible         Center
	LabelType             No_Label
	CommandDepth          3
	Font
	  Size                Large
	EndFont
End


Style Name
	CopyStyle             Special-section
	LatexName             name
	LabelIndent           MM
	LabelString           "Name:"
	CommandDepth          4
End


Style Abstract
	CommandDepth          3
End


Style Received
	CopyStyle             Special-section
	LatexName             received
	LabelString           "Received:"
	CommandDepth          3
End


Style Revised
	CopyStyle             Special-section
	LatexName             revised
	LabelString           "Revised:"
	CommandDepth          3
End


Style Accepted
	CopyStyle             Special-section
	LatexName             accepted
	LabelString           "Accepted:"
	CommandDepth          3
End


Style Ident-line
	CopyStyle             Special-section
	LatexName             ident-line
	LabelString           "Ident-line:"
	CommandDepth          3
End


Style Runhead
	CopyStyle             Special-section
	LatexName             runhead
	LabelString           "Runhead:"
	CommandDepth          3
End


Style Date
	CopyStyle             Special-section
	LatexName             published-online
	LabelString           "Published-online:"
	CommandDepth          3
End


Style Title
	CopyStyle             Special-section
	Margin                Static
	LeftMargin            MMM
	LatexName             title
	CommandDepth          3
	TopSep                1.0
	BottomSep             1.0
	Align                 Center
	AlignPossible         Center
	LabelType             No_Label
    Font
      Size                Largest
	EndFont
End


Style Citation
	CopyStyle             Special-section
	LatexName             citation
	LabelString           "Citation:"
	CommandDepth          3
End


Style Posting-order
	CopyStyle             Special-section
	LatexName             posting-order
	LabelString           "Posting-order:"
	CommandDepth          3
End


Style AGU-pages
	CopyStyle             Special-section
	LatexName             AGU-pages
	LabelString           "AGU-pages:"
	CommandDepth          3
End


Style Words
	CopyStyle             Special-section
	LatexName             words
	LabelString           "Words:"
	CommandDepth          3
End


Style Figures
	CopyStyle             Special-section
	LatexName             figures
	LabelString           "Figures:"
	CommandDepth          3
End


Style Tables
	CopyStyle             Special-section
	LatexName             tables
	LabelString           "Tables:"
	CommandDepth          3
End


Style Datasets
	CopyStyle             Special-section
	LatexName             datasets
	LabelString           "Datasets:"
	CommandDepth          3
End


# Inside AGU-journal:

InsetLayout Flex:ISSN
	LyXType element
	LabelString ISSN
	LatexType             Command
	LatexName             ISSN
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Flex:CODEN
	LyXType element
	LabelString CODEN
	LatexType             Command
	LatexName             CODEN
	LabelFont
	  Color               blue
	EndFont
End

# Inside Special-section:

InsetLayout Flex:SS-Code
	LyXType element
	LabelString SS-Code
	LatexType             Command
	LatexName             ss-code
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Flex:SS-Title
	LyXType element
	LabelString SS-Title
	LatexType             Command
	LatexName             ss-title
	LabelFont
	  Color               blue
	EndFont
End

# Inside Copyright:

InsetLayout Flex:CCC-Code
	LyXType element
	LabelString CCC-Code
	LatexType             Command
	LatexName             CCCcode
	LabelFont
	  Color               blue
	EndFont
End


# Inside Index-term:

InsetLayout Flex:Code
	LyXType element
	LabelString Code
	LatexType             Command
	LatexName             code
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Flex:Dscr
	LyXType element
	LabelString Dscr
	LatexType             Command
	LatexName             dscr
	LabelFont
	  Color               blue
	EndFont
End

# Inside Keywords:

InsetLayout Flex:Keyword
	LyXType element
	LabelString Keyword
	LatexType             Command
	LatexName             keyword
	LabelFont
	  Color               blue
	EndFont
End

#inside Affiliation:

InsetLayout Flex:Orgdiv
	LyXType element
	LabelString Orgdiv
	LatexType             Command
	LatexName             orgdiv
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Flex:Orgname
	LyXType element
	LabelString Orgname
	LatexType             Command
	LatexName             orgname
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Flex:Street
	LyXType element
	LabelString Street
	LatexType             Command
	LatexName             street
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Flex:City
	LyXType element
	LabelString City
	LatexType             Command
	LatexName             city
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Flex:State
	LyXType element
	LabelString State
	LatexType             Command
	LatexName             state
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Flex:Postcode
	LyXType element
	LabelString Postcode
	LatexType             Command
	LatexName             postcode
	LabelFont
	  Color               blue
	EndFont
End


InsetLayout Flex:Country
	LyXType element
	LabelString Country
	LatexType             Command
	LatexName             country
	LabelFont
	  Color               blue
	EndFont
End


