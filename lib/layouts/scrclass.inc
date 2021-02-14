# KOMA script textclass definition file.
# Base definitions for the KOMA script layouts.
# Bernd Rellermeyer <100.41728@germanynet.de>, 1998/7/23.
# Update for Koma Script Release >=2.8q
# Juergen Spitzmueller <spitz@lyx.org>, 2003/2/08.
# Mapped List to Labeling
# Guenter Milde <milde@lyx.org>, 2009-02-02
# Labeling is now used also in non-KOMA classes


Format 93
SecNumDepth             2
TocDepth                2
DefaultStyle            Standard
PageSize                a4
Provides                subscript 1

ClassOptions
	FontSize             9|10|11|12|13|14|15|16|17|18|19|20
	FontSizeFormat       fontsize=$$s
	PageSize             a0|a1|a2|a3|a4|a5|a6|b0|b1|b2|b3|b4|b5|b6|c0|c1|c2|c3|c4|c5|c6|executive|legal|letter
	PageSizeFormat       paper=$$s
End

Style Standard
	Category              MainText
	LatexName             dummy
	ParIndent             MM
	ParSkip               0.4
	AlignPossible         Block, Left, Right, Center
	# FIXME This ought to be set dynamically.
	HTMLStyle
		div.standard {
			text-indent: 2em;
			margin-bottom: 2ex;
		}
	EndHTMLStyle
	DocBookTag            para
End

Input stdinsets.inc
Input stdlists.inc
Input stdcounters.inc
Input stdfloats.inc


# Adapt enumii counter display to KOMA default
Counter enumii
	GuiName "Numbered List (Level 2)"
	Within               enumi
	LabelString          "\alph{enumii})"
End


Style Description
	LabelFont
	  Family              Sans
	EndFont
End

Style Labeling
	Category              List
	LatexName             labeling
	# overwrite the preamble code definition
	Preamble
	EndPreamble	
	DocBookTag            variablelist
	DocBookItemWrapperTag varlistentry
	DocBookItemTag        listitem
	DocBookItemInnerTag   para
	DocBookItemLabelTag   term
	DocBookItemLabelTagType   paragraph
End

Input stdsections.inc

Style Part
	Font
	  Family              Sans
	EndFont
End


Style Chapter
	LabelType             Static
	LabelSep              xxx
	Margin                Dynamic
	Align                 Left
	Font
	  Family              Sans
	  Size                Largest
	EndFont
End


Style Section
	Align                 Left
	Font
	  Family              Sans
	EndFont
End


Style Subsection
	Align                 Left
	Font
	  Family              Sans
	EndFont
End


Style Subsubsection
	Align                 Left
	Font
	  Family              Sans
	EndFont
End


Style Paragraph
	Align                 Left
	Font
	  Family              Sans
	EndFont
End


Style Subparagraph
	Align                 Left
	LeftMargin            ""
	Font
	  Family              Sans
	EndFont
End

Input stdstarsections.inc

Style Addpart
	CopyStyle             Part*
	LatexName             addpart
	Argument 1
		LabelString   "Short Title|S"
		Tooltip       "The part as it appears in the table of contents/running headers"
		InsertCotext  1
	EndArgument
End


Style Addchap
	CopyStyle             Chapter*
	LatexName             addchap
	Argument 1
		LabelString   "Short Title|S"
		Tooltip       "The chapter as it appears in the table of contents/running headers"
		InsertCotext  1
	EndArgument
End


Style Addsec
	CopyStyle             Section*
	LatexName             addsec
	Argument 1
		LabelString   "Short Title|S"
		Tooltip       "The section as it appears in the table of contents/running headers"
		InsertCotext  1
	EndArgument
End


Style Addchap*
	CopyStyle             Chapter*
	LatexName             addchap*
End


Style Addsec*
	CopyStyle             Section*
	LatexName             addsec*
End


Style Minisec
	Category              Sectioning
	LatexType             Command
	LatexName             minisec
	NeedProtect           1
	NextNoIndent          1
	LabelSep              xxx
	ParSkip               0.4
	TopSep                0.4
	ParSep                0.4
	AlignPossible         Block, Left
	Font
	  Family              Sans
	  Series              Bold
	EndFont
End

Input stdtitle.inc

Style Title
	Font
	  Family              Sans
	  Series              Bold
	EndFont
	DocBookTag            title
	DocBookInInfo         maybe
End

Style Subtitle
	CopyStyle             Title
	LatexName             subtitle
	Font
	  Size                Large
	EndFont
	DocBookTag            subtitle
	DocBookInInfo         maybe
End


Style Author
	Font
	  Size                Larger
	EndFont
	DocBookTag            personname
	DocBookTagType        paragraph
	DocBookWrapperTag     author
	DocBookWrapperTagType inline
	DocBookInInfo         always
End


Style Date
	Font
	  Size                Larger
	EndFont
	DocBookTag            date
	DocBookTagType        paragraph
	DocBookInInfo         always
    DocBookNoFontInside   true
End


Style Subject
	CopyStyle             Author
	LatexName             subject
End


Style Publishers
	CopyStyle             Author
	LatexName             publishers
End


Style Dedication
	CopyStyle             Author
	LatexName             dedication
End


Style Titlehead
	CopyStyle             Author
	LatexName             titlehead
	Align                 Left
	Font
	  Size                Normal
	EndFont
End


Style Uppertitleback
	CopyStyle             Titlehead
	LatexName             uppertitleback
End


Style Lowertitleback
	CopyStyle             Titlehead
	LatexName             lowertitleback
End


Style Extratitle
	CopyStyle             Titlehead
	LatexName             extratitle
End

Style Frontispiece
	CopyStyle             Titlehead
	LatexName             frontispiece
End

Input stdstruct.inc

Style Abstract
	LabelType             No_Label
End


Style Bibliography
	LabelFont
	  Family              Sans
	EndFont
End

Input stdlayouts.inc


InsetLayout Caption:Above
	LabelString          above
	LaTeXType            command
	LatexName            captionabove
	NeedProtect          1
	MultiPar             false
	Argument 1
		LabelString   "Short Title|S"
		Tooltip       "The caption as it appears in the list of figures/tables"
		InsertCotext  1
	EndArgument
	HTMLStyle
		div.float-caption {
			text-align: center;
			border: 2px solid black;
			padding: 1ex;
			margin: 1ex;
		}
	EndHTMLStyle
	HTMLAttr "class='float-caption float-caption-above'"
End


InsetLayout Caption:Below
	LabelString          below
	LaTeXType            command
	LatexName            captionbelow
	NeedProtect          1
	MultiPar             false
	Argument 1
		LabelString   "Short Title|S"
		Tooltip       "The caption as it appears in the list of figures/tables"
		InsertCotext  1
	EndArgument
	HTMLStyle
		div.float-caption {
			text-align: center;
			border: 2px solid black;
			padding: 1ex;
			margin: 1ex;
		}
	EndHTMLStyle
	HTMLAttr "class='float-caption float-caption-below'"
End


Style Dictum
	Category             MainText
	Margin               Right_Address_Box
	LatexType            Command
	LatexName            dictum
	ParSkip              0.4
	BottomSep            1.5
	ParSep               1.5
	Align                Left
	Argument 1
		LabelString   "Dictum Author"
		Tooltip       "The author of this dictum"
	EndArgument
	Font
	  Size               Normal
	EndFont
End

Input lyxmacros.inc

