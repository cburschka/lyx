# Standard textclass definition file. Taken from initial LyX source code
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Transposed by Pascal André <andre@via.ecp.fr>
# Heavily modifed and enhanced by several developers.

# This include files contains all non numbered versions of the
# sectionning-related commands that are useful for article-like
# document classes, but not for letters. It should be Input after
# modying the style of the regular sectionning layouts.

# Part* style definition
Style Part*
  CopyStyle		Part
  Margin		Static
  LatexName		part*
  LabelType		No_Label
End

# Chapter* style definition
Style Chapter*
  CopyStyle		Chapter
  Margin		Static
  LatexName		chapter*
  LabelType		No_Label
  OptionalArgs		0
End

# Section* style definition
Style Section*
  CopyStyle		Section
  Margin		Static
  LatexName		section*
  LabelType		No_Label
  OptionalArgs		0
End

# Subsection* style definition
Style Subsection*
  CopyStyle		Subsection
  Margin		Static
  LatexName		subsection*
  LabelType		No_Label
  OptionalArgs		0
End

# Subsubsection* style definition
Style Subsubsection*
  CopyStyle		Subsubsection
  Margin		Static
  LatexName		subsubsection*
  LabelType		No_Label
  OptionalArgs		0
End

# Paragraph* style definition
Style Paragraph*
  CopyStyle		Paragraph
  Margin		Static
  LatexName		paragraph*
  LabelType		No_Label
  OptionalArgs		0
End

# Subparagraph* style definition
Style Subparagraph*
  CopyStyle		Subparagraph
  Margin		Static
  LatexName		subparagraph*
  LabelType		No_Label
  OptionalArgs		0
End

