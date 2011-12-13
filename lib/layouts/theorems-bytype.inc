# Original Author : David L. Johnson <dlj0@lehigh.edu>
# Probably broken by Jean-Marc Lasgouttes <Jean-Marc.Lasgouttes@inria.fr>
# modified and modularized by Emmanuel GUREGHIAN <gureghia@boston.bertin.fr>
# Tinkered with Sep. '07 by Paul Rubin <rubin@msu.edu>
# Modularized Jan 08 by Richard Heck <rgheck@comcast.net>
# Hacked June '09 by Paul Rubin <rubin@msu.edu> to use separate counters

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
# - Remark
# - Claim
# - Proof
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
	LatexName             cor
	LabelString           "Corollary \thecorollary."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{cor}{\protect\corollaryname}
	EndPreamble
	LangPreamble
		\providecommand{\corollaryname}{_(Corollary)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\corollaryname}{_(Corollary)}}
	EndBabelPreamble
	LabelCounter          corollary
End


Style Lemma
	CopyStyle             Theorem
	LatexName             lem
	LabelString           "Lemma \thelemma."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{lem}{\protect\lemmaname}
	EndPreamble
	LangPreamble
		\providecommand{\lemmaname}{_(Lemma)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\lemmaname}{_(Lemma)}}
	EndBabelPreamble
	LabelCounter          lemma
End


Style Proposition
	CopyStyle             Theorem
	LatexName             prop
	LabelString           "Proposition \theproposition."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{prop}{\protect\propositionname}
	EndPreamble
	LangPreamble
		\providecommand{\propositionname}{_(Proposition)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\propositionname}{_(Proposition)}}
	EndBabelPreamble
	LabelCounter          proposition
End


Style Conjecture
	CopyStyle             Theorem
	LatexName             conjecture
	LabelString           "Conjecture \theconjecture."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{conjecture}{\protect\conjecturename}
	EndPreamble
	LangPreamble
		\providecommand{\conjecturename}{_(Conjecture)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\conjecturename}{_(Conjecture)}}
	EndBabelPreamble
	LabelCounter          conjecture
End


Style Fact
	CopyStyle             Theorem
	LatexName             fact
	LabelString           "Fact \thefact."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{fact}{\protect\factname}
	EndPreamble
	LangPreamble
		\providecommand{\factname}{_(Fact)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\factname}{_(Fact)}}
	EndBabelPreamble
	LabelCounter          fact
End


Style Definition
	CopyStyle             Theorem
	LatexName             defn
	LabelString           "Definition \thedefinition."
	Font
	  Shape               Up
	EndFont
	LabelFont
	  Shape               Up
	  Series              Bold
	EndFont
	Preamble
	  \theoremstyle{definition}
	  \newtheorem{defn}{\protect\definitionname}
	EndPreamble
	LangPreamble
		\providecommand{\definitionname}{_(Definition)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\definitionname}{_(Definition)}}
	EndBabelPreamble
	LabelCounter          definition  
End


Style Example
	CopyStyle             Definition
	LatexName             example
	LabelString           "Example \theexample."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem{example}{\protect\examplename}
	EndPreamble
	Requires              amsthm
	LangPreamble
		\providecommand{\examplename}{_(Example)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\examplename}{_(Example)}}
	EndBabelPreamble
	LabelCounter          example
End


Style Problem
	CopyStyle             Definition
	LatexName             problem
	LabelString           "Problem \theproblem."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem{problem}{\protect\problemname}
	EndPreamble
	Requires              amsthm
	LangPreamble
		\providecommand{\problemname}{_(Problem)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\problemname}{_(Problem)}}
	EndBabelPreamble
	LabelCounter          problem
End


Style Exercise
	CopyStyle             Definition
	LatexName             xca
	LabelString           "Exercise \theexercise."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem{xca}{\protect\exercisename}
	EndPreamble
	Requires              amsthm
	LangPreamble
		\providecommand{\exercisename}{_(Exercise)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\exercisename}{_(Exercise)}}
	EndBabelPreamble
	LabelCounter          exercise
End


Style Remark
	CopyStyle             Theorem
	LatexName             rem
	LabelString           "Remark \theremark."
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
	  \newtheorem{rem}{\protect\remarkname}
	EndPreamble
	LangPreamble
		\providecommand{\remarkname}{_(Remark)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\remarkname}{_(Remark)}}
	EndBabelPreamble
	LabelCounter          remark
End


Style Claim
	CopyStyle             Remark
	LatexName             claim
	LabelString           "Claim \theclaim."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{claim}{\protect\claimname}
	EndPreamble
	Requires              amsthm
	LangPreamble
		\providecommand{\claimname}{_(Claim)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\claimname}{_(Claim)}}
	EndBabelPreamble
	LabelCounter          claim
End

Input theorems-case.inc
Input theorems-refprefix.inc
