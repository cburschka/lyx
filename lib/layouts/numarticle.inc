# Author : André Pönitz
# This include file contains label definitions for an article-like numbering.

Format 93

# appendix applies to sections.
Counter section
	GuiName Section
	Within                "none"
	LabelString	          "\arabic{section}"
	LabelStringAppendix   "\Alph{section}"
End
