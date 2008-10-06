# svjour (article) textclass definition file for inclusion.
# for Springer Verlag journals
# --------
#
# Taken from initial LyX source code
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Transposed by Pascal André <andre@via.ecp.fr>
# Heavily modified and enhanced by serveral developers.
#
# --------
# ejour2 made originally by Martin Vermeer based on article.layout.
# Modified for LyX 1.2 by Jean-Marc Lasgouttes and MV.
# Minor change for LyX 1.3.3 by Cengiz Gunay.
#
# Adapted for new svjour LaTeX2e classes published by Springer in 1999;
# work done by MV in 2000.
#
# see www.springer.de/author/tex/help-journals.html for class files


Format 9
Columns                 2
Sides                   1
PageStyle               Plain


Style Standard
	Margin                Static
	LatexType             Paragraph
	LatexName             dummy
	ParIndent             MM
	ParSkip               0.4
	Align                 Block
	AlignPossible         Block, Left, Right, Center
	LabelType             No_Label
End


Input stdinsets.inc
Input stdsections.inc
Input stdstarsections.inc
Input stdlayouts.inc
Input stdlists.inc
Input stdfloats.inc
Input stdcounters.inc

NoStyle Part
NoStyle Part*
NoStyle Chapter
NoStyle Chapter*


Style Section
	Font
	  Series              Medium
	  Series              Bold
	  Size                Normal
	EndFont
End


Style Subsection
	Font
	  Series              Medium
	  Shape               Italic
	  Size                Normal
	EndFont
End


Style Subsubsection
	Font
	  Series              Medium
	  Shape               Up
	  Size                Normal
	EndFont
End


Style Paragraph
	Font
	  Series              Medium
	  Shape               Italic
	  Size                Normal
	EndFont
End


Style Subparagraph
	Font
	  Series              Medium
	  Shape               Up
	  Size                Normal
	EndFont
End


Style Headnote
	Margin                Static
	LatexType             Command
	InTitle               1
	LatexName             headnote
	NextNoIndent          1
	LeftMargin            "HeadNote (optional):xxx"
	LabelSep              xxx
	ParSkip               0.4
	TopSep                1.3
	BottomSep             0.7
	ParSep                0.7
	Align                 Left
	LabelType             Static
	LabelString           "Headnote (optional):"
	LabelFont
	  Series              Bold
	  Shape               Italic
	  Color               Red
	EndFont
End


Style Dedication
	CopyStyle             Headnote
	LatexName             dedication
	LeftMargin            Dedication:xxx
	LabelString           "Dedication:"
End


Style Title
	Margin                Static
	LatexType             Command
	InTitle               1
	LatexName             title
	NextNoIndent          1
	ParSkip               0.4
	TopSep                1.3
	BottomSep             0.7
	ParSep                0.7
	Align                 Left
	AlignPossible         Block, Center, Left, Right, Center
	LabelType             No_Label
	Fill_Top              1
	Fill_Bottom           1
	Font
	  Family              Sans
	  Series              Bold
	  Size                Largest
	EndFont
End


Style Subtitle
	Margin                Static
	LatexType             Command
	InTitle               1
	LatexName             subtitle
	NextNoIndent          1
	ParSkip               0.4
	ItemSep               0
	TopSep                0
	BottomSep             0
	ParSep                0
	Align                 Left
	LabelType             No_Label
	Font
	  Family              Sans
	  Series              Bold
	  Size                Large
	EndFont
End


Style Running_LaTeX_Title
	CopyStyle             Headnote
	LatexName             titlerunning
	LeftMargin            "Running title:xxx"
	LabelString           "Running title:"
End


Style Author
	Margin                Static
	LatexType             Command
	InTitle               1
	LatexName             author
	NextNoIndent          1
	ParSkip               0.4
	TopSep                0.7
	BottomSep             0.7
	ItemSep               0
	ParSep                0
	Align                 Left
	LabelType             No_Label
	Font
	  Series              Bold
	  Shape               Italic
	EndFont
End


Style Author_Running
	CopyStyle             Headnote
	LatexName             authorrunning
	LeftMargin            "Author Running:xxx"
	LabelString           "Author Running:"
End


Style Institute
	Margin                Static
	LatexType             Command
	InTitle               1
	LatexName             institute
	NextNoIndent          1
	ParSkip               0.4
	TopSep                1.3
	BottomSep             0.7
	ParSep                0.7
	Align                 Left
	LabelType             No_Label
	Font
	  Size                Small
	EndFont
End


Style Date
	CopyStyle             Institute
	LatexName             date
End


Style Mail
	CopyStyle             Headnote
	LatexName             mail
	LeftMargin            "Corr Author:xx."
	LabelString           "Corr Author:"
End


Style Offprints
	CopyStyle             Headnote
	LatexName             offprints
	LeftMargin            "Offprints:xx."
	LabelString           "Offprints:"
End


Style Abstract
	Margin                First_Dynamic
	LatexType             Environment
	LatexName             abstract
	NextNoIndent          0
	ParIndent             MM
	LabelSep              xx
	ItemSep               0
	TopSep                0.7
	BottomSep             0.7
	ParSep                0
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Top_Environment
	LabelString           "Abstract."
	LabelFont
	  Series              Bold
	EndFont
End


Style Keywords
	Margin                First_Dynamic
	LatexType             Command
	LatexName             keywords
	NextNoIndent          0
	ParIndent             MM
	LabelSep              xx
	ItemSep               0
	TopSep                0.7
	BottomSep             0.7
	ParSep                0
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Static
	LabelString           "Key words:"
	LabelFont
	  Series              Bold
	EndFont
End


Style Acknowledgements
	Margin                First_Dynamic
	LatexType             Environment
	LatexName             acknowledgements
	NextNoIndent          0
	ParIndent             MM
	LabelSep              xx
	ItemSep               0
	TopSep                0.7
	BottomSep             0.7
	ParSep                0
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Static
	LabelString           "Acknowledgements."
	Font
	  Size                Small
	EndFont
	LabelFont
	  Shape               Italic
	  Size                Small
	EndFont
End


Style Acknowledgement
	CopyStyle             Acknowledgements
	LatexName             acknowledgement
	LabelString           "Acknowledgement."
End


Style Bibliography
	Margin                First_Dynamic
	LatexType             Bib_Environment
	LatexName             thebibliography
	NextNoIndent          1
	LeftMargin            MM
	ParSkip               0.4
	ItemSep               0
	TopSep                0.7
	BottomSep             0.7
	ParSep                0.5
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Bibliography
	LabelString           "References"
	LabelBottomSep        0.7
	LabelFont
	  Series              Bold
	EndFont
End


Style Claim
	Margin                First_Dynamic
	LatexType             Environment
	LatexName             claim
	NextNoIndent          0
	ParIndent             MM
	LabelSep              xx
	ItemSep               0
	TopSep                0.5
	BottomSep             0.5
	ParSep                0
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Static
	LabelString           "Claim."
	LabelFont
	  Shape               Italic
	EndFont
End


Style Conjecture
	CopyStyle             Claim
	LatexName             conjecture
	LabelString           "Conjecture #."
End


Style Corollary
	CopyStyle             Claim
	LatexName             corollary
	LabelString           "Corollary #."
	Font
	  Shape               Italic
	EndFont
	LabelFont
	  Shape               Up
	  Series              Bold
	EndFont
End


Style Definition
	CopyStyle             Corollary
	LatexName             definition
	LabelString           "Definition #."
End


Style Example
	CopyStyle             Claim
	LatexName             example
	LabelString           "Example #."
End


Style Exercise
	CopyStyle             Claim
	LatexName             exercise
	LabelString           "Exercise #."
	LabelFont
	  Shape               Up
	  Series              Bold
	EndFont
End


Style Lemma
	CopyStyle             Corollary
	LatexName             lemma
	LabelString           "Lemma #."
End


Style Note
	CopyStyle             Claim
	LatexName             note
	LabelString           "Note #."
End


Style Problem
	CopyStyle             Exercise
	LatexName             problem
	LabelString           "Problem #."
End


Style Proof
	CopyStyle             Claim
	LatexName             proof
	LabelString           "Proof."
End


Style Property
	CopyStyle             Claim
	LatexName             property
	LabelString           "Property #."
End


Style Proposition
	CopyStyle             Claim
	LatexName             proposition
	LabelString           "Proposition #."
End


Style Question
	CopyStyle             Claim
	LatexName             question
	LabelString           "Question #."
End


Style Remark
	CopyStyle             Claim
	LatexName             remark
	LabelString           "Remark #."
End


Style Solution
	CopyStyle             Exercise
	LatexName             solution
	LabelString           "Solution #."
End


Style Theorem
	CopyStyle             Corollary
	LatexName             theorem
	LabelString           "Theorem #."
End
