# Common style for ACM SIGS layouts 
#
# Author: Gioele Barabucci

Format 59
Input stdclass.inc

Columns 2

Style Subtitle
	Category       FrontMatter
	Margin         Static

	LatexType      Command
        InTitle        1
	LatexName      subtitle

	ParSkip        0.4
        ItemSep        0
        TopSep         0
        BottomSep      1
        ParSep         1
        Align          Center
        LabelType      No_Label
        Font
		Size   Large
		Shape  Italic
	EndFont
End

Style Terms        
	Category              FrontMatter
        Margin                Dynamic

        LatexType             Command
        LatexName             terms
        LabelString           "General terms:"

	ParSkip               0.4
        BottomSep             1.5
        ParSep                1.5
        Align                 Block
        LabelType             Static
        LabelSep              M
	LabelFont
		Series   Bold
		Size     large
	EndFont
End

Style Keywords
        CopyStyle             Terms
        LatexName             keywords
        LabelString           "Keywords:"
End
