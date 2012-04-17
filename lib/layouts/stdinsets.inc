# Textclass definition file for LaTeX.
# Author : Martin vermeer <martin.vermeer@hut.fi>
# Inset layouts definition
#
# Detailled format description is available in the customization manual

Format 35

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

InsetLayout Note:Comment
	LabelString           Comment
	LatexType             environment
	LatexName             comment
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
	HTMLIsBlock           false
# FIXME HTML Need CSS
End


InsetLayout Note:Greyedout
	LabelString           Greyedout
	LatexType             environment
	LatexName             lyxgreyedout
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
	BgColor               listingsbg
	MultiPar              true
	PassThru              true
	ParbreakIsNewline     true
	KeepEmpty             true
	FreeSpacing           true
	ForceLTR              true
	RefPrefix             lst
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
	LabelString           opt
	LabelFont
	  Color               collapsable
	  Size                Small
	EndFont
	MultiPar              false
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

InsetLayout Caption
	HTMLStyle
		div.float-caption {
			text-align: center;
			border: 2px solid black;
			padding: 1ex;
			margin: 1ex;
		}
	EndHTMLStyle
End


InsetLayout Preview
	LabelString           Preview
	Decoration            minimalistic
	MultiPar              true
End

InsetLayout Script:superscript
	MultiPar false
	HTMLTag sup
	HTMLIsBlock 0
End

InsetLayout Script:subscript
	MultiPar false
	HTMLTag sub
	HTMLIsBlock 0
End
