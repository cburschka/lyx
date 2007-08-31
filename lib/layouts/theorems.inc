# Author: Richard Heck <rgheck@comcast.net>
# Adapted from amsmaths.inc

# The environnements defined are :
# - Theorem
# - Corollary
# - Lemma
# - Proposition
# - Conjecture
# - Definition
# - Example
# - Exercise
# - Remark
# - Note
# - Claim
# - Case
# - Conclusion
# - Fact

Style Theorem
	Margin                First_Dynamic
	LatexType             Environment
	LatexName             thm
	NextNoIndent          1
	LabelSep              xx
	ParIndent             MMM
	ParSkip               0.4
	ItemSep               0.2
	TopSep                0.7
	BottomSep             0.7
	ParSep                0.3
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Counter
	LabelCounter          theorem
	LabelString           "Theorem \arabic{theorem}."
	Font
	  Shape               Italic
	  Size                Normal
	EndFont
	LabelFont
	  Shape               Up
	  Series              Bold
	EndFont
	Preamble
		\newtheorem{thm}{Theorem}
	EndPreamble
End

Style Corollary
	CopyStyle             Theorem
	LatexName             cor
	LabelString           "Corollary \thetheorem."
	Preamble
	  \newtheorem{cor}[thm]{Corollary}
	EndPreamble
End

Style Lemma
	CopyStyle             Theorem
	LatexName             lem
	LabelString           "Lemma \thetheorem."
	Preamble
	  \newtheorem{lem}[thm]{Lemma}
	EndPreamble
End

Style Proposition
	CopyStyle             Theorem
	LatexName             prop
	LabelString           "Proposition \thetheorem."
	Preamble
	  \newtheorem{prop}[thm]{Proposition}
	EndPreamble
End

Style Conjecture
	CopyStyle             Theorem
	LatexName             conjecture
	LabelString           "Conjecture \thetheorem."
	Preamble
	  \newtheorem{conjecture}[thm]{Conjecture}
	EndPreamble
End

Style Fact
	CopyStyle             Theorem
	LatexName             fact
	LabelString           "Fact \thetheorem."
	Preamble
	  \newtheorem{fact}[thm]{Fact}
	EndPreamble
End

Style Definition
	CopyStyle             Theorem
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
	  \newtheorem{defn}[thm]{Definition}
	EndPreamble
End


Style Example
	CopyStyle             Definition
	LatexName             example
	LabelString           "Example \thetheorem."
	Preamble
	  \newtheorem{example}[thm]{Example}
	EndPreamble
End


Style Exercise
	CopyStyle             Definition
	LatexName             xca
	LabelString           "Exercise \thetheorem."
	Preamble
	  \newtheorem{xca}{Exercise}
	EndPreamble
End


Style Remark
	CopyStyle             Theorem
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
	  \newtheorem{rem}[thm]{Remark}
	EndPreamble
End


Style Claim
	CopyStyle             Remark
	LatexName             claim
	LabelString           "Claim \thetheorem."
	Preamble
	  \newtheorem{claim}[thm]{Claim}
	EndPreamble
End

Style Case
	CopyStyle             Remark
	LatexName             case
	LabelString           "Case \thetheorem."
	Preamble
	  \newtheorem{case}[thm]{Case}
	EndPreamble
End


Style Conclusion
	CopyStyle             Remark
	LatexName             conclusion
	LabelString           "Conclusion \thetheorem."
	Preamble
	  \newtheorem{conclusion}[thm]{Conclusion}
	EndPreamble
End
