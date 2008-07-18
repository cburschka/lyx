# Author : David L. Johnson <dlj0@lehigh.edu>
# Probably broken by Jean-Marc Lasgouttes <Jean-Marc.Lasgouttes@inria.fr>
# modified and modularized by Emmanuel GUREGHIAN <gureghia@boston.bertin.fr>
# This version has Theorems and other results numbered in one sequence, and all
# numbered environments (figures as well) containing the section number.
# (Themorem (1.1), Corollary (1.2) Theorem (1.3), etc.
# These are only the theorems styles environnements
# Tinkered with Sep. '07 by Paul Rubin <rubin@msu.edu>

# The environnements defined (regular and starred) are :
# - Theorem
# - Corollary
# - Lemma
# - Proposition
# - Conjecture
# - Criterion
# - Algorithm
# - Axiom
# - Definition
# - Example
# - Condition
# - Problem
# - Exercise
# - Remark
# - Note
# - Notation
# - Claim
# - Summary
# - Acknowledgement
# - Conclusion
# - Fact
# - Assumption
# - Case (regular only -- defined as an enumeration)

Format 4


Counter
	Name                 theorem
	Within               section
End


Style Theorem
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
	AlignPossible         Block, Left
	LabelType             Counter
	LabelCounter          theorem
	LabelString           "Theorem @Section@.\arabic{theorem}."
	Font
	  Shape               Italic
	  Size                Normal
	EndFont
	LabelFont
	  Shape               Up
	  Series              Bold
	EndFont
	Preamble
	  \@ifundefined{theoremstyle}{\usepackage{amsthm}}{}
	  \theoremstyle{plain}
	  \newtheorem{thm}{Theorem}[section]
	EndPreamble
End


Style Theorem*
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             thm*
	LabelType             Static
	LabelString           "Theorem."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{thm*}{Theorem}
	EndPreamble
End


Style Corollary
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             cor
	LabelString           "Corollary @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{cor}[thm]{Corollary}
	EndPreamble
End


Style Corollary*
	CopyStyle             Theorem*
	DependsOn	      Theorem
	LatexName             cor*
	LabelString           "Corollary."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{cor*}{Corollary}
	EndPreamble
End


Style Lemma
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             lem
	LabelString           "Lemma @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{lem}[thm]{Lemma}
	EndPreamble
End


Style Lemma*
	CopyStyle             Theorem*
	DependsOn	      Theorem
	LatexName             lem*
	LabelString           "Lemma."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{lem*}{Lemma}
	EndPreamble
End


Style Proposition
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             prop
	LabelString           "Proposition @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{prop}[thm]{Proposition}
	EndPreamble
End


Style Proposition*
	CopyStyle             Theorem*
	DependsOn	      Theorem
	LatexName             prop*
	LabelString           "Proposition."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{prop*}{Proposition}
	EndPreamble
End


Style Conjecture
	CopyStyle             Theorem
	DependsOn	      Theorem
	DependsOn	      Theorem
	LatexName             conjecture
	LabelString           "Conjecture @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{conjecture}[thm]{Conjecture}
	EndPreamble
End


Style Conjecture*
	CopyStyle             Theorem*
	DependsOn	      Theorem
	LatexName             conjecture*
	LabelString           "Conjecture."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{conjecture*}{Conjecture}
	EndPreamble
End


Style Criterion
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             criterion
	LabelString           "Criterion @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{criterion}[thm]{Criterion}
	EndPreamble
End

Style Criterion*
	CopyStyle             Theorem*
	DependsOn	      Theorem
	LatexName             criterion*
	LabelString           "Criterion."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{criterion*}{Criterion}
	EndPreamble
End


Style Algorithm
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             algorithm
	LabelString           "Algorithm @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{algorithm}[thm]{Algorithm}
	EndPreamble
End


Style Algorithm*
	CopyStyle             Theorem*
	DependsOn	      Theorem
	LatexName             algorithm*
	LabelString           "Algorithm."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{algorithm*}{Algorithm}
	EndPreamble
End


Style Fact
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             fact
	LabelString           "Fact @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{fact}[thm]{Fact}
	EndPreamble
End


Style Fact*
	CopyStyle             Theorem*
	DependsOn	      Theorem
	LatexName             fact*
	LabelString           "Fact."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{fact*}{Fact}
	EndPreamble
End


Style Axiom
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             ax
	LabelString           "Axiom @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{ax}[thm]{Axiom}
	EndPreamble
End


Style Axiom*
	CopyStyle             Theorem*
	DependsOn	      Theorem
	LatexName             ax*
	LabelString           "Axiom."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{ax*}{Axiom}
	EndPreamble
End


Style Definition
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             defn
	LabelString           "Definition @Section@.\arabic{theorem}."
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


Style Definition*
	CopyStyle             Definition
	DependsOn	      Theorem
	LatexName             defn*
	LabelType             Static
	LabelString           "Definition."
	Preamble
	 \theoremstyle{definition}
	 \newtheorem*{defn*}{Definition}
	EndPreamble
End


Style Example
	CopyStyle             Definition
	DependsOn	      Theorem
	LatexName             example
	LabelString           "Example @Section@.\arabic{theorem}."
	Preamble
	 \theoremstyle{definition}
	  \newtheorem{example}[thm]{Example}
	EndPreamble
End


Style Example*
	CopyStyle             Definition*
	DependsOn	      Theorem
	LatexName             example*
	LabelString           "Example."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem*{example*}{Example}
	EndPreamble
End


Style Condition
	CopyStyle             Definition
	DependsOn	      Theorem
	LatexName             condition
	LabelString           "Condition @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem{condition}[thm]{Condition}
	EndPreamble
End


Style Condition*
	CopyStyle             Definition*
	DependsOn	      Theorem
	LatexName             condition*
	LabelString           "Condition."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem*{condition*}{Condition}
	EndPreamble
End


Style Problem
	CopyStyle             Definition
	DependsOn	      Theorem
	LatexName             problem
	LabelString           "Problem @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem{problem}[thm]{Problem}
	EndPreamble
End


Style Problem*
	CopyStyle             Definition*
	DependsOn	      Theorem
	LatexName             problem*
	LabelString           "Problem."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem*{problem*}{Problem}
	EndPreamble
End


Style Exercise
	CopyStyle             Definition
	DependsOn	      Theorem
	LatexName             xca
	LabelString           "Exercise @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem{xca}[thm]{Exercise}
	EndPreamble
End


Style Exercise*
	CopyStyle             Definition*
	DependsOn	      Theorem
	LatexName             xca*
	LabelString           "Exercise."
	Preamble
	  \theoremstyle{definition}
	  \newtheorem*{xca*}{Exercise}
	EndPreamble
End


Style Remark
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             rem
	LabelString           "Remark @Section@.\arabic{theorem}."
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


Style Remark*
	CopyStyle             Remark
	DependsOn	      Theorem
	LatexName             rem*
	LabelType             Static
	LabelString           "Remark."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{rem*}{Remark}
	EndPreamble
End


Style Claim
	CopyStyle             Remark
	DependsOn	      Theorem
	LatexName             claim
	LabelString           "Claim @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{claim}[thm]{Claim}
	EndPreamble
End


Style Claim*
	CopyStyle             Remark*
	DependsOn	      Theorem
	LatexName             claim*
	LabelString           "Claim."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{claim*}{Claim}
	EndPreamble
End


Style Note
	CopyStyle             Remark
	DependsOn	      Theorem
	LatexName             note
	LabelString           "Note @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{note}[thm]{Note}
	EndPreamble
End


Style Note*
	CopyStyle             Remark*
	DependsOn	      Theorem
	LatexName             note*
	LabelString           "Note."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{note*}{Note}
	EndPreamble
End


Style Notation
	CopyStyle             Remark
	DependsOn	      Theorem
	LatexName             notation
	LabelString           "Notation @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{notation}[thm]{Notation}
	EndPreamble
End


Style Notation*
	CopyStyle             Remark*
	DependsOn	      Theorem
	LatexName             notation*
	LabelString           "Notation."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{notation*}{Notation}
	EndPreamble
End


Style Summary
	CopyStyle             Remark
	DependsOn	      Theorem
	LatexName             summary
	LabelString           "Summary @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{summary}[thm]{Summary}
	EndPreamble
End


Style Summary*
	CopyStyle             Remark*
	DependsOn	      Theorem
	LatexName             summary*
	LabelString           "Summary."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{summary*}{Summary}
	EndPreamble
End


Style Acknowledgement
	CopyStyle             Remark
	DependsOn	      Theorem
	LatexName             acknowledgement
	LabelString           "Acknowledgement @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{acknowledgement}[thm]{Acknowledgement}
	EndPreamble
End


Style Acknowledgement*
	CopyStyle             Remark*
	DependsOn	      Theorem
	LatexName             acknowledgement*
	LabelString           "Acknowledgement."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{acknowledgement*}{Acknowledgement}
	EndPreamble
End


Style Conclusion
	CopyStyle             Remark
	DependsOn	      Theorem
	LatexName             conclusion
	LabelString           "Conclusion @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem{conclusion}[thm]{Conclusion}
	EndPreamble
End


Style Conclusion*
	CopyStyle             Remark*
	DependsOn	      Theorem
	LatexName             conclusion*
	LabelString           "Conclusion."
	Preamble
	  \theoremstyle{remark}
	  \newtheorem*{conclusion*}{Conclusion}
	EndPreamble
End


Style Assumption
	CopyStyle             Theorem
	DependsOn	      Theorem
	LatexName             assumption
	LabelString           "Assumption @Section@.\arabic{theorem}."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem{assumption}[thm]{Assumption}
	EndPreamble
End


Style Assumption*
	CopyStyle             Theorem*
	DependsOn	      Theorem
	LatexName             assumption*
	LabelString           "Assumption."
	Preamble
	  \theoremstyle{plain}
	  \newtheorem*{assumption*}{Assumption}
	EndPreamble
End


# Define Case as an enumeration environment
Style Case
	CopyStyle             Enumerate
	LatexName             caseenv
	LabelType             Static
	LeftMargin            "MMMMMMN"
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
