# KOMA script textclass definition file.
# Base definitions for the KOMA script layouts.
# Bernd Rellermeyer <100.41728@germanynet.de>, 1998/7/23.
# Update for Koma Script Release >=2.8q
# Juergen Spitzmueller <j.spitzmueller@gmx.de>, 2003/2/08.

# General textclass parameters
SecNumDepth             2
TocDepth                2
DefaultStyle            Standard


Style Standard
	LatexName             dummy
	ParIndent             MM
	ParSkip               0.4
	AlignPossible         Block, Left, Right, Center
End

Input stdlists.inc
Input stdcounters.inc
Input stdfloats.inc

# Modify Description
Style Description
	LabelFont
	  Family              Sans
	EndFont
End

# Delete List
NoStyle List


Style Labeling
	Margin                Manual
	LatexType             List_Environment
	LatexName             labeling
	NextNoindent          1
	LabelSep              xxx
	ParSkip               0.4
	TopSep                0.7
	BottomSep             0.7
	ParSep                0.5
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Manual
	LabelString           00.00.0000
End

Input stdsections.inc

# Modify Part
Style Part
	Font
	  Family              Sans
	EndFont
End

# Modify Chapter
Style Chapter
	LabelString           ""
	Font
	  Family              Sans
	  Size                Largest
	EndFont
End

# Modify Section
Style Section
	Font
	  Family              Sans
	EndFont
End

# Modify Subsection
Style Subsection
	Font
	  Family              Sans
	EndFont
End

# Modify Subsubsection
Style Subsubsection
	Font
	  Family              Sans
	EndFont
End

# Modify Paragraph
Style Paragraph
	Font
	  Family              Sans
	EndFont
End

# Modify Subparagraph
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

# Modify Title
Style Title
	Font
	  Family              Sans
	  Series              Bold
	EndFont
End

# Modify Author
Style Author
	Font
	  Size                Larger
	EndFont
End

# Modify Date
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
	AlignPossible         Left

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

# Modify Abstract
Style Abstract
	LabelFont
	  Family              Sans
	EndFont
End

# Modify Bibliography
Style Bibliography
	LabelFont
	  Family              Sans
	EndFont
End

Input stdlayouts.inc


Style Captionabove
	CopyStyle            Caption
	LatexName            captionabove
End


Style Captionbelow
	CopyStyle            Caption
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
	AlignPossible        Left
	OptionalArgs         1
	
	Font
	  Size                Normal
	EndFont
End

Input lyxmacros.inc
Input scrmacros.inc
