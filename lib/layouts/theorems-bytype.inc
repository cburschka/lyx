# Original Author : David L. Johnson <dlj0@lehigh.edu>
# Probably broken by Jean-Marc Lasgouttes <Jean-Marc.Lasgouttes@inria.fr>
# modified and modularized by Emmanuel GUREGHIAN <gureghia@boston.bertin.fr>
# Tinkered with Sep. '07 by Paul Rubin <rubin@msu.edu>
# Modularized Jan 08 by Richard Heck <rgheck@comcast.net>
# Hacked June '09 by Paul Rubin <rubin@msu.edu> to use separate counters

# The environnements defined are :
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
# - Case (regular only -- defined as an enumeration)

Format 27

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
	LatexName             cor
	LabelString           "Corollary \thecorollary."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{cor}{Corollary}
	EndPreamble
	LabelCounter          corollary
End


Style Lemma
	CopyStyle             Theorem
	LatexName             lem
	LabelString           "Lemma \thelemma."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{lem}{Lemma}
	EndPreamble
	LabelCounter          lemma
End


Style Proposition
	CopyStyle             Theorem
	LatexName             prop
	LabelString           "Proposition \theproposition."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{prop}{Proposition}
	EndPreamble
	LabelCounter          proposition
End


Style Conjecture
	CopyStyle             Theorem
	LatexName             conjecture
	LabelString           "Conjecture \theconjecture."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{conjecture}{Conjecture}
	EndPreamble
	LabelCounter          conjecture
End


Style Fact
	CopyStyle             Theorem
	LatexName             fact
	LabelString           "Fact \thefact."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{fact}{Fact}
	EndPreamble
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
	  \newtheorem{defn}{Definition}
	EndPreamble
	LabelCounter          definition  
End


Style Example
	CopyStyle             Definition
	LatexName             example
	LabelString           "Example \theexample."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem{example}{Example}
	EndPreamble
	Requires              amsthm
	LabelCounter          example
End


Style Problem
	CopyStyle             Definition
	LatexName             problem
	LabelString           "Problem \theproblem."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem{problem}{Problem}
	EndPreamble
	Requires              amsthm
	LabelCounter          problem
End


Style Exercise
	CopyStyle             Definition
	LatexName             xca
	LabelString           "Exercise \theexercise."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem{xca}{Exercise}
	EndPreamble
	Requires              amsthm
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
	  \newtheorem{rem}{Remark}
	EndPreamble
	LabelCounter          remark
End


Style Claim
	CopyStyle             Remark
	LatexName             claim
	LabelString           "Claim \theclaim."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{claim}{Claim}
	EndPreamble
	Requires              amsthm
	LabelCounter          claim
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
	RefPrefix OFF
End

Input theorems-refprefix.inc
