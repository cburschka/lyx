# Standard RefPrefix definitions. 
# This defines the label prefixes for use with refstyle.sty.
#
# Author: Richard Heck <rgheck@comcast.net>
#
# Note that this file is included in stdsections.inc, which is included
# in stdclass.inc, so any file that includes either of these will get
# the standard RefPrefix definitions.

Format 35

IfStyle Part
	RefPrefix part
End

IfStyle Chapter
	RefPrefix chap
End

IfStyle Section
	RefPrefix sec
End

IfStyle Subsection
	RefPrefix sub
End

IfStyle Subsubsection
	RefPrefix sub
End

IfStyle Paragraph
	RefPrefix par
End

IfStyle Subparagraph
	RefPrefix par
End
