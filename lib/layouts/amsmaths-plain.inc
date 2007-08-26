# Author : David L. Johnson <dlj0@lehigh.edu>
# Probably broken by Jean-Marc Lasgouttes <Jean-Marc.Lasgouttes@inria.fr>
# modified and modularized by Emmanuel GUREGHIAN <gureghia@boston.bertin.fr>


# These are only the plain (unnumbered) theorem styles environments
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
# - Case
# - Conclusion


Format 5
Style Theorem
	LatexName             thm*
	LabelString           "Theorem."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{thm*}{Theorem}
	EndPreamble
End


Style Corollary
	LatexName             cor*
	LabelString           "Corollary."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{cor*}{Corollary}
	EndPreamble
End


Style Lemma
	LatexName             lem*
	LabelString           "Lemma."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{lem*}{Lemma}
	EndPreamble
End


Style Proposition
	LatexName             prop*
	LabelString           "Proposition."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{prop*}{Proposition}
	EndPreamble
End


Style Conjecture
	LatexName             conjecture*
	LabelString           "Conjecture."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{conjecture*}{Conjecture}
	EndPreamble
End


Style Criterion
	LatexName             criterion*
	LabelString           "Criterion."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{criterion*}{Criterion}
	EndPreamble
End


Style Algorithm
	LatexName             algorithm*
	LabelString           "Algorithm #."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{algorithm*}{Algorithm}
	EndPreamble
End


Style Fact
	LatexName             fact*
	LabelString           "Fact."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{fact*}{Fact}
	EndPreamble
End


Style Axiom
	LatexName             ax*
	LabelString           "Axiom."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{ax*}{Axiom}
	EndPreamble
End


Style Definition
	LatexName             defn*
	LabelString           "Definition."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem*{defn*}{Definition}
	EndPreamble
End


Style Example
	LatexName             example*
	LabelString           "Example."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem*{example*}{Example}
	EndPreamble
End


Style Condition
	LatexName             condition*
	LabelString           "Condition."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem*{condition*}{Condition}
	EndPreamble
End


Style Problem
	LatexName             problem*
	LabelString           "Problem."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem*{problem*}{Problem}
	EndPreamble
End


Style Exercise
	LatexName             xca*
	LabelString           "Exercise."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem*{xca*}{Exercise}
	EndPreamble
End


Style Remark
	LatexName             rem*
	LabelString           "Remark."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{rem*}{Remark}
	EndPreamble
End


Style Claim
	LatexName             claim*
	LabelString           "Claim."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{claim*}{Claim}
	EndPreamble
End


Style Note
	LatexName             note*
	LabelString           "Note."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{note*}{Note}
	EndPreamble
End


Style Notation
	LatexName             notation*
	LabelString           "Notation."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{notation*}{Notation}
	EndPreamble
End


Style Summary
	LatexName             summary*
	LabelString           "Summary."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{summary*}{Summary}
	EndPreamble
End


Style Acknowledgement
	LatexName             acknowledgement*
	LabelString           "Acknowledgement."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{acknowledgement*}{Acknowledgement}
	EndPreamble
End


Style Case
	LatexName             case*
	LabelString           "Case."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{case*}{Case}
	EndPreamble
End


Style Conclusion
	LatexName             conclusion*
	LabelString           "Conclusion."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{conclusion*}{Conclusion}
	EndPreamble
End
