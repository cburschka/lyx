# Author : David L. Johnson <dlj0@lehigh.edu>
# Probably broken by Jean-Marc Lasgouttes <Jean-Marc.Lasgouttes@inria.fr>
# modified and modularized by Emmanuel GUREGHIAN <gureghia@boston.bertin.fr>
# Tinkered with Sep. '07 by Paul Rubin <rubin@msu.edu>


# These are only the plain (unnumbered) theorem styles environments.
# Both starred and unstarred versions are included for compatibility
# with the other AMS classes.
# The environments defined are:
# - Theorem
# - Corollary
# - Lemma
# - Proposition
# - Conjecture
# - Criterion
# - Algorithm
# - Fact
# - Axiom
# - Definition
# - Example
# - Condition
# - Problem
# - Exercise
# - Remark
# - Claim
# - Note
# - Notation
# - Summary
# - Acknowledgement
# - Conclusion
# - Assumption
# - Case (regular only -- defined as an enumeration)


Format 4

Style Theorem
	LatexName             thmm*
	LabelString           "Theorem."
	Preamble
	  \@ifundefined{theoremstyle}{\usepackage{amsthm}}{}
	  \theoremstyle{plain}
	  \newtheorem*{thmm*}{Theorem}
	EndPreamble
End


Style Corollary
	LatexName             cor*
	LabelString           "Corollary."
	DependsOn             Corollary*
	Preamble
	EndPreamble
End


Style Lemma
	LatexName             lem*
	LabelString           "Lemma."
	DependsOn             Lemma*
	Preamble
	EndPreamble
End


Style Proposition
	LatexName             prop*
	LabelString           "Proposition."
	DependsOn             Proposition*
	Preamble
	EndPreamble
End


Style Conjecture
	LatexName             conjecture*
	LabelString           "Conjecture."
	DependsOn             Conjecture*
	Preamble
	EndPreamble
End


Style Criterion
	LatexName             criterion*
	LabelString           "Criterion."
	DependsOn             Criterion*
	Preamble
	EndPreamble
End


Style Algorithm
	LatexName             algorithm*
	LabelString           "Algorithm."
	DependsOn             Algorithm*
	Preamble
	EndPreamble
End


Style Fact
	LatexName             fact*
	LabelString           "Fact."
	DependsOn             Fact*
	Preamble
	EndPreamble
End


Style Axiom
	LatexName             ax*
	LabelString           "Axiom."
	DependsOn             Axiom*
	Preamble
	EndPreamble
End


Style Definition
	LatexName             defn*
	LabelString           "Definition."
	DependsOn             Definition*
	Preamble
	EndPreamble
End


Style Example
	LatexName             example*
	LabelString           "Example."
	DependsOn             Example*
	Preamble
	EndPreamble
End


Style Condition
	LatexName             condition*
	LabelString           "Condition."
	DependsOn             Condition*
	Preamble
	EndPreamble
End


Style Problem
	LatexName             problem*
	LabelString           "Problem."
	DependsOn             Problem*
	Preamble
	EndPreamble
End


Style Exercise
	LatexName             xca*
	LabelString           "Exercise."
	DependsOn             Exercise*
	Preamble
	EndPreamble
End


Style Remark
	LatexName             rem*
	LabelString           "Remark."
	DependsOn             Remark*
	Preamble
	EndPreamble
End


Style Claim
	LatexName             claim*
	LabelString           "Claim."
	DependsOn             Claim*
	Preamble
	EndPreamble
End


Style Note
	LatexName             note*
	LabelString           "Note."
	DependsOn             Note*
	Preamble
	EndPreamble
End


Style Notation
	LatexName             notation*
	LabelString           "Notation."
	DependsOn             Notation*
	Preamble
	EndPreamble
End


Style Summary
	LatexName             summary*
	LabelString           "Summary."
	DependsOn             Summary*
	Preamble
	EndPreamble
End


Style Acknowledgement
	LatexName             acknowledgement*
	LabelString           "Acknowledgement."
	DependsOn             Acknowledgement*
	Preamble
	EndPreamble
End


Style Conclusion
	LatexName             conclusion*
	LabelString           "Conclusion."
	DependsOn             Conclusion*
	Preamble
	EndPreamble
End


Style Assumption
	LatexName             assumption*
	LabelString           "Assumption."
	DependsOn             Assumption*
	Preamble
	EndPreamble
End
