# Original Author : David L. Johnson <dlj0@lehigh.edu>
# Probably broken by Jean-Marc Lasgouttes <Jean-Marc.Lasgouttes@inria.fr>
# modified and modularized by Emmanuel GUREGHIAN <gureghia@boston.bertin.fr>
# Tinkered with Sep. '07 by Paul Rubin <rubin@msu.edu>
# Modularized Jan 08 by Richard Heck <rgheck@comcast.net>

# The starred environnements defined are :
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

Format 8

Style Theorem*
	Category              Theorem
	LatexType             Environment
	Margin                First_Dynamic
	LatexName             thm*
	LabelType             Static
	LabelString           "Theorem."
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
	  \newtheorem*{thm*}{Theorem}
	EndPreamble
	Requires              amsthm
End


Style Corollary*
	CopyStyle             Theorem*
	LatexName             cor*
	LabelString           "Corollary."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{cor*}{Corollary}
	EndPreamble
	Requires              amsthm
End


Style Lemma*
	CopyStyle             Theorem*
	LatexName             lem*
	LabelString           "Lemma."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{lem*}{Lemma}
	EndPreamble
	Requires              amsthm
End


Style Proposition*
	CopyStyle             Theorem*
	LatexName             prop*
	LabelString           "Proposition."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{prop*}{Proposition}
	EndPreamble
	Requires              amsthm
End


Style Conjecture*
	CopyStyle             Theorem*
	LatexName             conjecture*
	LabelString           "Conjecture."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{conjecture*}{Conjecture}
	EndPreamble
	Requires              amsthm
End


Style Fact*
	CopyStyle             Theorem*
	LatexName             fact*
	LabelString           "Fact."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{fact*}{Fact}
	EndPreamble
	Requires              amsthm
End


Style Definition*
	CopyStyle             Theorem*
	LatexName             defn*
#LabelType             Static
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
	 \newtheorem*{defn*}{Definition}
	EndPreamble
	Requires              amsthm
End


Style Example*
	CopyStyle             Definition*
	LatexName             example*
	LabelString           "Example."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem*{example*}{Example}
	EndPreamble
	Requires              amsthm
End


Style Problem*
	CopyStyle             Definition*
	LatexName             problem*
	LabelString           "Problem."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem*{problem*}{Problem}
	EndPreamble
	Requires              amsthm
End


Style Exercise*
	CopyStyle             Definition*
	LatexName             xca*
	LabelString           "Exercise."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem*{xca*}{Exercise}
	EndPreamble
	Requires              amsthm
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
	  \newtheorem*{rem*}{Remark}
	EndPreamble
	Requires              amsthm
End


Style Claim*
	CopyStyle             Remark*
	LatexName             claim*
	LabelString           "Claim."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{claim*}{Claim}
	EndPreamble
	Requires              amsthm
End

