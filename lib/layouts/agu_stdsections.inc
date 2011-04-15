# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>
#          Martin Vermeer <martin.vermeer@hut.fi>

# This file is the counterpart of stdsections.inc
# It is desirable, as far as possible, to have the same look and feel for
# related layouts in latex and docbook.

Format 35
Input stdsections.inc

Style Part
	LatexType             Command
	LatexName             part
	InnerTag              head
	LabelTag              label
	CommandDepth          0
	TocLevel 0
End


Style Chapter
	LatexType             Command
	LatexName             chapter
	InnerTag              head
	LabelTag              label
	CommandDepth          1
	TocLevel 0
End


Style Section
	LatexType             Command
	LatexName             section
	InnerTag              head
	LabelTag              label
	CommandDepth          2
	LatexParam            "id=<s#>"
	TocLevel 1
End


Style Subsection
	LatexType             Command
	LatexName             subsect1
	InnerTag              head
	LabelTag              label
	CommandDepth          3
	LatexParam            "id=<s#>"
	TocLevel 2
End


Style Subsubsection
	LatexType             Command
	LatexName             subsect2
	InnerTag              head
	LabelTag              label
	CommandDepth          4
	LatexParam            "id=<s#>"
	TocLevel 3
End


Style Paragraph
	LatexType             Command
	LatexName             subsect3
	InnerTag              head
	LabelTag              label
	CommandDepth          5
	LatexParam            "id=<s#>"
	TocLevel 4
End


Style Subparagraph
	LatexType             Command
	LatexName             subsect4
	InnerTag              head
	LabelTag              label
	CommandDepth          6
	LatexParam            "id=<s#>"
	TocLevel 5
End

Style Subsubparagraph
	LatexType             Command
	LatexName             subsect5
	InnerTag              head
	LabelTag              label
	CommandDepth          7
	LatexParam            "id=<s#>"
End

