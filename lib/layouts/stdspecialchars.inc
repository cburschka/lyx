#
# Standard special characters
#

Format 54
SpecialChar LigatureBreak
	Type		Formatting
	LabelString	"Ligature Break|k"
	LyXOutput	0x007c
	LatexOutput	\textcompwordmark{}
	PlaintextOutput	0x200c
	XHTMLOutput	&#8204;
	IsLetter	1
End

SpecialChar EndOfSentence
	LabelString	"End of Sentence|E"
	LyXOutput	0x002e
	LatexOutput	\@.
	PlaintextOutput	0x002e
	DocBookOutput	"."
	XHTMLOutput	"."
End

SpecialChar LDots
	LabelString	"Ellipsis|i"
	Width		"."
	LyXOutput	0x2026
	LatexOutput	\ldots{}
	PlaintextOutput	0x2026
	DocBookOutput	"..."
	XHTMLOutput	&hellip;
End

SpecialChar MenuSeparator
	LabelString	"Menu Separator|M"
	Width		" x "
	LyXOutput	0x25b7
	LatexOutput	\lyxarrow{}
	LaTexOutputRTL	\lyxarrow*{}
	PlaintextOutput	"->"
	DocBookOutput	&lyxarrow;
	XHTMLOutput	&rArr;
	Requires	lyxarrow
End

SpecialChar Hyphenation
	Type		Formatting
	LabelString	"Hyphenation Point|H"
	LyXOutput	0x2010
	LatexOutput	\-
	IsLetter	1
End

SpecialChar Slash
	LabelString	"Breakable Slash|a"
	LyXOutput	0x002f
	LatexOutput	\slash{}
	PlaintextOutput	0x002f
	DocBookOutput	/
	XHTMLOutput	&frasl;
End

SpecialChar NobreakDash
	LabelString	"Protected Hyphen|y"
	LyXOutput	0x2010
	LatexOutput	\nobreakdash-
	NeedsProtect	1
	PlaintextOutput	0x2011
	DocBookOutput	-
	XHTMLOutput	&#8209;
	Requires	amsmath
	IsLetter	1
	Font
	  Color		latex
	EndFont
End

