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

Format 6

Style Theorem*
	CopyStyle             Theorem
	LatexName             thm*
	LabelType             Static
	LabelString           "Theorem."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{thm*}{Theorem}
	EndPreamble
End


Style Corollary*
	CopyStyle             Theorem*
	LatexName             cor*
	LabelString           "Corollary."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{cor*}{Corollary}
	EndPreamble
End


Style Lemma*
	CopyStyle             Theorem*
	LatexName             lem*
	LabelString           "Lemma."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{lem*}{Lemma}
	EndPreamble
End


Style Proposition*
	CopyStyle             Theorem*
	LatexName             prop*
	LabelString           "Proposition."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{prop*}{Proposition}
	EndPreamble
End


Style Conjecture*
	CopyStyle             Theorem*
	LatexName             conjecture*
	LabelString           "Conjecture."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{conjecture*}{Conjecture}
	EndPreamble
End


Style Fact*
	CopyStyle             Theorem*
	LatexName             fact*
	LabelString           "Fact."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{fact*}{Fact}
	EndPreamble
End


Style Definition*
	CopyStyle             Definition
	LatexName             defn*
	LabelType             Static
	LabelString           "Definition."
	Preamble
	 \theoremstyle{definition}
	 \newtheorem*{defn*}{Definition}
	EndPreamble
End


Style Example*
	CopyStyle             Definition*
	LatexName             example*
	LabelString           "Example."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem*{example*}{Example}
	EndPreamble
End


Style Problem*
	CopyStyle             Definition*
	LatexName             problem*
	LabelString           "Problem."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem*{problem*}{Problem}
	EndPreamble
End


Style Exercise*
	CopyStyle             Definition*
	LatexName             xca*
	LabelString           "Exercise."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem*{xca*}{Exercise}
	EndPreamble
End


Style Remark*
	CopyStyle             Remark
	LatexName             rem*
	LabelType             Static
	LabelString           "Remark."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{rem*}{Remark}
	EndPreamble
End


Style Claim*
	CopyStyle             Remark*
	LatexName             claim*
	LabelString           "Claim."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{claim*}{Claim}
	EndPreamble
End

