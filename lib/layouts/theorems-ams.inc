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

Format 9

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
		\newtheorem{thm}{Theorem}
	EndPreamble
	Requires              amsthm
End


Style Corollary
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             cor
	LabelString           "Corollary \thetheorem."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{cor}[thm]{Corollary}
	EndPreamble
End


Style Lemma
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             lem
	LabelString           "Lemma \thetheorem."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{lem}[thm]{Lemma}
	EndPreamble
End


Style Proposition
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             prop
	LabelString           "Proposition \thetheorem."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{prop}[thm]{Proposition}
	EndPreamble
End


Style Conjecture
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             conjecture
	LabelString           "Conjecture \thetheorem."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{conjecture}[thm]{Conjecture}
	EndPreamble
End


Style Fact
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             fact
	LabelString           "Fact \thetheorem."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{fact}[thm]{Fact}
	EndPreamble
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
	  \newtheorem{defn}[thm]{Definition}
	EndPreamble
End


Style Example
	CopyStyle             Definition
	LatexName             example
	LabelString           "Example \thetheorem."
	Preamble
	 \theoremstyle{definition}
	  \newtheorem{example}[thm]{Example}
	EndPreamble
	Requires              amsthm
End


Style Problem
	CopyStyle             Definition
	LatexName             problem
	LabelString           "Problem \thetheorem."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem{problem}[thm]{Problem}
	EndPreamble
	Requires              amsthm
End


Style Exercise
	CopyStyle             Definition
	LatexName             xca
	LabelString           "Exercise \thetheorem."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem{xca}[thm]{Exercise}
	EndPreamble
	Requires              amsthm
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
	  \newtheorem{rem}[thm]{Remark}
	EndPreamble
End


Style Claim
	CopyStyle             Remark
	LatexName             claim
	LabelString           "Claim \thetheorem."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{claim}[thm]{Claim}
	EndPreamble
	Requires              amsthm
End


# Define Case as an enumeration environment
Style Case
	Category              Theorem
	CopyStyle             Enumerate
	LatexName             caseenv
	LabelType             Static
	LeftMargin            "Case ###."
	LabelString           "Case #."
	LabelFont
	  Series              Medium
	  Shape               Italic
	EndFont
	Preamble
	  \newcounter{casectr}
	  \newenvironment{caseenv}
	  {\begin{list}{{\itshape\ Case} \arabic{casectr}.}{%
	   \setlength{\leftmargin}{\labelwidth}
	   \addtolength{\leftmargin}{\parskip}
	   \setlength{\itemindent}{\listparindent}
	   \setlength{\itemsep}{\medskipamount}
	   \setlength{\topsep}{\itemsep}}
	   \setcounter{casectr}{0}
	   \usecounter{casectr}}
	  {\end{list}}
	EndPreamble
End
