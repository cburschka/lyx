# Author : David L. Johnson <dlj0@lehigh.edu>
# Probably broken by Jean-Marc Lasgouttes <Jean-Marc.Lasgouttes@inria.fr>
# modified and modularized by Emmanuel GUREGHIAN <gureghia@boston.bertin.fr>


# These are only the plain (unnumbered) theorem styles environnements
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
# - Case
# - Conclusion

# Theorem-plain style declaration
Style Theorem
  LatexName		thm*
  LabelString		"Theorem."

  Preamble
    \theoremstyle{plain}    
    \newtheorem*{thm*}{Theorem}
  EndPreamble
End  

# Corollary-plain style declaration
Style Corollary
  LatexName		cor*
  LabelString		"Corollary."

  Preamble
    \theoremstyle{plain}    
    \newtheorem*{cor*}{Corollary}
  EndPreamble
End

# Lemma-plain style declaration
Style Lemma
  LatexName		lem*
  LabelString		"Lemma."

  Preamble
    \theoremstyle{plain}    
    \newtheorem*{lem*}{Lemma}
  EndPreamble
End

# Proposition-plain style declaration
Style Proposition
  LatexName		prop*
  LabelString		"Proposition."

  Preamble
    \theoremstyle{plain}    
    \newtheorem*{prop*}{Proposition}
  EndPreamble
End  

# Conjecture-plain style declaration
Style Conjecture
  LatexName		conjecture*
  LabelString		"Conjecture."

  Preamble
    \theoremstyle{plain}    
    \newtheorem*{conjecture*}{Conjecture}
  EndPreamble
End  

# Criterion-plain style declaration
Style Criterion
  LatexName		criterion*
  LabelString		"Criterion."

  Preamble
    \theoremstyle{plain}    
    \newtheorem*{criterion*}{Criterion}
  EndPreamble
End  

# Algorithm-plain style declaration
Style Algorithm
  LatexName		algorithm*
  LabelString		"Algorithm #."

  Preamble
    \theoremstyle{plain}    
    \newtheorem*{algorithm*}{Algorithm} 
  EndPreamble
End  

# Fact-plain style declaration
Style Fact
  LatexName		fact*
  LabelString		"Fact."

  Preamble
    \theoremstyle{plain}    
    \newtheorem*{fact*}{Fact} 
  EndPreamble
End  

# Axiom-plain style declaration
Style Axiom
  LatexName		ax*
  LabelString		"Axiom."

  Preamble
    \theoremstyle{plain}    
    \newtheorem*{ax*}{Axiom}
  EndPreamble
End

# Definition-plain style declaration
Style Definition
  LatexName		defn*
  LabelString		"Definition."

  Preamble
    \theoremstyle{definition}
    \newtheorem*{defn*}{Definition}
  EndPreamble
End  

# Example-plain style declaration
Style Example
  LatexName		example*
  LabelString		"Example."

  Preamble
    \theoremstyle{definition}
    \newtheorem*{example*}{Example}
  EndPreamble
End  

# Condition-plain style declaration
Style Condition
  LatexName		condition*
  LabelString		"Condition."

  Preamble
    \theoremstyle{definition}
    \newtheorem*{condition*}{Condition}
  EndPreamble
End  

# Problem-plain style declaration
Style Problem
  LatexName		problem*
  LabelString		"Problem."

  Preamble
    \theoremstyle{definition}
    \newtheorem*{problem*}{Problem}
  EndPreamble
End  

# Exercise-plain style declaration
Style Exercise
  LatexName		xca*
  LabelString		"Exercise."

  Preamble
    \theoremstyle{definition}
    \newtheorem*{xca*}{Exercise}
  EndPreamble
End  

# Remark-plain style declaration
Style Remark
  LatexName		rem*
  LabelString		"Remark."

  Preamble
    \theoremstyle{remark}
    \newtheorem*{rem*}{Remark}
  EndPreamble
End  

# Claim-plain style declaration
Style Claim
  LatexName		claim*
  LabelString		"Claim."

  Preamble
    \theoremstyle{remark}    
    \newtheorem*{claim*}{Claim} 
  EndPreamble
End  

# Note-plain style declaration
Style Note
  LatexName		note*
  LabelString		"Note."

  Preamble
    \theoremstyle{remark}    
    \newtheorem*{note*}{Note} 
  EndPreamble
End  

# Notation style declaration
Style Notation
  LatexName		notation*
  LabelString		"Notation."

  Preamble
    \theoremstyle{remark}
    \newtheorem*{notation*}{Notation}
  EndPreamble
End  
 

# Summary style declaration
Style Summary
  LatexName		summary*
  LabelString		"Summary."

  Preamble
    \theoremstyle{remark}
    \newtheorem*{summary*}{Summary}
  EndPreamble
End  
 

# Acknowledgement style declaration
Style Acknowledgement
  LatexName		acknowledgement*
  LabelString		"Acknowledgement."

  Preamble
    \theoremstyle{remark}
    \newtheorem*{acknowledgement*}{Acknowledgement}
  EndPreamble
End  
 

# Case style declaration
Style Case
  LatexName		case*
  LabelString		"Case."

  Preamble
    \theoremstyle{remark}
    \newtheorem*{case*}{Case}
  EndPreamble
End  
 

# Conclusion style declaration
Style Conclusion
  LatexName		conclusion*
  LabelString		"Conclusion."

  Preamble
    \theoremstyle{remark}
    \newtheorem*{conclusion*}{Conclusion}
  EndPreamble
End  
 
