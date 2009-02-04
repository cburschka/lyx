# Original Author : David L. Johnson <dlj0@lehigh.edu>
# Probably broken by Jean-Marc Lasgouttes <Jean-Marc.Lasgouttes@inria.fr>
# modified and modularized by Emmanuel GUREGHIAN <gureghia@boston.bertin.fr>
# Tinkered with Sep. '07 by Paul Rubin <rubin@msu.edu>
# Modularized Jan 08 by Richard Heck <rgheck@comcast.net>

# The environnements defined are :
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
# - Proof
# - Case (regular only -- defined as an enumeration)

Format 12

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
		\providecommand{\theoremname}{Theorem}
		\theoremstyle{plain}
		\newtheorem{thm}{\protect\theoremname}
	EndPreamble
	I18nPreamble
	  \AtBeginDocument{\ifx\LdfInit\undefined\relax\else\addto\captions$$lang{\renewcommand{\theoremname}{_(Theorem)}}\fi}
	EndI18nPreamble
	Requires              amsthm
End


Style Corollary
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             cor
	LabelString           "Corollary \thetheorem."
	Preamble
	  \providecommand{\corollaryname}{Corollary}
	  \theoremstyle{plain}
	  \newtheorem{cor}[thm]{\protect\corollaryname}
	EndPreamble
	I18nPreamble
	  \AtBeginDocument{\ifx\LdfInit\undefined\relax\else\addto\captions$$lang{\renewcommand{\corollaryname}{_(Corollary)}}\fi}
	EndI18nPreamble
End


Style Lemma
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             lem
	LabelString           "Lemma \thetheorem."
	Preamble
	  \providecommand{\lemmaname}{Lemma}
	  \theoremstyle{plain}
	  \newtheorem{lem}[thm]{\protect\lemmaname}
	EndPreamble
	I18nPreamble
	  \AtBeginDocument{\ifx\LdfInit\undefined\relax\else\addto\captions$$lang{\renewcommand{\lemmaname}{_(Lemma)}}\fi}
	EndI18nPreamble
End


Style Proposition
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             prop
	LabelString           "Proposition \thetheorem."
	Preamble
	  \providecommand{\propositionname}{Proposition}
	  \theoremstyle{plain}
	  \newtheorem{prop}[thm]{\protect\propositionname}
	EndPreamble
	I18nPreamble
	  \AtBeginDocument{\ifx\LdfInit\undefined\relax\else\addto\captions$$lang{\renewcommand{\propositionname}{_(Proposition)}}\fi}
	EndI18nPreamble
End


Style Conjecture
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             conjecture
	LabelString           "Conjecture \thetheorem."
	Preamble
	  \providecommand{\conjecturename}{Conjecture}
	  \theoremstyle{plain}
	  \newtheorem{conjecture}[thm]{\protect\conjecturename}
	EndPreamble
	I18nPreamble
	  \AtBeginDocument{\ifx\LdfInit\undefined\relax\else\addto\captions$$lang{\renewcommand{\conjecturename}{_(Conjecture)}}\fi}
	EndI18nPreamble
End


Style Fact
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             fact
	LabelString           "Fact \thetheorem."
	Preamble
	  \providecommand{\factname}{Fact}
	  \theoremstyle{plain}
	  \newtheorem{fact}[thm]{\protect\factname}
	EndPreamble
	I18nPreamble
	  \AtBeginDocument{\ifx\LdfInit\undefined\relax\else\addto\captions$$lang{\renewcommand{\factname}{_(Fact)}}\fi}
	EndI18nPreamble
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
	  \providecommand{\definitionname}{Definition}
	  \theoremstyle{definition}
	  \newtheorem{defn}[thm]{\protect\definitionname}
	EndPreamble
	I18nPreamble
	  \AtBeginDocument{\ifx\LdfInit\undefined\relax\else\addto\captions$$lang{\renewcommand{\definitionname}{_(Definition)}}\fi}
	EndI18nPreamble
End


Style Example
	CopyStyle             Definition
	LatexName             example
	LabelString           "Example \thetheorem."
	Preamble
	  \providecommand{\examplename}{Example}
	  \theoremstyle{definition}
	  \newtheorem{example}[thm]{\protect\examplename}
	EndPreamble
	I18nPreamble
	  \AtBeginDocument{\ifx\LdfInit\undefined\relax\else\addto\captions$$lang{\renewcommand{\examplename}{_(Example)}}\fi}
	EndI18nPreamble
End


Style Problem
	CopyStyle             Definition
	LatexName             problem
	LabelString           "Problem \thetheorem."
	Preamble
	  \providecommand{\problemname}{Problem}
	  \theoremstyle{definition}
	  \newtheorem{problem}[thm]{\protect\problemname}
	EndPreamble
	I18nPreamble
	  \AtBeginDocument{\ifx\LdfInit\undefined\relax\else\addto\captions$$lang{\renewcommand{\problemname}{_(Problem)}}\fi}
	EndI18nPreamble
End


Style Exercise
	CopyStyle             Definition
	LatexName             xca
	LabelString           "Exercise \thetheorem."
	Preamble
	  \providecommand{\exercisename}{Exercise}
	  \theoremstyle{definition}
	  \newtheorem{xca}[thm]{\protect\exercisename}
	EndPreamble
	I18nPreamble
	  \AtBeginDocument{\ifx\LdfInit\undefined\relax\else\addto\captions$$lang{\renewcommand{\exercisename}{_(Exercise)}}\fi}
	EndI18nPreamble
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
	  \providecommand{\remarkname}{Remark}
	  \theoremstyle{remark}
	  \newtheorem{rem}[thm]{\protect\remarkname}
	EndPreamble
	I18nPreamble
	  \AtBeginDocument{\ifx\LdfInit\undefined\relax\else\addto\captions$$lang{\renewcommand{\remarkname}{_(Remark)}}\fi}
	EndI18nPreamble
End


Style Claim
	CopyStyle             Remark
	LatexName             claim
	LabelString           "Claim \thetheorem."
	Preamble
	  \providecommand{\claimname}{Claim}
	  \theoremstyle{remark}
	  \newtheorem{claim}[thm]{\protect\claimname}
	EndPreamble
	I18nPreamble
	  \AtBeginDocument{\ifx\LdfInit\undefined\relax\else\addto\captions$$lang{\renewcommand{\claimname}{_(Claim)}}\fi}
	EndI18nPreamble
End


# Define Case as an enumeration environment
Style Case
	Category              Theorem
	Margin                Static
	LatexType             Item_Environment
	LatexName             caseenv
	LabelType             Static
	NextNoIndent          1
	LeftMargin            "Case ###."
	ParSkip               0.0
	ItemSep               0.2
	TopSep                0.7
	BottomSep             0.7
	ParSep                0.3
	Align                 Block
	AlignPossible         Block, Left
	LabelSep              xx
	LabelString           "Case #."
	LabelFont
	  Series              Medium
	  Shape               Italic
	EndFont
	Preamble
	  \providecommand{\casename}{Case}
	  \newcounter{casectr}
	  \newenvironment{caseenv}
	  {\begin{list}{{\itshape\ \protect\casename} \arabic{casectr}.}{%
	   \setlength{\leftmargin}{\labelwidth}
	   \addtolength{\leftmargin}{\parskip}
	   \setlength{\itemindent}{\listparindent}
	   \setlength{\itemsep}{\medskipamount}
	   \setlength{\topsep}{\itemsep}}
	   \setcounter{casectr}{0}
	   \usecounter{casectr}}
	  {\end{list}}
	EndPreamble
	I18nPreamble
	  \AtBeginDocument{\ifx\LdfInit\undefined\relax\else\addto\captions$$lang{\renewcommand{\casename}{_(Case)}}\fi}
	EndI18nPreamble
End
