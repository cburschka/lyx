# Author : André Pönitz
# This include file contains label definitions for an article-like numbering.

Format 8

# appendix applies to sections.
Counter
	Name                  section
	Within                "none"
	LabelString	          "\arabic{section}"
	LabelStringAppendix   "\Alph{section}"
End




