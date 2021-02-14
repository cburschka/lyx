# Author : David L. Johnson <dlj0@lehigh.edu>
# Probably broken by Jean-Marc Lasgouttes <Jean-Marc.Lasgouttes@inria.fr>
# modified and modularized by Emmanuel GUREGHIAN <gureghia@boston.bertin.fr>
# Tinkered with Sep. '07 by Paul Rubin <rubin@msu.edu>
# Modularized Jan '08 by Richard Kimberly Heck <rikiheck@lyx.org>

# Including the maths stuff
Format 93

# By default AMS articles & books use leqno option (as SIAM journals).
Provides leqno 1

# the environments copied from the old amsart.layout are:
# - Bibliography
# - Title
# - Author
# - Date
# - Abstract (This is a title component for this style.)
# - Address
# - Email
# - Keywords
# - Subjectclass


### Now the title stuff. We copy do not use stdstruct.inc to keep
### things in the right order
Style Title
	Category              FrontMatter
	Margin                Static
	LatexType             Command
	InTitle               1
	LatexName             title
	ParSkip               0.4
	ItemSep               0
	TopSep                0
	BottomSep             1
	ParSep                1
	Align                 Center
	LabelType             No_Label
	Argument 1
		LabelString   "Short Title|S"
		Tooltip       "Short title which appears in the running headers"
		InsertCotext  1
	EndArgument
	Font
	  Shape               Smallcaps
	  Series              Bold
	  Size                Larger
	EndFont
	HTMLTitle             true
	DocBookTag            title
	DocBookTagType        paragraph
	DocBookInInfo         maybe
End


Style Author
	Category              FrontMatter
	Margin                Static
	LatexType             Command
	InTitle               1
	LatexName             author
	LabelSep              xxx
	ParSkip               0.4
	TopSep                1.3
	BottomSep             0.7
	ParSep                0.7
	Align                 Center
	LabelType             No_Label
	Font
	  Size                Large
	  Shape               Smallcaps
	EndFont
	DocBookTag            personname
	DocBookTagType        paragraph
	DocBookWrapperTag     author
	DocBookWrapperMergeWithPrevious true
	DocBookInInfo         always
End


Style Date
	Category              FrontMatter
	Margin                Dynamic
	LatexType             Command
	InTitle               1
	LatexName             date
	LabelSep              xxx
	ParSkip               0.4
	TopSep                0.9
	BottomSep             0.5
	ParSep                1
	Align                 Center
	LabelType             Static
	LabelString           "Date:"
	Font
	  Size                Large
	EndFont
	LabelFont
	  Shape               Italic
	  Size                Large
	EndFont
	DocBookTag            date
	DocBookTagType        paragraph
	DocBookInInfo         always
    DocBookNoFontInside   true
End


Style Abstract
	Category              FrontMatter
	Margin                First_Dynamic
	LatexType             Environment
	InTitle               1
	LatexName             abstract
	LeftMargin            MMM
	LabelIndent           MMM
	RightMargin           MMM
	ParIndent             MM
	TopSep                0.7
	BottomSep             0.7
	Align                 Block
	LabelType             Static
	LabelString           "Abstract."
	LabelSep              M
	Font
	  Size                Small
	EndFont
	LabelFont
	  Shape               Smallcaps
	EndFont
	DocBookTag            para
	DocBookWrapperTag     abstract
	DocBookWrapperMergeWithPrevious true
	DocBookInInfo         always
End

# Clear Address definition from lyxmacros
NoStyle Address

Style Address
	Category              FrontMatter
	Margin                Dynamic
	LatexType             Command
	InTitle               1
	LatexName             address
	ParSkip               0.4
	BottomSep             1.5
	ParSep                1.5
	Align                 Left
	AlignPossible         Block, Left, Right, Center
	LabelType             Static
	LabelString           "Address:"
	LabelSep              M
	LabelFont
	  Shape               Italic
	EndFont
	DocBookTag            address
	DocBookTagType        paragraph
	DocBookWrapperTag     author
	DocBookWrapperMergeWithPrevious true
	DocBookInInfo         always
End


Style Current_Address
	CopyStyle             Address
	LatexName             curraddr
	LabelString           "Current address:"
	DocBookTag            address
	DocBookTagType        paragraph
	DocBookInInfo         always
	DocBookAttr           role='current'
End


Style Email
	CopyStyle             Address
	Margin                First_Dynamic
	LatexName             email
	LabelString           "E-mail address:"
	DocBookTag            email
	DocBookTagType        paragraph
	DocBookWrapperTag     author
	DocBookWrapperMergeWithPrevious true
	DocBookInInfo         always
End


Style URL
	CopyStyle             Address
	Margin                First_Dynamic
	LatexName             urladdr
	LabelString           "URL:"
	DocBookTag            uri
	DocBookTagType        paragraph
	DocBookWrapperTag     author
	DocBookWrapperMergeWithPrevious true
	DocBookInInfo         always
End


Style Keywords
	CopyStyle             Address
	LatexName             keywords
	LabelString           "Key words and phrases:"
	DocBookTag            keyword
	DocBookTagType        paragraph
	DocBookWrapperTag     keywordset
	DocBookWrapperMergeWithPrevious true
	DocBookInInfo         always
End


Style Thanks
	CopyStyle             Address
	LatexName             thanks
	LabelString           "Thanks:"
	DocBookTag            personname
	DocBookTagType        paragraph
	DocBookWrapperTag     othercredit
	DocBookWrapperMergeWithPrevious true
	DocBookInInfo         always
End


Style Dedicatory
	CopyStyle             Address
	LatexName             dedicatory
	LabelString           "Dedication:"
	DocBookTag            para
	DocBookWrapperTag     dedication
	DocBookWrapperMergeWithPrevious true
	DocBookInInfo         never
End


Style Translator
	CopyStyle             Address
	LatexName             translator
	LabelString           "Translator:"
	DocBookTag            personname
	DocBookTagType        paragraph
	DocBookWrapperTag     othercredit
	DocBookWrapperAttr    role='translator'
	DocBookWrapperMergeWithPrevious true
	DocBookInInfo         always
End


Style Subjectclass
	CopyStyle             Address
	LatexName             subjclass[2000]
	LabelString           "2000 Mathematics Subject Classification:"
	DocBookItemTag        subjectterm
	DocBookItemTagType    inline
	DocBookTag            subject
	DocBookTagType        paragraph
	DocBookWrapperTag     subjectset
	DocBookWrapperMergeWithPrevious true
	DocBookInInfo         always
End


Style Bibliography
	Category              BackMatter
	Margin                First_Dynamic
	LatexType             Bib_Environment
	LatexName             thebibliography
	NextNoIndent          1
	ToggleIndent          Never
	LeftMargin            MM
	ParSkip               0.4
	ItemSep               0
	TopSep                0.7
	BottomSep             0.7
	ParSep                0.5
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Bibliography
	LabelString           "References"
	LabelBottomSep        0.7
	Font
	  Size                Small
	EndFont
	LabelFont
	  Size                Normal
	  Shape               Smallcaps
	EndFont
	TocLevel             1
	IsTocCaption         0
	Argument listpreamble:1
	  LabelString    "Bib preamble"
	  MenuString     "Bibliography Preamble"
	  Tooltip        "LaTeX code to be inserted before the first bibliography item"
	  PassThru           1
	  Font
	    Family           typewriter
	    Color            latex
	  EndFont
	EndArgument
End
