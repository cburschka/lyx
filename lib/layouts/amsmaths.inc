# Author : David L. Johnson <dlj0@lehigh.edu>
# Probably broken by Jean-Marc Lasgouttes <Jean-Marc.Lasgouttes@inria.fr>
# modified and modularized by Emmanuel GUREGHIAN <gureghia@boston.bertin.fr>
# This version has Theorems and other results numbered in one sequence, and all
# numbered environments (figures as well) containing the section number.
# (Themorem (1.1), Corollary (1.2) Theorem (1.3), etc.
# These are only the theorems styles environnements


# The environnements defined are :
# - Theorem
# - Theorem*
# - Corollary
# - Corollary*
# - Lemma
# - Lemma*
# - Proposition
# - Proposition*
# - Conjecture
# - Conjecture*
# - Criterion
# - Algorithm
# - Axiom
# - Definition
# - Definition*
# - Example
# - Example*
# - Condition
# - Problem
# - Exercise
# - Remark
# - Remark*
# - Note
# - Note*
# - Notation
# - Claim
# - Claim*
# - Summary
# - Acknowledgement
# - Acknowledgement*
# - Case
# - Conclusion
# - Conclusion*
# - Fact 
# - Fact*

# Theorem-numbered style declaration
Style Theorem
  Margin		First_Dynamic
  LatexType		Environment
  LatexName		thm
  NextNoIndent		1
  LabelSep		xx
  ParIndent             MMM  
  ParSkip		0.4
  ItemSep		0.2
  TopSep		0.7
  BottomSep		0.7
  ParSep		0.3
  Align			Block
  AlignPossible		Block, Left
  LabelType		Static
  LabelString		"Theorem #."

  
  # standard font definition
  Font
    Shape 		Italic
    Size		Normal
  EndFont

  # label font definition
  LabelFont
    Shape		Up
    Series		Bold
  EndFont

End  

# Theorem-unnumbered style declaration
Style Theorem*
  CopyStyle		Theorem
  LatexName		thm*
  LabelString		"Theorem."

  Preamble
    \theoremstyle{plain}    
    \newtheorem*{thm*}{Theorem} 
  EndPreamble
End  

# Corollary-numbered style declaration
Style Corollary
  CopyStyle		Theorem
  LatexName		cor
  LabelString		"Corollary #."

  Preamble
    \theoremstyle{plain}    
    \newtheorem{cor}[thm]{Corollary} %%Delete [thm] to re-start numbering
  EndPreamble
End  
# Corollary-unnumbered style declaration
Style Corollary*
  CopyStyle		Theorem
  LatexName		cor*
  LabelString		"Corollary."

  Preamble
    \theoremstyle{plain}    
    \newtheorem*{cor*}{Corollary}
  EndPreamble
End  

# Lemma-numbered style declaration
Style Lemma
  CopyStyle		Theorem
  LatexName		lem
  LabelString		"Lemma #."

  Preamble
    \theoremstyle{plain}    
    \newtheorem{lem}[thm]{Lemma} %%Delete [thm] to re-start numbering
  EndPreamble
End  

# Lemma-unnumbered style declaration
Style Lemma*
  CopyStyle		Theorem
  LatexName		lem*
  LabelString		"Lemma."

  Preamble
    \theoremstyle{plain}    
    \newtheorem*{lem*}{Lemma} %%Delete [thm] to re-start numbering
  EndPreamble
End  

# Proposition-numbered style declaration
Style Proposition
  CopyStyle		Theorem
  LatexName		prop
  LabelString		"Proposition #."

  Preamble
    \theoremstyle{plain}    
    \newtheorem{prop}[thm]{Proposition} %%Delete [thm] to re-start numbering
  EndPreamble
End  

# Proposition-unnumbered style declaration
Style Proposition*
  CopyStyle		Theorem
  LatexName		prop*
  LabelString		"Proposition."

  Preamble
    \theoremstyle{plain}    
    \newtheorem*{prop*}{Proposition} 
  EndPreamble
End  

# Conjecture-numbered style declaration
Style Conjecture
  CopyStyle		Theorem
  LatexName		conjecture
  LabelString		"Conjecture #."

  Preamble
    \theoremstyle{plain}    
    \newtheorem{conjecture}[thm]{Conjecture} %%Delete [thm] to re-start numbering
  EndPreamble
End  

# Conjecture-unnumbered style declaration
Style Conjecture*
  CopyStyle		Theorem
  LatexName		conjecture*
  LabelString		"Conjecture."

  Preamble
    \theoremstyle{plain}    
    \newtheorem*{conjecture*}{Conjecture} 
  EndPreamble
End 

# Criterion-numbered style declaration
Style Criterion
  CopyStyle		Theorem
  LatexName		criterion
  LabelString		"Criterion #."

  Preamble
    \theoremstyle{plain}    
    \newtheorem{criterion}[thm]{Criterion} %%Delete [thm] to re-start numbering
  EndPreamble
End  

# Algorithm-numbered style declaration
Style Algorithm
  CopyStyle		Theorem
  LatexName		algorithm
  LabelString		"Algorithm #."

  Preamble
    \theoremstyle{plain}    
    \newtheorem{algorithm}[thm]{Algorithm} %%Delete [thm] to re-start numbering
  EndPreamble
End  

# Fact-numbered style declaration
Style Fact
  CopyStyle		Theorem
  LatexName		fact
  LabelString		"Fact #."

  Preamble
    \theoremstyle{plain}    
    \newtheorem{fact}[thm]{Fact} 
  EndPreamble
End  

# Fact-unnumbered style declaration
Style Fact*
  CopyStyle		Theorem
  LatexName		fact*
  LabelString		"Fact."

  Preamble
    \theoremstyle{plain}    
    \newtheorem*{fact*}{Fact} 
  EndPreamble
End  

# Axiom-numbered style declaration
Style Axiom
  CopyStyle		Theorem
  LatexName		ax
  LabelString		"Axiom #."
  
  Preamble
    \theoremstyle{plain}    
    \newtheorem{ax}[thm]{Axiom} 
  EndPreamble
End  

# Definition-numbered style declaration
Style Definition
  CopyStyle		Theorem
  LatexName		defn
  LabelString		"Definition #."
  
  # standard font definition
  Font
    Shape 		Up
  EndFont

  # label font definition
  LabelFont
    Shape		Up
    Series		Bold
  EndFont
 
  Preamble
   \theoremstyle{definition}
   \newtheorem{defn}[thm]{Definition}
  EndPreamble
End  
# Definition-unnumbered style declaration
Style Definition*
  CopyStyle		Definition
  LatexName		defn*
  LabelString		"Definition."
  
  Preamble
   \theoremstyle{definition}
   \newtheorem*{defn*}{Definition}
  EndPreamble
End  

# Example-numbered style declaration
Style Example
  CopyStyle		Definition
  LatexName		example
  LabelString		"Example #."

  Preamble
   \theoremstyle{definition}
    \newtheorem{example}[thm]{Example}
  EndPreamble
End  

# Example-unnumbered style declaration
Style Example*
  CopyStyle		Definition
  LatexName		example*
  LabelString		"Example."

  Preamble
   \theoremstyle{definition}
    \newtheorem*{example*}{Example}
  EndPreamble
End  

# Condition-numbered style declaration
Style Condition
  CopyStyle		Definition
  LatexName		condition
  LabelString		"Condition #."

  Preamble
   \theoremstyle{definition}
    \newtheorem{condition}[thm]{Condition}
  EndPreamble
End  

# Problem-numbered style declaration
Style Problem
  CopyStyle		Definition
  LatexName		problem
  LabelString		"Problem #."

  Preamble
   \theoremstyle{definition}
    \newtheorem{problem}[thm]{Problem}
  EndPreamble
End  

# Exercise-numbered style declaration
Style Exercise
  CopyStyle		Definition
  LatexName		xca
  LabelString		"Exercise #."

  Preamble
   \theoremstyle{definition}
    \newtheorem{xca}[section]{Exercise}%%Delete [section] for sequential numbering
  EndPreamble
End  

# Remark-numbered style declaration
Style Remark
  CopyStyle		Theorem
  LatexName		rem
  LabelString		"Remark #."
  
  # standard font definition
  Font
    Shape 		Up
    Size		Normal
  EndFont

  # label font definition
  LabelFont
    Series		Medium
    Shape		Italic
  EndFont

  Preamble
    \theoremstyle{remark}
    \newtheorem{rem}[thm]{Remark}
  EndPreamble
End  
# Remark-unnumbered style declaration
Style Remark*
  CopyStyle		Remark
  LatexName		rem*
  LabelString		"Remark."

  Preamble
    \theoremstyle{remark}
    \newtheorem*{rem*}{Remark}
  EndPreamble
End  

# Claim-numbered style declaration
Style Claim
  CopyStyle		Remark
  LatexName		claim
  LabelString		"Claim #."

  Preamble
    \theoremstyle{remark}    
    \newtheorem{claim}[thm]{Claim}
  EndPreamble
End  

# Claim-unnumbered style declaration
Style Claim*
  CopyStyle		Remark
  LatexName		claim*
  LabelString		"Claim."

  Preamble
    \theoremstyle{remark}    
    \newtheorem*{claim*}{Claim}
  EndPreamble
End  

# Note-numbered style declaration
Style Note
  CopyStyle		Remark
  LatexName		note
  LabelString		"Note #."

  Preamble
    \theoremstyle{remark}    
    \newtheorem{note}[thm]{Note} 
  EndPreamble
End  

# Note-unnumbered style declaration
Style Note*
  CopyStyle		Remark
  LatexName		note*
  LabelString		"Note."

  Preamble
    \theoremstyle{remark}    
    \newtheorem*{note*}{Note} 
  EndPreamble
End  


# Notation-numbered style declaration
Style Notation
  CopyStyle		Remark
  LatexName		notation
  LabelString		"Notation #."

  Preamble
    \theoremstyle{remark}    
    \newtheorem{notation}[thm]{Notation} 
  EndPreamble
End  

# Summary-numbered style declaration
Style Summary
  CopyStyle		Remark
  LatexName		summary
  LabelString		"Summary #."

  Preamble
    \theoremstyle{remark}    
    \newtheorem{summary}[thm]{Summary} 
  EndPreamble
End  

# Acknowledgement-numbered style declaration
Style Acknowledgement
  CopyStyle		Remark
  LatexName		acknowledgement
  LabelString		"Acknowledgement #."

  Preamble
    \theoremstyle{remark}    
    \newtheorem{acknowledgement}[thm]{Acknowledgement} 
  EndPreamble
End  
# Acknowledgement-unnumbered style declaration
Style Acknowledgement*
  CopyStyle		Remark
  LatexName		acknowledgement*
  LabelString		"Acknowledgement."

  Preamble
    \theoremstyle{remark}    
    \newtheorem*{acknowledgement*}{Acknowledgement} 
  EndPreamble
End  

# Case-numbered style declaration
Style Case
  CopyStyle		Remark
  LatexName		case
  LabelString		"Case #."

  Preamble
    \theoremstyle{remark}    
    \newtheorem{case}{Case} %%Numbering of Cases not keyed to sections 
  EndPreamble
End  

# Conclusion-numbered style declaration
Style Conclusion
  CopyStyle		Remark
  LatexName		conclusion
  LabelString		"Conclusion #."

  Preamble
    \theoremstyle{remark}    
    \newtheorem{conclusion}[thm]{Conclusion} 
  EndPreamble
End  

# Conclusion-unnumbered style declaration
Style Conclusion*
  CopyStyle		Remark
  LatexName		conclusion*
  LabelString		"Conclusion #."

  Preamble
    \theoremstyle{remark}    
    \newtheorem*{conclusion*}{Conclusion} 
  EndPreamble
End  

