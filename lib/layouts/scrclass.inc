# KOMA script textclass definition file.
# Base definitions for the KOMA script layouts.
# Bernd Rellermeyer <100.41728@germanynet.de>, 1998/7/23.
# Update for Koma Script Release >=2.8q
# Juergen Spitzmueller <j.spitzmueller@gmx.de>, 2003/2/08.
# Mapped List to Labeling
# Guenter Milde <milde users.berlios.de>, 2009-02-02
# Labeling is now used also in non-KOMA classes


Format 35
SecNumDepth             2
TocDepth                2
DefaultStyle            Standard
Provides                subscript 1

Style Standard
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
End

Input stdinsets.inc
Input stdlists.inc
Input stdcounters.inc
Input stdfloats.inc

Style Description
	LabelFont
	  Family              Sans
	EndFont
End

Style Labeling
	LatexName             labeling
	# overwrite the preamble code definition
	Preamble
	EndPreamble
End

Input stdsections.inc

Style Part
	Font
	  Family              Sans
	EndFont
	TocLevel 0
End


Style Chapter
	Font
	  Family              Sans
	  Size                Largest
	EndFont
	TocLevel 0
End


Style Section
	Font
	  Family              Sans
	EndFont
	TocLevel 1
End


Style Subsection
	Font
	  Family              Sans
	EndFont
	TocLevel 2
End


Style Subsubsection
	Font
	  Family              Sans
	EndFont
	TocLevel 3
End


Style Paragraph
	Font
	  Family              Sans
	EndFont
	TocLevel 4
End


Style Subparagraph
	LeftMargin            ""
	Font
	  Family              Sans
	EndFont
	TocLevel 5
End

Input stdstarsections.inc

Style Addpart
	CopyStyle             Part*
	LatexName             addpart
	OptionalArgs          1
End


Style Addchap
	CopyStyle             Chapter*
	LatexName             addchap
	OptionalArgs          1
End


Style Addsec
	CopyStyle             Section*
	LatexName             addsec
	OptionalArgs          1
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
	Category              Section
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
End

Style Subtitle
	CopyStyle             Title
	LatexName             subtitle
	Font
	  Size                Large
	EndFont
End


Style Author
	Font
	  Size                Larger
	EndFont
End


Style Date
	Font
	  Size                Larger
	EndFont
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

Input stdstruct.inc

Style Abstract
	LabelFont
	  Family              Sans
	EndFont
End


Style Bibliography
	LabelFont
	  Family              Sans
	EndFont
End

Input stdlayouts.inc

Style Captionabove
	Category              Captions
	Margin                First_Dynamic
	LatexType             Command
	LatexName             caption
	NeedProtect           1
	LabelSep              xx
	ParSkip               0.4
	TopSep                0.5
	Align                 Center
	LabelType             Sensitive
	LabelString           "Senseless!"
	OptionalArgs          1
	LabelFont
	  Series              Bold
	EndFont
	LatexName            captionabove
End


Style Captionbelow
	Category              Captions
	Margin                First_Dynamic
	LatexType             Command
	LatexName             caption
	NeedProtect           1
	LabelSep              xx
	ParSkip               0.4
	TopSep                0.5
	Align                 Center
	LabelType             Sensitive
	LabelString           "Senseless!"
	OptionalArgs          1
	LabelFont
	  Series              Bold
	EndFont
	LatexName            captionbelow
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
	OptionalArgs         1
	Font
	  Size               Normal
	EndFont
End

Input lyxmacros.inc

