# The special "LyXList". Separated out for inclusion where the
# whole of stdlist.inc is not needed.

Format 93

Style Labeling
	Category              List
	Margin                Manual
	LatexType             List_Environment
	LatexName             lyxlist
	NextNoindent          1
	LabelSep              xxx
	ParSkip               0.4
	TopSep                0.7
	BottomSep             0.7
	ParSep                0.5
	Align                 Block
	AlignPossible         Block, Left
	LabelType             Manual
	LabelString           "00.00.0000"
	Argument listpreamble:1
		LabelString    "List preamble"
		MenuString     "List Preamble"
		Tooltip        "LaTeX code to be inserted before the first item"
		PassThru       1
		Font
			Family typewriter
			Color latex
		EndFont
	EndArgument
	HTMLTag               ol
	HTMLAttr              class='lyxlist'
	HTMLItem              li
	HTMLLabel             span
	HTMLLabelAttr         class='lyxlist'

	#define the environment lyxlist
	Preamble
		\newenvironment{lyxlist}[1]
			{\begin{list}{}
				{\settowidth{\labelwidth}{#1}
				 \setlength{\leftmargin}{\labelwidth}
				 \addtolength{\leftmargin}{\labelsep}
				 \renewcommand{\makelabel}[1]{##1\hfil}}}
			{\end{list}}
	EndPreamble

	HTMLStyle
		ol.lyxlist {
			list-style-type: none;
		}
		li.labeling_item {
			text-indent: -5em;
			margin-left: 5em;
		}
		span.lyxlist {
			margin-right: 1em;
		}
	EndHTMLStyle
	
	DocBookTag            variablelist
	DocBookItemWrapperTag varlistentry
	DocBookItemTag        listitem
	DocBookItemInnerTag   para
	DocBookItemLabelTag   term
	DocBookItemLabelTagType   paragraph
End
