# Standard textclass definition file. Taken from initial LyX source code
# Author : Matthias Ettrich <ettrich@informatik.uni-tuebingen.de>
# Transposed by Pascal André <andre@via.ecp.fr>
# Heavily modifed and enhanced by several developers.
# Split from stdstruct.inc by Hartmut Goebel <hartmut@noris.net>

# This include files contains all layouts that handle the title area
# a document, like title, author and such.


Format 35

# CSS for the TOC
AddToHTMLPreamble
<style type="text/css">
	div.toc {
		margin: 2em 0em;
		border-style: solid;
		border-width: 2px 0px;
		padding: 1em 0em;
	}
	div.tochead { font-size: x-large; font-weight: bold; }
	div.lyxtoc-0 {
		margin: 2em 0em 0em 0em;
		font-size: xx-large;
		font-weight: bold;
	}
	div.lyxtoc-1 {
		margin: 1em 0em 0em 0em;
		font-size: x-large;
		font-weight: bold;
	}
	div.lyxtoc-2 {
		margin: 0em 0em 0em 1em;
		font-size: large;
		font-weight: normal;
	}
	div.lyxtoc-3 { margin: 0em 0em 0em 0.5em; font-size: medium; }
	div.lyxtoc-4 { margin: 0em 0em 0em 0.5em; }
	div.lyxtoc-5 { margin: 0em 0em 0em 0.5em; }
	div.lyxtoc-6 { margin: 0em 0em 0em 0.5em; }
	a.tocentry {
		text-decoration: none;
		color: black;
	}
	a.tocentry:visited { color: black; }
	a.tocarrow {
		font-weight: bold;
		text-decoration: none;
		color: #909090;
	}
	a.tocarrow:visited { color: #C0C0C0; }
</style>
EndPreamble


Style Title
	Margin                Static
	LatexType             Command
	Category              FrontMatter
	InTitle               1
	LatexName             title
	ParSkip               0.4
	ItemSep               0
	TopSep                0
	BottomSep             1
	ParSep                1
	Align                 Center
	LabelType             No_Label
	Font
	  Size                Largest
	EndFont
	HTMLTag               h1
	HTMLTitle             true
End


Style Author
	Margin                Static
	LatexType             Command
	Category              FrontMatter
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
	EndFont
End


Style Date
	Margin                Dynamic
	LatexType             Command
	Category              FrontMatter
	InTitle               1
	LatexName             date
	LabelSep              xxx
	ParSkip               0.4
	TopSep                0.9
	BottomSep             0.5
	ParSep                1
	Align                 Center
	LabelType             No_Label
	Font
	  Size                Large
	EndFont
End
