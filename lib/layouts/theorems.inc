# Original Author : David L. Johnson <dlj0@lehigh.edu>
# Probably broken by Jean-Marc Lasgouttes <Jean-Marc.Lasgouttes@inria.fr>
# modified and modularized by Emmanuel GUREGHIAN <gureghia@boston.bertin.fr>
# Tinkered with Sep. '07 by Paul Rubin <rubin@msu.edu>
# Modularized Jan 08 by Richard Kimberly Heck <rikiheck@lyx.org>
# Edited sep '22 by Udi Fogiel <udifoglle@gmail.com>

# The environments defined are :
# - Theorem
# - Corollary
# - Lemma
# - Proposition
# - Conjecture
# - Fact
# - Definition
# - Example
# - Problem
# - Exercise
# - Solution
# - Remark
# - Claim
# - Case (by inclusion)

Format 99

OutlinerName thm "Definitions & Theorems"

Style Theorem
	Category              Reasoning
	DependsOn             Udi
	Margin                First_Dynamic
	LatexType             Environment
	LatexName             thm
	NextNoIndent          0
	ResetArgs             1
	AddToToc              thm
	IsTocCaption          1
	Argument 1
		LabelString    "Additional Theorem Text"
		Tooltip        "Additional text appended to the theorem header"
		IsTocCaption      1
	EndArgument
	LabelSep              xx
	ParIndent             MMM
	ParSkip               0.4
	ItemSep               0.2
	TopSep                0.7
	BottomSep             0.7
	ParSep                0.3
	Align                 Block
	AlignPossible         Left
	LabelType             Static
	LabelCounter          theorem
	LabelString           "Theorem \thetheorem."
	Font
	    Shape             Italic
	    Size              Normal
	EndFont
	LabelFont
	    Shape             Up
	    Series            Bold
	EndFont
	Preamble
		\newcommand\lyx@reg@newtheorem[2]{\newtheorem{#1}[thm]{#2}}
		\newcommand\lyx@newtheorem{\@ifstar{\newtheorem*}{\lyx@reg@newtheorem}}
		\theoremstyle{plain}
		\newtheorem{thm}{\protect\theoremname}
	EndPreamble
	LangPreamble
		\providecommand{\theoremname}{_(Theorem)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\theoremname}{_(Theorem)}}
	EndBabelPreamble
	Requires              amsthm
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='theorem'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Theorem*
	DependsOn             Theorem
	Category              Reasoning
	LatexType             Environment
	Margin                First_Dynamic
	LatexName             thm*
	LabelType             Static
	LabelString           "Theorem."
	NextNoIndent          1
	ResetArgs             1
	AddToToc              thm
	IsTocCaption          1
	Argument 1
		LabelString       "Additional Theorem Text"
		Tooltip           "Additional text appended to the theorem header"
		IsTocCaption      1
	EndArgument
	LabelSep              xx
	ParIndent             MMM
	ParSkip               0.4
	ItemSep               0.2
	TopSep                0.7
	BottomSep             0.7
	ParSep                0.3
	Align                 Block
	AlignPossible         Left
	Font
	    Shape             Italic
	    Size              Normal
	EndFont
	LabelFont
	    Shape             Up
	    Series            Bold
	EndFont
	Preamble
	    \theoremstyle{plain}
	    \lyx@newtheorem*{thm*}{\protect\theoremname}
	EndPreamble
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


Style Corollary
	CopyStyle             Theorem
	DependsOn             Theorem
	LatexName             cor
	LabelString           "Corollary \thetheorem."
	Preamble
		\theoremstyle{plain}
		\lyx@newtheorem{cor}{\protect\corollaryname}
	EndPreamble
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


Style Corollary*
	CopyStyle             Theorem*
	LatexName             cor*
	LabelString           "Corollary."
	Preamble
	    \theoremstyle{plain}
	    \lyx@newtheorem*{cor*}{\protect\corollaryname}
	EndPreamble
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


Style Lemma
	CopyStyle             Theorem
	DependsOn             Theorem
	LatexName             lem
	LabelString           "Lemma \thetheorem."
	Preamble
		\theoremstyle{plain}
		\lyx@newtheorem{lem}{\protect\lemmaname}
	EndPreamble
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


Style Lemma*
	CopyStyle             Theorem*
	LatexName             lem*
	LabelString           "Lemma."
	Preamble
	    \theoremstyle{plain}
	    \lyx@newtheorem*{lem*}{\protect\lemmaname}
	EndPreamble
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


Style Proposition
	CopyStyle             Theorem
	DependsOn             Theorem
	LatexName             prop
	LabelString           "Proposition \thetheorem."
	Preamble
		\theoremstyle{plain}
		\lyx@newtheorem{prop}{\protect\propositionname}
	EndPreamble
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


Style Proposition*
	CopyStyle             Theorem*
	LatexName             prop*
	LabelString           "Proposition."
	Preamble
	    \theoremstyle{plain}
	    \lyx@newtheorem*{prop*}{\protect\propositionname}
	EndPreamble
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


Style Conjecture
	CopyStyle             Theorem
	DependsOn             Theorem
	LatexName             conjecture
	LabelString           "Conjecture \thetheorem."
	Preamble
		\theoremstyle{plain}
		\lyx@newtheorem{conjecture}{\protect\conjecturename}
	EndPreamble
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


Style Conjecture*
	CopyStyle             Theorem*
	LatexName             conjecture*
	LabelString           "Conjecture."
	Preamble
	    \theoremstyle{plain}
	    \lyx@newtheorem*{conjecture*}{\protect\conjecturename}
	EndPreamble
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


Style Fact
	CopyStyle             Theorem
	DependsOn             Theorem
	LatexName             fact
	LabelString           "Fact \thetheorem."
	Preamble
		\theoremstyle{plain}
		\lyx@newtheorem{fact}{\protect\factname}
	EndPreamble
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


Style Fact*
	CopyStyle             Theorem*
	LatexName             fact*
	LabelString           "Fact."
	Preamble
	    \theoremstyle{plain}
	    \lyx@newtheorem*{fact*}{\protect\factname}
	EndPreamble
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


Style Definition
	CopyStyle             Theorem
	DependsOn             Theorem
	LatexName             defn
	LabelString           "Definition \thetheorem."
	Font
	  Shape               Up
	EndFont
	LabelFont
	  Shape               Up
	  Series              Bold
	EndFont
	Preamble
		\theoremstyle{definition}
		\lyx@newtheorem{defn}{\protect\definitionname}
	EndPreamble
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


Style Definition*
	CopyStyle             Theorem*
	LatexName             defn*
	LabelString           "Definition."
	Font
	    Shape               Up
	EndFont
	LabelFont
	    Shape               Up
	    Series              Bold
	EndFont
	Preamble
	    \theoremstyle{definition}
	    \lyx@newtheorem*{defn*}{\protect\definitionname}
	EndPreamble
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


Style Example
	CopyStyle             Definition
	LatexName             example
	LabelString           "Example \thetheorem."
	Preamble
		\theoremstyle{definition}
		\lyx@newtheorem{example}{\protect\examplename}
	EndPreamble
	LangPreamble
		\providecommand{\examplename}{_(Example)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\examplename}{_(Example)}}
	EndBabelPreamble
	DocBookWrapperTag     example
	DocBookTag            para
	DocBookAttr           ""
	DocBookGenerateTitle  true
End


Style Example*
	CopyStyle             Definition*
	LatexName             example*
	LabelString           "Example."
	Preamble
	    \theoremstyle{definition}
	    \lyx@newtheorem*{example*}{\protect\examplename}
	EndPreamble
	LangPreamble
	    \providecommand{\examplename}{_(Example)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\examplename}{_(Example)}}
	EndBabelPreamble
	DocBookWrapperTag     informalexample
	DocBookTag            para
	DocBookAttr           ""
	DocBookGenerateTitle  true
End


Style Problem
	CopyStyle             Definition
	LatexName             problem
	LabelString           "Problem \thetheorem."
	Preamble
		\theoremstyle{definition}
		\lyx@newtheorem{problem}{\protect\problemname}
	EndPreamble
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


Style Problem*
	CopyStyle             Definition*
	LatexName             problem*
	LabelString           "Problem."
	Preamble
	    \theoremstyle{definition}
	    \lyx@newtheorem*{problem*}{\protect\problemname}
	EndPreamble
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


Style Exercise
	CopyStyle             Definition
	LatexName             xca
	LabelString           "Exercise \thetheorem."
	Preamble
		\theoremstyle{definition}
		\lyx@newtheorem{xca}{\protect\exercisename}
	EndPreamble
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


Style Exercise*
	CopyStyle             Definition*
	LatexName             xca*
	LabelString           "Exercise."
	Preamble
	    \theoremstyle{definition}
	    \lyx@newtheorem*{xca*}{\protect\exercisename}
	EndPreamble
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


Style Solution
	CopyStyle             Definition
	LatexName             sol
	LabelString           "Solution \thetheorem."
	Preamble
		\theoremstyle{definition}
		\lyx@newtheorem{sol}{\protect\solutionname}
	EndPreamble
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


Style Solution*
	CopyStyle             Definition*
	LatexName             sol*
	LabelString           "Solution."
	Preamble
	    \theoremstyle{definition}
	    \lyx@newtheorem*{sol*}{\protect\solutionname}
	EndPreamble
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


Style Remark
	CopyStyle             Theorem
	DependsOn             Theorem
	LatexName             rem
	LabelString           "Remark \thetheorem."
	Font
	    Shape             Up
	    Size              Normal
	EndFont
	LabelFont
	    Series            Medium
	    Shape             Italic
	EndFont
	Preamble
		\theoremstyle{remark}
		\lyx@newtheorem{rem}{\protect\remarkname}
	EndPreamble
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


Style Remark*
	CopyStyle             Theorem*
	LatexName             rem*
	LabelString           "Remark."
	Font
	    Shape               Up
	    Size                Normal
	EndFont
	LabelFont
	    Series              Medium
	    Shape               Italic
	EndFont
	Preamble
	    \theoremstyle{remark}
	    \lyx@newtheorem*{rem*}{\protect\remarkname}
	EndPreamble
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


Style Claim
	CopyStyle             Remark
	LatexName             claim
	LabelString           "Claim \thetheorem."
	Preamble
		\theoremstyle{remark}
		\lyx@newtheorem{claim}{\protect\claimname}
	EndPreamble
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


Style Claim*
	CopyStyle             Remark*
	LatexName             claim*
	LabelString           "Claim."
	Preamble
	    \theoremstyle{remark}
	    \lyx@newtheorem*{claim*}{\protect\claimname}
	EndPreamble
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


Input theorems-case.inc
Input theorems-refprefix.inc
