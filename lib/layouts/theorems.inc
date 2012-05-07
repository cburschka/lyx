# Original Author : David L. Johnson <dlj0@lehigh.edu>
# Probably broken by Jean-Marc Lasgouttes <Jean-Marc.Lasgouttes@inria.fr>
# modified and modularized by Emmanuel GUREGHIAN <gureghia@boston.bertin.fr>
# Tinkered with Sep. '07 by Paul Rubin <rubin@msu.edu>
# Modularized Jan 08 by Richard Heck <rgheck@comcast.net>

# The environments defined are :
# - Theorem
# - Corollary
# - Lemma
# - Proposition
# - Conjecture
# - Definition
# - Example
# - Problem
# - Exercise
# - Remark
# - Claim
# - Case (by inclusion)

Format 35

Style Theorem
	Category              Theorem
	Margin                First_Dynamic
	LatexType             Environment
	LatexName             thm
	NextNoIndent          1
	OptionalArgs          1
	LabelSep              xx
	ParIndent             MMM
	ParSkip               0.4
	ItemSep               0.2
	TopSep                0.7
	BottomSep             0.7
	ParSep                0.3
	Align                 Block
	AlignPossible         Left
	LabelType             Counter
	LabelCounter          theorem
	LabelString           "Theorem \thetheorem."
	Font
	  Shape               Italic
	  Size                Normal
	EndFont
	LabelFont
	  Shape               Up
	  Series              Bold
	EndFont
	Preamble
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
End


Style Corollary
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             cor
	LabelString           "Corollary \thetheorem."
	Preamble
		\theoremstyle{plain}
		\newtheorem{cor}[thm]{\protect\corollaryname}
	EndPreamble
	LangPreamble
		\providecommand{\corollaryname}{_(Corollary)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\corollaryname}{_(Corollary)}}
	EndBabelPreamble
End


Style Lemma
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             lem
	LabelString           "Lemma \thetheorem."
	Preamble
		\theoremstyle{plain}
		\newtheorem{lem}[thm]{\protect\lemmaname}
	EndPreamble
	LangPreamble
		\providecommand{\lemmaname}{_(Lemma)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\lemmaname}{_(Lemma)}}
	EndBabelPreamble
End


Style Proposition
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             prop
	LabelString           "Proposition \thetheorem."
	Preamble
		\theoremstyle{plain}
		\newtheorem{prop}[thm]{\protect\propositionname}
	EndPreamble
	LangPreamble
		\providecommand{\propositionname}{_(Proposition)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\propositionname}{_(Proposition)}}
	EndBabelPreamble
End


Style Conjecture
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             conjecture
	LabelString           "Conjecture \thetheorem."
	Preamble
		\theoremstyle{plain}
		\newtheorem{conjecture}[thm]{\protect\conjecturename}
	EndPreamble
	LangPreamble
		\providecommand{\conjecturename}{_(Conjecture)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\conjecturename}{_(Conjecture)}}
	EndBabelPreamble
End


Style Fact
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             fact
	LabelString           "Fact \thetheorem."
	Preamble
		\theoremstyle{plain}
		\newtheorem{fact}[thm]{\protect\factname}
	EndPreamble
	LangPreamble
		\providecommand{\factname}{_(Fact)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\factname}{_(Fact)}}
	EndBabelPreamble
End


Style Definition
	CopyStyle             Theorem
	DependsOn	      Theorem
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
		\newtheorem{defn}[thm]{\protect\definitionname}
	EndPreamble
	LangPreamble
		\providecommand{\definitionname}{_(Definition)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\definitionname}{_(Definition)}}
	EndBabelPreamble
End


Style Example
	CopyStyle             Definition
	LatexName             example
	LabelString           "Example \thetheorem."
	Preamble
		\theoremstyle{definition}
		\newtheorem{example}[thm]{\protect\examplename}
	EndPreamble
	LangPreamble
		\providecommand{\examplename}{_(Example)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\examplename}{_(Example)}}
	EndBabelPreamble
End


Style Problem
	CopyStyle             Definition
	LatexName             problem
	LabelString           "Problem \thetheorem."
	Preamble
		\theoremstyle{definition}
		\newtheorem{problem}[thm]{\protect\problemname}
	EndPreamble
	LangPreamble
		\providecommand{\problemname}{_(Problem)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\problemname}{_(Problem)}}
	EndBabelPreamble
End


Style Exercise
	CopyStyle             Definition
	LatexName             xca
	LabelString           "Exercise \thetheorem."
	Preamble
		\theoremstyle{definition}
		\newtheorem{xca}[thm]{\protect\exercisename}
	EndPreamble
	LangPreamble
		\providecommand{\exercisename}{_(Exercise)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\exercisename}{_(Exercise)}}
	EndBabelPreamble
End


Style Remark
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             rem
	LabelString           "Remark \thetheorem."
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
		\newtheorem{rem}[thm]{\protect\remarkname}
	EndPreamble
	LangPreamble
		\providecommand{\remarkname}{_(Remark)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\remarkname}{_(Remark)}}
	EndBabelPreamble
End


Style Claim
	CopyStyle             Remark
	LatexName             claim
	LabelString           "Claim \thetheorem."
	Preamble
		\theoremstyle{remark}
		\newtheorem{claim}[thm]{\protect\claimname}
	EndPreamble
	LangPreamble
		\providecommand{\claimname}{_(Claim)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\claimname}{_(Claim)}}
	EndBabelPreamble
End

Input theorems-case.inc
Input theorems-refprefix.inc
