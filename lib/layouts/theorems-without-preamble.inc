# This file provides theorem environments without adding preamble code or
# requiring packages. This is useful for classes that define their own theorems
# like svjour3 or ijmpc.
# Author: Uwe Stöhr <uwestoehr@web.de>

# The environnements defined are :
# - Theorem
# - Corollary
# - Lemma
# - Proposition
# - Prop
# - Conjecture
# - Fact
# - Definition
# - Example
# - Problem
# - Prob
# - Solution
# - Sol
# - Exercise
# - Remark
# - Claim
# - Case (defined without a counter)
# - Property
# - Question
# - Note


Format 98

OutlinerName thm "Definitions & Theorems"

Counter		theorem
	GuiName Theorem
	LaTeXName thm
End

Style Theorem
	Category	Reasoning
	Margin		First_Dynamic
	LatexType	Environment
	LatexName	theorem
	NextNoIndent	1
	AddToToc              thm
	IsTocCaption          1
	Argument 1
	  LabelString	"Name/Title"
	  Tooltip	"Alternative optional name or title"
	  IsTocCaption      1
	EndArgument
	LabelSep	xx
	ParIndent	MMM
	ParSkip		0.4
	ItemSep		0.2
	TopSep		0.7
	BottomSep	0.7
	ParSep		0.3
	Align		Block
	AlignPossible	Left
	LabelType	Static
	LabelCounter	theorem
	LabelString	"Theorem \thetheorem."
	Font
	  Shape		Italic
	  Size		Normal
	EndFont
	LabelFont
	  Shape		Up
	  Series	Bold
	EndFont
	LangPreamble
	  \providecommand{\theoremname}{_(Theorem)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\theoremname}{_(Theorem)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='theorem'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		corollary
	GuiName Corollary
End
Style Corollary
	CopyStyle	Theorem
	LabelCounter	corollary
	LatexName	corollary
	LabelString	"Corollary \thecorollary."
	LangPreamble
	  \providecommand{\corollaryname}{_(Corollary)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\corollaryname}{_(Corollary)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='corollary'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		lemma
	GuiName Lemma
End
Style Lemma
	CopyStyle	Theorem
	LabelCounter	lemma
	LatexName	lemma
	LabelString	"Lemma \thelemma."
	LangPreamble
	  \providecommand{\lemmaname}{_(Lemma)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\lemmaname}{_(Lemma)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='lemma'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		proposition
	GuiName Proposition
End
Style Proposition
	CopyStyle	Theorem
	LabelCounter	proposition
	LatexName	proposition
	LabelString	"Proposition \theproposition."
	LangPreamble
	  \providecommand{\propositionname}{_(Proposition)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\propositionname}{_(Proposition)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='proposition'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		prop
	GuiName Prop(osition)
End
Style Prop
	CopyStyle	Theorem
	LabelCounter	prop
	LatexName	prop
	LabelString	"Prop \theprop."
	Font
	  Shape		Up
	EndFont
	LabelFont
	  Series	Medium
	EndFont
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='proposition'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		conjecture
	GuiName Conjecture
End
Style Conjecture
	CopyStyle	Theorem
	LabelCounter	conjecture
	LatexName	conjecture
	LabelString	"Conjecture \theconjecture."
	Font
	  Shape		Up
	EndFont
	LabelFont
	  Series	Medium
	  Shape		Italic
	EndFont
	LangPreamble
	  \providecommand{\conjecturename}{_(Conjecture)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\conjecturename}{_(Conjecture)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='conjecture'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		fact
	GuiName Fact
End
Style Fact
	CopyStyle	Theorem
	LabelCounter	fact
	LatexName	fact
	LabelString	"Fact \thefact."
	LangPreamble
	  \providecommand{\factname}{_(Fact)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\factname}{_(Fact)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='fact'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		definition
	GuiName Definition
End
Style Definition
	CopyStyle	Theorem
	LabelCounter	definition
	LatexName	definition
	LabelString	"Definition \thedefinition."
	RefPrefix	def
	Font
	  Shape		Up
	EndFont
	LabelFont
	  Shape		Up
	  Series	Bold
	EndFont
	LangPreamble
	  \providecommand{\definitionname}{_(Definition)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\definitionname}{_(Definition)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='definition'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		example
	GuiName Example
End
Style Example
	CopyStyle	Conjecture
	LabelCounter	example
	LatexName	example
	LabelString	"Example \theexample."
	RefPrefix	exa
	LangPreamble
	  \providecommand{\examplename}{_(Example)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\examplename}{_(Example)}}
	EndBabelPreamble
	DocBookWrapperTag     informalexample
	DocBookTag            para
	DocBookAttr           ""
End


Counter		problem
	GuiName Problem
End
Style Problem
	CopyStyle	Definition
	Category	Exercise
	LabelCounter	problem
	LatexName	problem
	LabelString	"Problem \theproblem."
	RefPrefix	prob
	LangPreamble
	  \providecommand{\problemname}{_(Problem)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\problemname}{_(Problem)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='problem'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		prob
	GuiName Prob(lem)
  Within	chapter
End
# Prob does not copy a style to avoid preamble code
Style Prob
	Category	Exercise
	LatexType	Environment
	LatexName	prob
	Margin		First_Dynamic
	LabelType	Static
	LabelCounter	prob
	LabelString	"\theprob."
	RefPrefix	pro
	NextNoIndent	1
	ResetArgs       1
	LabelSep	xx
	ParIndent	MMM
	ParSkip		0.4
	ItemSep		0.2
	TopSep		0.7
	BottomSep	0.7
	ParSep		0.3
	Align		Block
	AlignPossible	Left
	LabelFont
	  Series	Bold
	EndFont
	AddToToc	thm
	IsTocCaption	true
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='problem'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		solution
	GuiName Solution
End
Style Solution
	CopyStyle	Problem
	LabelCounter	solution
	LatexName	solution
	LabelString	"Solution \thesolution."
	RefPrefix	solu
	LangPreamble
	  \providecommand{\solutionname}{_(Solution)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\solutionname}{_(Solution)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='solution'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Sol
	CopyStyle	Prob
	LatexName	sol
	LabelType	Static
	LabelString	"# [number of Prob]"
	Argument 1
	  Mandatory	1
	  LabelString	"Label of Problem"
	  Tooltip	"Label of the corresponding problem"
	  IsTocCaption true
	EndArgument
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='solution'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		exercise
	GuiName Exercise
End
Style Exercise
	CopyStyle	Definition
	LabelCounter	exercise
	LatexName	exercise
	LabelString	"Exercise \theexercise."
	RefPrefix	exer
	LangPreamble
	  \providecommand{\exercisename}{_(Exercise)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\exercisename}{_(Exercise)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='exercise'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		remark
	GuiName Remark
End
Style Remark
	CopyStyle	Conjecture
	LabelCounter	remark
	LatexName	remark
	LabelString	"Remark \theremark."
	RefPrefix	rem
	LangPreamble
	  \providecommand{\remarkname}{_(Remark)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\remarkname}{_(Remark)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='remark'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style		Claim
	CopyStyle	Remark
	LatexName	claim
	LabelString	"Claim"
	LangPreamble
	  \providecommand{\claimname}{_(Claim)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\claimname}{_(Claim)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='claim'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		case
	GuiName Case
End
Style Case
	CopyStyle	Remark
	LabelCounter	case
	LatexName	case
	LabelString	"Case \thecase."
	RefPrefix	case
	LangPreamble
	  \providecommand{\casename}{_(Case)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\casename}{_(Case)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='case'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		property
	GuiName Property
End
Style Property
	CopyStyle	Claim
	LabelCounter	property
	LatexName	property
	LabelString	"Property \theproperty."
	RefPrefix	prop
	LangPreamble
	  \providecommand{\propertyname}{_(Property)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\propertyname}{_(Property)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='property'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		question
	GuiName Question
End
Style Question
	CopyStyle	Claim
	LabelCounter	question
	LatexName	question
	LabelString	"Question \thequestion."
	RefPrefix	que
	LangPreamble
	  \providecommand{\questionname}{_(Question)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\questionname}{_(Question)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='question'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Counter		note
	GuiName Note
End
Style Note
	CopyStyle	Claim
	LabelCounter	note
	LatexName	note
	LabelString	"Note \thenote."
	RefPrefix	note
	LangPreamble
	  \providecommand{\notename}{_(Note)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\notename}{_(Note)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='note'
	DocBookTag            para
	DocBookGenerateTitle  true
End

