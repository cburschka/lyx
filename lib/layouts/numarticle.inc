# Author : André Pönitz
# This include file contains label definitions for an article-like numbering.

Format 41

# appendix applies to sections.
Counter section
	Within                "none"
	LabelString	          "\arabic{section}"
	LabelStringAppendix   "\Alph{section}"
End
