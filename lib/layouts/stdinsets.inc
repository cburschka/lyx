# Textclass definition file for LaTeX.
# Author : Martin vermeer <amrtin.vermeer@hut.fi>
# Inset layouts definition

Format 5

InsetLayout Marginal
	LabelString           margin
	LatexType             command
	LatexName             marginpar
	Font
	  Color               foreground
	  Size                Small
	  Family              Roman
	  Shape               Up
	  Series              Medium
	  Misc                No_Emph
	  Misc                No_Noun
	  Misc                No_Bar
	EndFont
	LabelFont
	  Color               Red
	  Size                Small
	EndFont
	MultiPar              true
End

InsetLayout Foot
	LabelString           foot
	LatexType             command
	LatexName             footnote
	Font
	  Color               foreground
	  Size                Small
	  Family              Roman
	  Shape               Up
	  Series              Medium
	  Misc                No_Emph
	  Misc                No_Noun
	  Misc                No_Bar
	EndFont
	LabelFont
	  Color               Green
	  Size                Small
	EndFont
	MultiPar              true
End

InsetLayout Note:Comment
	LabelString           comment
	LatexType             environment
	LatexName             comment
	BgColor               commentbg
	LabelFont
	  Color               comment
	  Size                Small
	EndFont
	MultiPar              true
End


InsetLayout Note:Note
	LabelString           note
	LatexType             command
	LatexName             note
	BgColor               notebg
	LabelFont
	  Color               note
	  Size                Small
	EndFont
	MultiPar              true
End

InsetLayout Note:Greyedout
	LabelString           greyedout
	LatexType             environment
	LatexName             lyxgreyedout
	BgColor               greyedoutbg
	LabelFont
	  Color               greyedout
	  Size                Small
	EndFont
	MultiPar              true
End

InsetLayout Note:Framed
	LabelString           framed
	LatexType             environment
	LatexName             framed
	BgColor               greyedoutbg
	LabelFont
	  Color               greyedout
	  Size                Small
	EndFont
	MultiPar              true
End

InsetLayout Note:Shaded
	LabelString           shaded
	LatexType             environment
	LatexName             shaded
	BgColor               shaded
	LabelFont
	  Color               greyedout
	  Size                Small
	EndFont
	MultiPar              true
End

InsetLayout ERT
	LabelString           ERT
	LatexType             none
	Decoration            minimalistic
	Font
	  Color               latex
	  Family              typewriter
	EndFont
	LabelFont
	  Color               latex
	  Size                Small
	EndFont
	MultiPar              true
	PassThru              true
	KeepEmpty             true
	FreeSpacing           true
	ForceLTR              true
End

InsetLayout Listings
	LabelString           Listings
	LatexType             none
	Decoration            minimalistic
	Font
	  Color               foreground
	  Family              typewriter
	EndFont
	LabelFont
	  Color               foreground
	  Size                Small
	EndFont
	MultiPar              true
	PassThru              true
	KeepEmpty             true
	FreeSpacing           true
End

InsetLayout Branch
	Decoration            minimalistic
	LabelFont
	  Color               Red
	  Size                Small
	EndFont
	MultiPar              true
End

InsetLayout Index
	LabelString           Idx
	LatexType             command
	LatexName             index
	Decoration            minimalistic
	Font
	  Color               Green
	  Size                Small
	  Family              Roman
	  Shape               Up
	  Series              Medium
	  Misc                No_Emph
	  Misc                No_Noun
	  Misc                No_Bar
	EndFont
	LabelFont
	  Color               Green
	  Size                Small
	EndFont
	MultiPar              false
	NeedProtect           true
End

InsetLayout Box
	LabelFont
	  Color               foreground
	  Size                Small
	EndFont
	MultiPar              true
End

InsetLayout figure
	LabelString           Figure
	LabelFont
	  Color               collapsable
	  Size                Small
	EndFont
	MultiPar              true
End

InsetLayout table
	LabelString           Table
	LabelFont
	  Color               collapsable
	  Size                Small
	EndFont
	MultiPar              true
End

InsetLayout URL
	LyXType               standard
	LatexName             url
	LatexType             command
	Decoration            minimalistic
	LabelString           URL
	PassThru              true
	ForceLTR              true
	Font
	  Family              Typewriter
	  Color               Blue
	EndFont
	LabelFont
	  Family              Typewriter
	  Color               Blue
	  Size                Small
	EndFont
	Preamble
	  \usepackage{url}
	EndPreamble
End

InsetLayout OptArg
	LabelString           opt
	LabelFont
	  Color               collapsable
	  Size                Small
	EndFont
	MultiPar              false
End
