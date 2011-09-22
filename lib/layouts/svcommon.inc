# svcommon (article) textclass definition file for inclusion.
# for Springer Verlag publications
# based on sjour.inc:
#     --------
#     Taken from initial LyX source code
#     Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
#     Transposed by Pascal André <andre@via.ecp.fr>
#     Heavily modified and enhanced by serveral developers.
#     --------
#     ejour2 made originally by Martin Vermeer based on article.layout.
#     Modified for LyX 1.2 by Jean-Marc Lasgouttes and MV.
#     Minor change for LyX 1.3.3 by Cengiz Gunay.
#
#     Adapted for new svjour LaTeX2e classes published by Springer in 1999;
#     work done by MV in 2000.
#
# Adapted for svmono, svmult, and svjour3 2011
# by Marcus Kriele (mkriele@mac.com) and Uwe Stöhr
# see http://www.springer.com/authors for class files and further instructions


Format 35
PageStyle		Plain


Style Standard
	Margin		Static
	LatexType	Paragraph
	LatexName	dummy
	ParIndent	MM
	ParSkip		0.4
	Align		Block
	AlignPossible	Block, Left, Right, Center
	LabelType	No_Label
End

Input stdcounters.inc
Input stdfloats.inc
Input stdlists.inc
Input stdinsets.inc
Input stdsections.inc
Input stdstarsections.inc
Input stdlayouts.inc
Input stdtitle.inc
Input numreport.inc

ProvidesModule theorems-std
Input theorems-without-preamble.inc
NoStyle Fact
NoStyle Prop

NoCounter subparagraph
NoStyle Subparagraph*


Style Title
	Align		Left
	AlignPossible	Block, Center, Left, Right
	Font
	  Family	Sans
	  Series	Bold
	EndFont
End



Style Front_Matter
	Category	Structure
	LatexType	Command
	LatexName	frontmatter
	KeepEmpty	1
	OptionalArgs	0
	Margin		First_Dynamic
	NextNoIndent	1
	ParSkip		0
	TopSep		2.5
	BottomSep	0.5
	ParSep		0
	Align		Center
	LabelType	Static
	LabelBottomSep	0
	LeftMargin	M
	LabelString	"--- Front Matter ---"

	Font
	  Family	Sans
	  Series	Bold
	  Size		Largest
	  Color		Blue
	EndFont
End

Style Main_Matter
	CopyStyle	Front_Matter
	Category	Structure
	LatexName	mainmatter
	LabelString	"--- Main Matter ---"
End

Style Back_Matter
	CopyStyle	Front_Matter
	Category	Structure
	LatexName	backmatter
	LabelString	"--- Back Matter ---"
End

Style Part
	Margin		Static
	Align		Right
End

## The following is a variant of style Part.  It allows to place some
## text on the following page which would otherwise be blank.  

Style PartBacktext
	Category	Section
	Margin		First_Dynamic
	LabelString	"Part \thepart"
	LabelType	Counter
	TocLevel	0
	LabelCounter	part
	Labelsep	xx
	NeedProtect	1
	ParIndent	MM
	TopSep		4
	BottomSep	4
	LaTexType	Environment
	LatexName	svmultpartbacktext
	LabelFont
	  Series	Bold
	  Size		Huge
	EndFont
	TextFont
	  Series	Medium
	  Shape		Up
	  Size		normal
	EndFont
	RequiredArgs	1
	Preamble
	  \newenvironment{svmultpartbacktext}[1]{\begin{partbacktext}
	  \part{#1}}{\end{partbacktext}}
	EndPreamble
	HTMLTag		h1
End

Style Chapter
	Align		Left
End

Style ChapSubtitle
	LatexName	chapsubtitle
	LatexType	Command
	Category	Section
	TopSep		0.7
	Font
	  Series	Bold
	  Shape		Up
	  Size		large
	EndFont

End

Style ChapAuthor
	LatexName	chapauthor
	LatexType	Command
	Category	Section
	TopSep		0.7
	Font
	  Series	Bold
	  Shape		Up
	  Size		normal
	EndFont
End

Style ChapMotto
	LatexName	motto
	LatexType	Command
	Category	Section
	TopSep		0.7
	BottomSep	0.7
	Align		Left
	LeftMargin	MMMMMMMMMMMM
	Font
	  Series	Medium
	  Shape		Italic
	  Size		normal
	EndFont
End

Style Section
	Font
	  Series	Bold
	  Shape		Up
	  Size		large
	EndFont
	TocLevel	1
End

Style Subsection
	Font
	  Series	Bold
	  Shape		Italic
	  Size		large
	EndFont
	TocLevel	2
End

Style Subsubsection
	Font
	  Series	Bold
	  Shape		Up
	  Size		Normal
	EndFont
	TocLevel	3
End

Style Paragraph
	Font
	  Series	Medium
	  Shape		Up
	  Size		Normal
	EndFont
	TopSep		0.7
	BottomSep	0.4
	TocLevel	4
End

Style Subparagraph
	LeftMargin	""
	Font
	  Series	Medium
	  Shape		Italic
	  Size		Normal
	EndFont
	TopSep		0.7
	BottomSep	0.4
	TocLevel	4
End

Style Part*
      Margin		Static
      Align		Right
End

Style Chapter*
	Align		Left
End

Style Section*
	Font
	  Series	Bold
	  Shape		Up
	  Size		large
	EndFont
End

Style Subsection*
	Font
	  Series	Bold
	  Shape		Italic
	  Size		large
	EndFont
End

Style Subsubsection*
	Font
	  Series	Bold
	  Shape		Up
	  Size		Normal
	EndFont
End

Style Paragraph*
	Font
	  Series	Medium
	  Shape		Up
	  Size		Normal
	EndFont
	TopSep		0.7
	BottomSep	0.4
End

Style Run-in_headings
	Category	Unnumbered
	LabelType	No_Label
	LabelSep	xx
	TopSep		0.7
     	Font
	  Series	Bold
	  Shape		Up
	  Size		Normal
	EndFont
	Margin		Static
	LatexType	Command
	LatexName	runinhead
End

Style Sub-run-in_headings
	CopyStyle	Run-in_headings
	Font 
	  Series	Medium
	  Shape		Italic
	  Size		Normal
	EndFont
	LatexName	subruninhead
End

Style Extrachap
	CopyStyle	Chapter*
	LatexName	Extrachap
End

Style extrachap
	CopyStyle	Chapter*
	LatexName	extrachap
End

Style Title
	Align		Left
	AlignPossible	Block, Center, Left, Right, Center
	Font
	  Family	Sans
	  Series	Bold
	EndFont
End

Style Subtitle
	CopyStyle	Title
	LatexName	subtitle
	NextNoIndent	1
	BottomSep	0
	ParSep		0
	Font
	  Size		Large
	EndFont
End

Style Author
	NextNoIndent	1
	Align		Left
	Font
	  Series	Bold
	  Shape		Italic
	EndFont
End

Style Institute
	Category	FrontMatter
	LatexType	Command
	InTitle		1
	LatexName	institute
	Margin		First_Dynamic
	LabelSep	x
	LabelType	Static
	LabelString	"Author data:"
	LabelFont
	  Family	Typewriter
	  Color		Blue
	EndFont
	Requires	url
	Align 		Left
End

Style Date
	Align		Left
	Font
	  Size		Small
	EndFont
End

Style TOC_Title
	Category	FrontMatter
	Margin		First_Dynamic
	Align		Left
	LatexType	Command
	LatexName	toctitle
	LeftMargin	xx
	TopSep		0.4
	BottomSep	0.4
	LabelType	Static
	LabelSep	x
	LabelString	"TOC title:"
	LabelFont
	  Family	Typewriter
	  Color		Blue
	EndFont  
End

Style TOC_Author
	CopyStyle	TOC_Title
	LatexName	tocauthor
	LabelString	"TOC author:"
End

Style Running_Title
	CopyStyle	TOC_Title
	LatexName	titlerunning
	InTitle		1
	LabelString	"Running title:"
End

Style Running_Author
	CopyStyle	Running_Title
	LatexName	authorrunning
	LabelString	"Running author:"
End

Style Running_Chapter
	CopyStyle	Running_Title
	Category	Section
	InTitle		0
	LatexName	chaptermark
	LabelString	"Running chapter:"
End

Style Running_Section
	CopyStyle	Running_Chapter
	LatexName	sectionmark
	LabelString	"Running section:"
End

Style Abstract
	Category	FrontMatter
	Margin		First_Dynamic
	LatexType	Environment
	LatexName	abstract
	LabelType	Static
	LabelString	"Abstract"
	LabelFont
	  Series	Bold
	EndFont
	NextNoIndent	0
	ParIndent	MM
	LabelSep	xx
	ItemSep		0
	TopSep		0.7
	BottomSep	0.7
	ParSep		0
	Align		Block
	AlignPossible	Block, Left
End

Style Abstract*
	CopyStyle	Abstract
	LatexType	Command
	LatexName	abstract*
	LabelString	"Abstract* (not printed)"
	LabelFont
	  Color		Blue
	EndFont
	TextFont	
	  Color		Blue
	EndFont
End

Style Keywords
	Margin		First_Dynamic
	LatexType	Command
	LatexName	keywords
	Category	FrontMatter
	NextNoIndent	0
	ParIndent	MM
	LabelSep	xx
	ItemSep		0
	TopSep		0.7
	BottomSep	0.7
	ParSep		0
	Align		Block
	AlignPossible	Block, Left
	LabelType	Static
	LabelString	"Keywords"
	LabelFont
	  Series	Bold
	EndFont
End

Style Dedication
	Category	FrontMatter
	Margin		Static
	LatexType	Environment
	LatexName	dedication
	LeftMargin	MMMMMMMMMMMM
	TopSep		2
	ParIndent	MM
	Parsep		0
	BottomSep	2
	Align		Left
	LabelType	Static
	LabelString	""
	LabelIndent	""
	Font
	  Shape		Italic
	  Size		large
	EndFont
End

Style Foreword
	Margin		First_Dynamic
	Category	FrontMatter
	LabelType	Static
	LabelString	"Foreword"
	KeepEmpty	1
	LatexName	foreword
	TocLevel	0
	LatexType	Command
	NeedProtect	1
	NextNoIndent	1
	ParSkip		0.4
	TopSep		4
	BottomSep	0.8
	ParSep		0.8
	Align		Block
	OptionalArgs	1
	Font
	  Series	Bold
	  Size		Huge
	EndFont
	HTMLTag		h1
End

Style Preface
	CopyStyle	Foreword
	Category	FrontMatter
	LatexName	preface
	InTitle		0
	Margin		First_Dynamic
	TopSep		4
	BottomSep	0.8
	LabelType	Static
	LabelString	"Preface"
	LabelSep	xx
	Align		Left
End

Style Acknowledgement
	Category	BackMatter
	Margin		First_Dynamic
	LatexType	Environment
	LatexName	acknowledgement
	NextNoIndent	0
	ParIndent	MM
	LabelSep	xx
	ItemSep		0
	TopSep		0.7
	BottomSep	0.7
	ParSep		0
	Align		Block
	AlignPossible	Block, Left
	LabelType	Static
	LabelString	"Acknowledgements"
	Font
	  Size		Small
	EndFont
	LabelFont
	  Series	Bold
	  Shape		Up
	  Size		Small
	EndFont
End

Style Acknowledgements
	CopyStyle	Acknowledgement
	LatexName	acknowledgements
End

Style Bibliography
	Margin		First_Dynamic
	LatexType	Bib_Environment
	LatexName	thebibliography
	Category	BackMatter
	NextNoIndent	1
	LeftMargin	MM
	ParSkip		0.4
	ItemSep		0
	TopSep		0.7
	BottomSep	0.7
	ParSep		0.5
	Align		Block
	AlignPossible	Block, Left
	LabelType	Bibliography
	LabelString	"References"
	LabelBottomSep	0.7
	LabelFont
	  Series	Bold
	EndFont
End

Style Description
	OptionalArgs	1
	LabelFont
	  Series	Medium
	EndFont
End

Style Petit
	CopyStyle	Standard
	Category	MainText
	LatexType	Environment
	LatexName	petit
	TopSep		0.7
	BottomSep	0.7
	Font
	  Size		Small
	EndFont
End

Style Svgraybox
	Category	MainText
	Margin		Static
	LatexType	Environment
	LatexName	svgraybox
	RightMargin	MMM
	TopSep		0.5
	ParIndent	MM
	Parsep		0
	BottomSep	0.5
	Align		Block
	AlignPossible	Block, Left, Right, Center
	LabelType	No_Label
	Font
	  Family	Sans
	EndFont
End

Style Proof
	Category	Theorem
	LatexType	Environment
	LatexName	proof
	Margin		First_Dynamic
	LeftMargin	xxx
	TopSep		0.4
	BottomSep	0.4
	LabelType	Static
	LabelSep	xx
	LabelString	"Proof."
	LabelFont
	  Shape		Italic
	EndFont
	LangPreamble
		\providecommand{\proofname}{_(Proof)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\proofname}{_(Proof)}}
	EndBabelPreamble
End

Style Proof(QED)
	CopyStyle	Proof
	LatexName	svmultproof
	EndLabelType	Box
	Preamble
		\newenvironment{svmultproof}{\begin{proof}}{\qed\end{proof}}
	EndPreamble
End

Style Proof(smartQED)
	CopyStyle	Proof
	LatexName	svmultproof2
	EndLabelType	Box
	Preamble
		\newenvironment{svmultproof2}{\begin{proof}}{\smartqed\qed\end{proof}}
	EndPreamble
End

InsetLayout Flex:Email
	Decoration	Conglomerate
	LyxType		custom
	LatexType	command
	LatexName	email
	LabelString	"Email"
	Font
	  Color		foreground
	  Size		Small
	  Family	Roman
	  Shape		Up
	  Series	Medium
	  Misc		No_Emph
	  Misc		No_Noun
	  Misc		No_Bar
	EndFont
	LabelFont
	  Color		urltext
	  Size		Small
	EndFont
End

