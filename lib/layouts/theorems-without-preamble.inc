# This file provides theorem environments without adding preamble code or
# requiring packages. THis is useful for classes that define their own theorems
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


Format 35


Counter		theorem
End
Style Theorem
	Category	Theorem
	Margin		First_Dynamic
	LatexType	Environment
	LatexName	theorem
	NextNoIndent	1
	OptionalArgs	1
	LabelSep	xx
	ParIndent	MMM
	ParSkip		0.4
	ItemSep		0.2
	TopSep		0.7
	BottomSep	0.7
	ParSep		0.3
	Align		Block
	AlignPossible	Left
	LabelType	Counter
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
End


Counter		corollary
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
End


Counter		lemma
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
End


Counter		proposition
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
End


Counter		prop
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
End


Counter		conjecture
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
End


Counter		fact
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
End


Counter		definition
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
End


Counter		example
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
End


Counter		problem
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
End


Counter		prob
  Within	chapter
End
# Prob does not copy a style to avoid preamble code
Style Prob
	Category	Exercise
	LatexType	Environment
	LatexName	prob
	Margin		First_Dynamic
	LabelType	Counter
	LabelCounter	prob
	LabelString	"\theprob."
	RefPrefix	pro
	NextNoIndent	1
	OptionalArgs	0
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
End


Counter		solution
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
End


Style Sol
	CopyStyle	Prob
	LatexName	sol
	LabelType	Static
	LabelString	"# [number of Prob]"
	RequiredArgs	1
End


Counter		exercise
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
End


Counter		remark
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
End


Counter		case
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
End


Counter		property
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
End


Counter		question
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
End


Counter		note
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
End

