# KOMA script textclass definition file.
# Base definitions for the KOMA script layouts.
# Bernd Rellermeyer <100.41728@germanynet.de>, 1998/7/23.
# Update for Koma Script Release >=2.8q
# Juergen Spitzmueller <j.spitzmueller@gmx.de>, 2003/2/08.
# Mapped List to Labeling
# Guenter Milde <g.milde web.de>


Format 10
SecNumDepth             2
TocDepth                2
DefaultStyle            Standard

Style Standard
	LatexName             dummy
	ParIndent             MM
	ParSkip               0.4
	AlignPossible         Block, Left, Right, Center
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
	CopyStyle             List
	LatexName             labeling
	# overwrite the preamble code definition
	Preamble
	EndPreamble
End

Style List
	Obsoletedby           Labeling
End

Input stdsections.inc

Style Part
	Font
	  Family              Sans
	EndFont
End


Style Chapter
	Font
	  Family              Sans
	  Size                Largest
	EndFont
End


Style Section
	Font
	  Family              Sans
	EndFont
End


Style Subsection
	Font
	  Family              Sans
	EndFont
End


Style Subsubsection
	Font
	  Family              Sans
	EndFont
End


Style Paragraph
	Font
	  Family              Sans
	EndFont
End


Style Subparagraph
	LeftMargin            ""
	Font
	  Family              Sans
	EndFont
End

Input stdstarsections.inc

Style Addpart
	CopyStyle             Part*
	LatexName             addpart
End


Style Addchap
	CopyStyle             Chapter*
	LatexName             addchap
End


Style Addsec
	CopyStyle             Section*
	LatexName             addsec
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

