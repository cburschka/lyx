# Japanese jlreq textclass definition file.
# Author : Koji Yokota (yokota6@gmail.com)
#
# This class internally loads following packages:
# Any time:			 	l3keys2e, lmodern
# With non-LuaLaTeX:	everyhook
# With LuaLaTeX:		luatexja, luatexja-adjust

Format 99

Style Part
	Argument post:1
		LabelString		"Subtitle|u"
		Tooltip			"Subtitle of the part"
		InsertCotext	1
	EndArgument
End

Style Part*
	Argument post:1
		LabelString		"Subtitle|u"
		Tooltip			"Subtitle of the part"
		InsertCotext	1
	EndArgument
End

Style Chapter
	Argument post:1
		LabelString		"Subtitle|u"
		Tooltip			"Subtitle of the chapter"
		InsertCotext	1
	EndArgument
End

Style Chapter*
	Argument post:1
		LabelString		"Subtitle|u"
		Tooltip			"Subtitle of the chapter"
		InsertCotext	1
	EndArgument
End

Style Section
	Argument post:1
		LabelString		"Subtitle|u"
		Tooltip			"Subtitle of the section"
		InsertCotext	1
	EndArgument
End

Style Section*
	Argument post:1
		LabelString		"Subtitle|u"
		Tooltip			"Subtitle of the section"
		InsertCotext	1
	EndArgument
End

Style Subsection
	Argument post:1
		LabelString		"Subtitle|u"
		Tooltip			"Subtitle of the subsection"
		InsertCotext	1
	EndArgument
End

Style Subsection*
	Argument post:1
		LabelString		"Subtitle|u"
		Tooltip			"Subtitle of the subsection"
		InsertCotext	1
	EndArgument
End

Style Subsubsection
	Argument post:1
		LabelString		"Subtitle|u"
		Tooltip			"Subtitle of the subsubsection"
		InsertCotext	1
	EndArgument
End

Style Subsubsection*
	Argument post:1
		LabelString		"Subtitle|u"
		Tooltip			"Subtitle of the subsubsection"
		InsertCotext	1
	EndArgument
End

Style Abstract_over_Columns
	CopyStyle			Abstract
	InPreamble			1
	LabelString			"Abstract"
	Category			FrontMatter
	LatexType			Command
	LatexName			jlreqabstract
	Preamble
		\newcommand{\jlreqabstract}[1]{%
		\begin{abstract}%
		#1%
		\end{abstract}%
		}
	EndPreamble
End

Style JLReq_Setup
	InPreamble			1
	LatexType			Command
	LatexName			jlreqsetup
	Category			FrontMatter
	LabelType			Static
	LabelString			"JLReq Setup"
	LabelSep			"M"
	LeftMargin			"MMMMMMM"
	Font
		Family			Typewriter
		Size			Small
	EndFont
	LabelFont
		Color			eolmarker
		Family			Sans
	EndFont
	ParagraphGroup		1
	PassThru			1
	HTMLTag				!--
End

InsetLayout Flex:Endnote
	LyxType				custom
	LatexType			Command
	LatexName			endnote
	Counter				footnote
	Decoration			Classic
	LabelString			"Endnote"
	ForcePlain			1
	BgColor				background
	ResetsFont true
	HTMLLabel			\arabic{footnote}
	HTMLInnerTag        div
	HTMLStyle
		span.flex_endnote_label {
			vertical-align: super;
			font-size: smaller;
			font-weight: bold;
			text-decoration: underline;
		}
		div.flex_endnote {
			display: inline;
			font-size: small;
			font-weight: normal;
			font-family: serif;
			font-variant: normal;
			font-style: normal;
		}
		div.flex_endnote_inner { display: none; }
		div.flex_endnote:hover div.flex_endnote_inner {
			display: block;
			border: 1px double black;
			margin: 0em 1em;
			padding: 1em;
		}
	EndHTMLStyle
	AddToToc              footnote
	IsTocCaption          true
	DocBookTag            footnote
	DocBookTagType        inline
End

InsetLayout Flex:Sidenote
	CopyStyle			Flex:Endnote
	LatexName			sidenote
	LabelString			"Sidenote"
	HTMLLabel			\arabic{footnote}
	HTMLInnerTag        div
	HTMLStyle
		span.flex_sidenote_label {
			vertical-align: super;
			font-size: smaller;
			font-weight: bold;
			text-decoration: underline;
		}
		div.flex_sidenote {
			display: inline;
			font-size: small;
			font-weight: normal;
			font-family: serif;
			font-variant: normal;
			font-style: normal;
		}
		div.flex_sidenote_inner { display: none; }
		div.flex_sidenote:hover div.flex_sidenote_inner {
			display: block;
			border: 1px double black;
			margin: 0em 1em;
			padding: 1em;
		}
	EndHTMLStyle
	Argument post:1
		LabelString		"Note"
		MenuString		"Endnote contents"
		Tooltip			"Contents of the sidenote. Only makes sense when \jlreqsetup{sidenote_type=symbol} is specified in the preamble."
		Mandatory		0
		LeftDelim		{
		RightDelim		}
	EndArgument
End

InsetLayout Flex:Warichu
	CopyStyle			Flex:Endnote
	LatexName			warichu
	LabelString			"Warichu"
	HTMLLabel			\arabic{footnote}
	HTMLInnerTag        div
	HTMLStyle
		span.flex_warichu_label {
			vertical-align: super;
			font-size: smaller;
			font-weight: bold;
			text-decoration: underline;
		}
		div.flex_warichu {
			display: inline;
			font-size: small;
			font-weight: normal;
			font-family: serif;
			font-variant: normal;
			font-style: normal;
		}
		div.flex_warichu_inner { display: none; }
		div.flex_warichu:hover div.flex_warichu_inner {
			display: block;
			border: 1px double black;
			margin: 0em 1em;
			padding: 1em;
		}
	EndHTMLStyle
End

InsetLayout Flex:Warichu*
	CopyStyle			Flex:Warichu
	LatexName			warichu*
	LabelString			"Warichu*"
	PassThru			1
	MultiPar			1
	ParbreakIgnored		1
	TextFont
		Family			Typewriter
	EndFont
	HTMLLabel			\arabic{footnote}
	HTMLInnerTag        div
	HTMLStyle
		span.flex_warichu__label {
			vertical-align: super;
			font-size: smaller;
			font-weight: bold;
			text-decoration: underline;
		}
		div.flex_warichu_ {
			display: inline;
			font-size: small;
			font-weight: normal;
			font-family: serif;
			font-variant: normal;
			font-style: normal;
		}
		div.flex_warichu__inner { display: none; }
		div.flex_warichu_:hover div.flex_warichu__inner {
			display: block;
			border: 1px double black;
			margin: 0em 1em;
			padding: 1em;
		}
	EndHTMLStyle
End

InsetLayout Flex:Tatechuyoko
	LatexType			Command
	LyxType				charstyle
	LatexName			tatechuyoko*
	LabelString			"Tatechuyoko"
	Decoration			Conglomerate
End

InsetLayout Flex:Jidori
	CopyStyle			Flex:Tatechuyoko
	LatexName			jidori
	LabelString			"Jidori"
	Argument 1
		LabelString		"Length"
		MenuString		"Jidori Length|L"
		Tooltip			"Length of Jidori. Contents is fit to this length."
		Mandatory		1
		AutoInsert		1
	EndArgument
End

InsetLayout Flex:Akigumi
	CopyStyle			Flex:Tatechuyoko
	LatexName			akigumi
	LabelString			"Akigumi"
	MenuString			"Akigumi (LuaLaTeX)"
	Argument 1
		LabelString		"Char Space"
		MenuString		"Char Space|D"
		Tooltip			"Distance between chars is set to this length."
		Mandatory		1
		AutoInsert		1
	EndArgument
End
