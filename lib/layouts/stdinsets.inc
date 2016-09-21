# Textclass definition file for LaTeX.
# Author : Martin vermeer <martin.vermeer@hut.fi>
# Inset layouts definition
#
# Detailed format description is available in the customization manual

Format 60

Provides stdinsets 1

InsetLayout Marginal
	LabelString           margin
	LatexType             command
	LatexName             marginpar
	Font
	  Size                Small
	EndFont
	LabelFont
	  Color               marginlabel
	  Size                Small
	EndFont
	MultiPar              true
	NeedProtect           true
	HTMLStyle
		div.marginal {
			border: 2px solid black;
			padding: 1ex;
			margin: 1ex;
			background-color: #F0F0F0;
			float:right;
			font-size: small;
			font-weight: medium;
			font-family: serif;
			font-variant: normal;
			font-style: normal;
		}
	EndHTMLStyle
End

InsetLayout Foot
	LabelString           foot
	LatexType             Command
	LatexName             footnote
	Counter               footnote
	Font
	  Size                Small
	EndFont
	LabelFont
	  Color               footlabel
	  Size                Small
	EndFont
	MultiPar              true
	RefPrefix             fn
	HTMLLabel             \arabic{footnote}
	HTMLInnerTag          div
	HTMLStyle
		span.foot_label {
			vertical-align: super;
			font-size: smaller;
			font-weight: bold;
			text-decoration: underline;
		}
		div.foot {
			display: inline;
			font-size: small;
			font-weight: medium;
			font-family: serif;
			font-variant: normal;
			font-style: normal;
		}
		div.foot_inner { display: none; }
		div.foot:hover div.foot_inner {
			display: block;
			border: 1px double black;
			margin: 0em 1em;
			padding: 1em;
		}
	EndHTMLStyle
End

InsetLayout Foot:InTitle
	CopyStyle             Foot
	LatexName             thanks
	NeedProtect           true
	Counter               thanks
	MultiPar              false
	HTMLLabel             \fnsymbol{thanks}
	HTMLInnerTag          span
	HTMLStyle
		span.foot_intitle_label {
			vertical-align: super;
			font-size: smaller;
			font-weight: bold;
			text-decoration: underline;
		}
		span.foot_intitle {
			display: inline;
			font-size: small;
			font-weight: medium;
			font-family: serif;
			font-variant: normal;
			font-style: normal;
		}
		span.foot_intitle_inner { display: none; }
		span.foot_intitle:hover span.foot_intitle_inner {
			display: block;
			border: 1px double black;
			margin: 0em 1em;
			padding: 1em;
		}
	EndHTMLStyle
End

InsetLayout Note:Comment
	LabelString           Comment
	LatexType             environment
	LatexName             comment
	Requires	      verbatim
	BgColor               commentbg
	LabelFont
	  Color               comment
	  Size                Small
	EndFont
	MultiPar              true
	Display               false
	HTMLTag               div
	HTMLIsBlock           true
	HTMLStyle
	div.note_comment {
		display: none;
	}
	EndHTMLStyle
End


InsetLayout Note:Note
	LabelString           Note
	LatexType             command
	LatexName             note
	BgColor               notebg
	LabelFont
	  Color               note
	  Size                Small
	EndFont
	MultiPar              true
	# Not output for XHTML
End


InsetLayout Note:Greyedout
	LabelString           Greyedout
	LatexType             environment
	LatexName             lyxgreyedout
	Requires	      color,lyxgreyedout
	BgColor               greyedoutbg
	Font
	  Color               greyedouttext
	EndFont
	LabelFont
	  Color               greyedout
	  Size                Small
	EndFont
	MultiPar              true
	Display               false
	HTMLInnerTag div
	HTMLStyle
		div.note_greyedout {
			display: inline;
			color: #A0A0A0;
			padding-left: 1ex;
			padding-right: 1ex;
		}
	EndHTMLStyle
	HTMLIsBlock           false
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
	CustomPars            false
	ForcePlain            true
	PassThru              true
	ParbreakIsNewline     true
	KeepEmpty             true
	FreeSpacing           true
	ForceLTR              true
End

InsetLayout Phantom
	Decoration            minimalistic
	Font
	  Color               phantomtext
	EndFont
	MultiPar              false
	CustomPars            false
	ForcePlain            true
End

InsetLayout IPADeco
	Decoration            minimalistic
	Font
	  Color               foreground
	EndFont
	MultiPar              false
	CustomPars            false
	ForcePlain            true
End

InsetLayout TOC
  HTMLTag h2
  HTMLStyle
    div.toc {
      margin: 2em 0em;
      border-style: solid;
      border-width: 2px 0px;
      padding: 1em 0em;
    }
    h2.tochead { font-size: x-large; font-weight: bold; }
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
      margin: 0em 0.1em 0em 1em;
      font-size: large;
      font-weight: normal;
    }
    div.lyxtoc-3 { margin: 0em 0.1em 0em 0.5em; font-size: medium; }
    div.lyxtoc-4 { margin: 0em 0.1em 0em 0.5em; }
    div.lyxtoc-5 { margin: 0em 0.1em 0em 0.5em; }
    div.lyxtoc-6 { margin: 0em 0.1em 0em 0.5em; }
    a.tocentry {
      text-decoration: none;
      color: black;
    }
    a.tocentry:visited { color: black; }
  EndHTMLStyle
End

InsetLayout TOC:Listings
	# We need the [[List of Listings]] context, since "Listings" is also
	# the name of the inset and translated differently.
	# "Listings[[List of Listings]]" is the name of the "List of listings"
	# ("Listings" is the predefined english name) in listings.sty, so it
	# must be used here as well.
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\lstlistlistingname}{_(Listings[[List of Listings]])}}
	EndBabelPreamble
	# The command does not need to be defined in LangPreamble, since
	# listings.sty does that already. However it needs to be redefined
	# in order to be used for non-english single-language documents.
	LangPreamble
		\renewcommand{\lstlistlistingname}{_(Listings[[List of Listings]])}
	EndLangPreamble
	FixedWidthPreambleEncoding true
  HTMLTag h2
  HTMLStyle
    div.lyxtoc-flat {
      margin: 0em 0em 0em 1em;
      font-size: large;
      font-weight: normal;
    }
  EndHTMLStyle
End

InsetLayout Include:Listings
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\lstlistingname}{_(Listing)}}
	EndBabelPreamble
	# The command does not need to be defined in LangPreamble, since
	# listings.sty does that already. However it needs to be redefined
	# in order to be used for non-english single-language documents.
	LangPreamble
		\renewcommand{\lstlistingname}{_(Listing)}
	EndLangPreamble
	FixedWidthPreambleEncoding true
End

InsetLayout Listings
	CopyStyle Include:Listings
	LabelString           Listings[[inset]]
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
	BgColor               listingsbg
	MultiPar              true
	PassThru              true
	ParbreakIsNewline     true
	KeepEmpty             true
	FreeSpacing           true
	ForceLTR              true
	RefPrefix             lst
	HTMLTag               pre
	HTMLStyle
		div.float-listings {
			border: 2px solid black;
			padding: 1ex;
			margin: 1ex;
		}
		div.listings-caption {
			text-align: center;
			border: 2px solid black;
			padding: 1ex;
			margin: 1ex;
	}
	EndHTMLStyle
End

InsetLayout Branch
	Decoration            classic
	LabelFont
	  Color               branchlabel
	  Size                Small
	EndFont
	MultiPar              true
	InToc                 true
	HTMLIsBlock           false
End

InsetLayout Index
	LabelString           Idx
	Decoration            classic
	Font
	  Size                Small
	EndFont
	LabelFont
	  Color               indexlabel
	  Size                Small
	EndFont
	MultiPar              false
	CustomPars            false
	ForcePlain            true
	ContentAsLabel        true
End

InsetLayout Box
	LabelFont
	  Color               foreground
	  Size                Small
	EndFont
	MultiPar              true
End

InsetLayout Box:Frameless
	CopyStyle Box
	HTMLStyle
		div.Frameless { margin: 1em; }
	EndHTMLStyle
End

InsetLayout Box:Shaded
	CopyStyle Box
	BgColor shaded
	HTMLStyle
		div.Shaded {
			background-color: gray;
			padding: 0.5ex;
		}
	EndHTMLStyle
End

InsetLayout Box:Boxed
	CopyStyle Box
	HTMLStyle
		div.Boxed {
			border: solid thick black;
			padding: 0.5ex;
		}
	EndHTMLStyle
End

InsetLayout Box:Framed
	CopyStyle Box
	HTMLStyle
		div.Framed {
			border: solid thick black;
			padding: 0.5ex;
		}
	EndHTMLStyle
End

InsetLayout Box:ovalbox
	CopyStyle Box
	HTMLStyle
		div.ovalbox {
			border: groove medium black;
			padding: 0.5ex;
		}
	EndHTMLStyle
End

InsetLayout Box:Ovalbox
	CopyStyle Box
	HTMLStyle
		div.Ovalbox {
			border: ridge thick black;
			padding: 0.5ex;
		}
	EndHTMLStyle
End

InsetLayout Box:Shadowbox
	CopyStyle Box
	HTMLStyle
		div.Shadowbox { border: solid gray medium; border-bottom: solid black 1ex; border-right: solid black 1ex; padding: 0.5ex; }
	EndHTMLStyle
End

InsetLayout Box:Doublebox
	CopyStyle Box
	HTMLStyle
		div.Doublebox {
			border: double thick black;
			padding: 0.5ex;
		}
	EndHTMLStyle
End

InsetLayout Float
	LabelFont
	  Color               collapsable
	  Size                Small
	EndFont
	MultiPar              true
	HTMLStyle
		div.float {
			border: 2px solid black;
			text-align: center;
		}
	EndHTMLStyle
End

InsetLayout Wrap
	LabelFont
	  Color               collapsable
	  Size                Small
	EndFont
	MultiPar              true
	RefPrefix             wrap
	HTMLStyle
		div.wrap {
			float: right;
			border: 2px solid black;
			padding: 1ex;
			margin: 1ex;
		}
	EndHTMLStyle
End

InsetLayout "Flex:URL"
	LyXType               standard
	Decoration            classic
	LabelString           URL
	ContentAsLabel        true
	LatexName             url
	LatexType             command
	Requires              url
	MultiPar              false
	CustomPars            false
	ForcePlain            true
	PassThru              true
	FreeSpacing           true
	ForceLTR              true
	Font
	  Family              Typewriter
	  Color               urltext
	EndFont
	LabelFont
	  Family              Typewriter
	  Color               urllabel
	  Size                Small
	EndFont
	HTMLIsBlock           false
End

InsetLayout Argument
	LabelString           Argument
	LabelFont
	  Color               collapsable
	  Size                Small
	EndFont
	MultiPar              false
	ForceLocalFontSwitch  1
End

InsetLayout Info
	Decoration	      conglomerate
	HTMLStyle
		span.info { font-family: sans-serif; }
	EndHTMLStyle
	HTMLTag               span
	MultiPar              false
End

InsetLayout Info:menu
	CopyStyle             Info
	LatexType             command
	LatexName             menuitem
	Preamble
	  \providecommand{\menuitem}[1]{\textsf{#1}}
	EndPreamble
	Decoration	      conglomerate
	Font
	  Family              sans
	EndFont
	HTMLAttr              "class='info menu'"
	HTMLStyle
		span.menu { font-family: sans-serif; }
	EndHTMLStyle
End

InsetLayout Info:shortcut
	CopyStyle             Info
	LatexType             command
	LatexName             shortcut
	Preamble
	  \providecommand{\shortcut}[1]{\mbox{\textsf{#1}}}
	EndPreamble
	Decoration	      conglomerate
	Font
	  Family              sans
	EndFont
	HTMLAttr              "class='info shortcut'"
	HTMLStyle
		span.shortcut { font-family: sans-serif; }
	EndHTMLStyle
End

InsetLayout Info:shortcuts
	CopyStyle             Info
	LatexType             command
	LatexName             shortcut
	Preamble
	  \providecommand{\shortcut}[1]{\textsf{#1}}
	EndPreamble
	Decoration	      conglomerate
	Font
	  Family              sans
	EndFont
	HTMLAttr              "class='info shortcut'"
	HTMLStyle
		span.shortcuts { font-family: sans-serif; }
	EndHTMLStyle
End

InsetLayout Caption:Standard
	LabelString          standard
	LaTeXType            command
	LatexName            caption
	NeedProtect	     1
	MultiPar             false
	Argument 1
		LabelString   "Short Title|S"
		Tooltip       "The caption as it appears in the list of figures/tables"
		InsertCotext  1
	EndArgument
	HTMLStyle
		div.float-caption {
			text-align: center;
			border: 2px solid black;
			padding: 1ex;
			margin: 1ex;
		}
	EndHTMLStyle
	HTMLAttr "class='float-caption float-caption-standard'"
End


InsetLayout Caption:Unnumbered
	CopyStyle            Caption:Standard
	LabelString          unlabelled
	LatexName            caption*
	ResetArgs            1
End


InsetLayout Preview
	LabelString           Preview
	Decoration            minimalistic
	MultiPar              true
End


InsetLayout Script
	LatexType       command
	MultiPar        false
	ForcePlain      1
	CustomPars      0
	InToc           1
	HTMLIsBlock     0
End


InsetLayout Script:superscript
	CopyStyle       Script
	LaTeXName       textsuperscript
	HTMLTag         sup
End


InsetLayout Script:subscript
	CopyStyle       Script
	Requires        subscript
	LaTeXName       textsubscript
	HTMLTag         sub
End


InsetLayout PrintNomencl
  HTMLTag h2
End

InsetLayout Tabular
	HTMLStyle
		table {
			border-collapse: collapse;
			display: inline-block;
		}
		td {
			border: 1px solid black;
			padding: 0.5ex;
		}
	EndHTMLStyle
End
