# Textclass definition file for LaTeX.
# Author : Martin vermeer <amrtin.vermeer@hut.fi>
# Inset layouts definition

Format 2

InsetLayout Marginal
	LabelString           margin
	LatexType             command
	LatexName             marginpar
	LabelFont
	  Color               Red
	  Size                Small
	EndFont
End

InsetLayout Foot
	LabelString           foot
	LatexType             command
	LatexName             footnote
	LabelFont
	  Color               Green
	  Size                Small
	EndFont
End

InsetLayout Note:Comment
	LabelString           comment
	LatexType             environment
	LatexName             comment
	LabelFont
	  Color               Magenta
	  Size                Small
	EndFont
End


InsetLayout Note:Note
	LabelString           note
	LatexType             command
	LatexName             note
	LabelFont
	  Color               Blue
	  Size                Small
	EndFont
End

InsetLayout Note:Greyedout
	LabelString           greyedout
	LatexType             command
	LatexName             greyedout
	LabelFont
	  Color               Red
	  Size                Small
	EndFont
End

InsetLayout Note:Framed
	LabelString           framed
	LatexType             command
	LatexName             framed
	LabelFont
	  Color               Red
	  Size                Small
	EndFont
End

InsetLayout Note:Shaded
	LabelString           shaded
	LatexType             command
	LatexName             shaded
	LabelFont
	  Color               Red
	  Size                Small
	EndFont
End


