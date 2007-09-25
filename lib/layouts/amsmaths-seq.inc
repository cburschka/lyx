# Author : David L. Johnson <dlj0@lehigh.edu>
# Probably broken by Jean-Marc Lasgouttes <Jean-Marc.Lasgouttes@inria.fr>
# modified and modularized by Emmanuel GUREGHIAN <gureghia@boston.bertin.fr>
# Tinkered with Sep. '07 by Paul Rubin <rubin@msu.edu>

# These are only the theorems styles environnements
# The environnements defined are :
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

# Use a simpler preamble.

Preamble
EndPreamble


# but more counters

# Do not restart theorem counter after section change
Counter
	Name                  thm
End

Counter
	Name                  corollary
End


Counter
	Name                  lemma
End


Counter
	Name                  proposition
End


Counter
	Name                  conjecture
End


Counter
	Name                  criterion
End


Counter
	Name                  algorithm
End


Counter
	Name                  fact
End


Counter
	Name                  axiom
End


Counter
	Name                  definition
End


Counter
	Name                  example
End


Counter
	Name                  condition
End


Counter
	Name                  problem
End


Counter
	Name                  exercise
End


Counter
	Name                  remark
End


Counter
	Name                  claim
End


Counter
	Name                  note
End


Counter
	Name                  notation
End


Counter
	Name                  summary
End


Counter
	Name                  acknowledgement
End


Counter
	Name                  conclusion
End

Counter
	Name                  assumption
End

Style Theorem
	LabelCounter          thm
	LabelString           "Theorem \arabic{thm}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{thm}{Theorem}
	EndPreamble
End


Style Corollary
	LabelCounter          corollary
	LabelString           "Corollary \arabic{corollary}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{cor}{Corollary}
	EndPreamble
End


Style Lemma
	LabelCounter          lemma
	LabelString           "Lemma \arabic{lemma}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{lem}{Lemma}
	EndPreamble
End


Style Proposition
	LabelCounter          proposition
	LabelString           "Proposition \arabic{proposition}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{prop}{Proposition}
	EndPreamble
End


Style Conjecture
	LabelCounter          conjecture
	LabelString           "Conjecture \arabic{conjecture}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{conjecture}{Conjecture}
	EndPreamble
End


Style Criterion
	LabelCounter          criterion
	LabelString           "Criterion \arabic{criterion}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{criterion}{Criterion}
	EndPreamble
End


Style Algorithm
	LabelCounter          algorithm
	LabelString           "Algorithm \arabic{algorithm}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{algorithm}{Algorithm}
	EndPreamble
End


Style Fact
	LabelCounter          fact
	LabelString           "Fact \arabic{fact}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{fact}{Fact}
	EndPreamble
End


Style Axiom
	LabelCounter          axiom
	LabelString           "Axiom \arabic{axiom}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{ax}{Axiom}
	EndPreamble
End


Style Definition
	LabelCounter          definition
	LabelString           "Definition \arabic{definition}."
	Preamble
	 \theoremstyle{definition}
	 \newtheorem{defn}{Definition}
	EndPreamble
End


Style Example
	LabelCounter          example
	LabelString           "Example \arabic{example}."
	Preamble
	 \theoremstyle{definition}
	  \newtheorem{example}{Example}
	EndPreamble
End


Style Condition
	LabelCounter          condition
	LabelString           "Condition \arabic{condition}."
	Preamble
	 \theoremstyle{definition}
	  \newtheorem{condition}{Condition}
	EndPreamble
End


Style Problem
	LabelCounter          problem
	LabelString           "Problem \arabic{problem}."
	Preamble
	 \theoremstyle{definition}
	  \newtheorem{problem}{Problem}
	EndPreamble
End


Style Exercise
	LabelCounter          exercise
	LabelString           "Exercise \arabic{exercise}."
	Preamble
	 \theoremstyle{definition}
	  \newtheorem{xca}{Exercise}
	EndPreamble
End


Style Remark
	LabelCounter          remark
	LabelString           "Remark \arabic{remark}."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{rem}{Remark}
	EndPreamble
End


Style Claim
	LabelCounter          claim
	LabelString           "Claim \arabic{claim}."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{claim}{Claim}
	EndPreamble
End


Style Note
	LabelCounter          note
	LabelString           "Note \arabic{note}."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{note}{Note}
	EndPreamble
End


Style Notation
	LabelCounter          notation
	LabelString           "Notation \arabic{notation}."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{notation}{Notation}
	EndPreamble
End


Style Summary
	LabelCounter          summary
	LabelString           "Summary \arabic{summary}."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{summary}{Summary}
	EndPreamble
End


Style Acknowledgement
	LabelCounter          acknowledgement
	LabelString           "Acknowledgement \arabic{acknowledgement}."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{acknowledgement}{Acknowledgement}
	EndPreamble
End


Style Conclusion
	LabelCounter          conclusion
	LabelString           "Conclusion \arabic{conclusion}."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{conclusion}{Conclusion}
	EndPreamble
End


Style Assumption
	LabelCounter          assumption
	LabelString           "Assumption \arabic{assumption}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{assumption}{Assumption}
	EndPreamble
End
