# Suggested style to write your code:
# Within same scrap, lines are separated by newlines (Ctrl-Return), use:
#  ItemSep               0.4
#    . disavantage: must type ctrl-return every single line
#    . advantage:   looks better (IMHO)
#                   resembles more closely the produced paper doc (more WYSIWYG)
#

Format 7
OutputType              literate

Style Scrap
	Margin                First_Dynamic
	LatexType             Paragraph
	LatexName             dummy
	NewLine               0
	LeftMargin            MMM
	ParSep                0.4
	TopSep                0.4
	BottomSep             0.4
	ItemSep               0.4
	Align                 Left
	AlignPossible         Block,Left
	FreeSpacing           1
	PassThru              1
	LabelType             Static
	LabelFont
	  Color               magenta
	EndFont
	TextFont
	  Color               latex
	  Family              Typewriter
	EndFont
End
