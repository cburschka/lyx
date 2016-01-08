# Standard RefPrefix definitions. 
# This defines the label prefixes for use with refstyle.sty.
#
# Author: Richard Heck <rgheck@comcast.net>
#
# Note that this file is included in stdsections.inc, which is included
# in stdclass.inc, so any file that includes either of these will get
# the standard RefPrefix definitions.

Format 59

ModifyStyle Part
	RefPrefix part
End

ModifyStyle Chapter
	RefPrefix chap
End

ModifyStyle Section
	RefPrefix sec
End

ModifyStyle Subsection
	RefPrefix subsec
End

ModifyStyle Subsubsection
	RefPrefix subsec
End

ModifyStyle Paragraph
	RefPrefix par
End

ModifyStyle Subparagraph
	RefPrefix par
End
