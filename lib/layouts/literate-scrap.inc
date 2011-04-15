# Suggested style to write your code:
# Within same scrap, lines are separated by newlines (Ctrl-Return), use:
#  ItemSep               0.4
#    . disavantage: must type ctrl-return every single line
#    . advantage:   looks better (IMHO)
#                   resembles more closely the produced paper doc (more WYSIWYG)
#

Format 35
OutputType              literate

Style Scrap
	Margin                First_Dynamic
	LatexType             Paragraph
	LatexName             dummy
	LeftMargin            MMM
	Align                 Left
	AlignPossible         Block,Left
	NewLine               0
	FreeSpacing           1
	PassThru              1
	ParbreakIsNewline     1
	LabelType             Static
	LabelFont
	  Color               magenta
	EndFont
	TextFont
	  Color               latex
	  Family              Typewriter
	EndFont
End
