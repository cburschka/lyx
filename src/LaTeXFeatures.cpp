/**
 * \file LaTeXFeatures.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Jürgen Vigna
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LaTeXFeatures.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "ColorSet.h"
#include "Converter.h"
#include "Encoding.h"
#include "Floating.h"
#include "FloatList.h"
#include "Language.h"
#include "LaTeXFonts.h"
#include "LaTeXPackages.h"
#include "Layout.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "TextClass.h"
#include "TexRow.h"
#include "texstream.h"

#include "insets/InsetLayout.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/regex.h"

#include <algorithm>


using namespace std;
using namespace lyx::support;


namespace lyx {

/////////////////////////////////////////////////////////////////////
//
// Strings
//
/////////////////////////////////////////////////////////////////////

//\NeedsTeXFormat{LaTeX2e}
//\ProvidesPackage{lyx}[1996/01/11 LLE v0.2 (LyX LaTeX Extensions)]
//\message{LyX LaTeX Extensions (LLE v0.2) of 11-Jan-1996.}

static docstring const lyx_def = from_ascii(
	"\\providecommand{\\LyX}{L\\kern-.1667em\\lower.25em\\hbox{Y}\\kern-.125emX\\@}");

static docstring const lyx_hyperref_def = from_ascii(
	"\\providecommand{\\LyX}{\\texorpdfstring%\n"
	"  {L\\kern-.1667em\\lower.25em\\hbox{Y}\\kern-.125emX\\@}\n"
	"  {LyX}}");

static docstring const noun_def = from_ascii(
	"\\newcommand{\\noun}[1]{\\textsc{#1}}");

static docstring const lyxarrow_def = from_ascii(
	"\\DeclareRobustCommand*{\\lyxarrow}{%\n"
	"\\@ifstar\n"
	"{\\leavevmode\\,$\\triangleleft$\\,\\allowbreak}\n"
	"{\\leavevmode\\,$\\triangleright$\\,\\allowbreak}}");

// ZERO WIDTH SPACE (ZWSP) is actually not a space character
// but marks a line break opportunity. Several commands provide a
// line break opportunity. They differ in side-effects:
// \allowbreak prevents hyphenation after hyphen or dash + ZWSP
// \linebreak[<n>] takes an optional argument denoting "urgency".
// The \LyXZeroWidthSpace wrapper allows customization in the preamble.
static docstring const lyxZWSP_def = from_ascii(
	"\\newcommand*\\LyXZeroWidthSpace{\\hspace{0pt}}");

// for quotes without babel. This does not give perfect results, but
// anybody serious about non-english quotes should use babel (JMarc).

static docstring const quotedblbase_def = from_ascii(
	"\\ProvideTextCommandDefault{\\quotedblbase}{%\n"
	"  \\raisebox{-1.4ex}[1ex][.5ex]{\\textquotedblright}%\n"
	"  \\penalty10000\\hskip0em\\relax%\n"
	"}");

static docstring const quotesinglbase_def = from_ascii(
	"\\ProvideTextCommandDefault{\\quotesinglbase}{%\n"
	"  \\raisebox{-1.4ex}[1ex][.5ex]{\\textquoteright}%\n"
	"  \\penalty10000\\hskip0em\\relax%\n"
	"}");

static docstring const guillemotleft_def = from_ascii(
	"\\ProvideTextCommandDefault{\\guillemotleft}{%\n"
	"  {\\usefont{U}{lasy}{m}{n}\\char'50\\kern-.15em\\char'50}%\n"
	"\\penalty10000\\hskip0pt\\relax%\n"
	"}");

static docstring const guillemotright_def = from_ascii(
	"\\ProvideTextCommandDefault{\\guillemotright}{%\n"
	"  \\penalty10000\\hskip0pt%\n"
	"  {\\usefont{U}{lasy}{m}{n}\\char'51\\kern-.15em\\char'51}%\n"
	"}");

static docstring const guilsinglleft_def = from_ascii(
	"\\ProvideTextCommandDefault{\\guilsinglleft}{%\n"
	"  {\\usefont{U}{lasy}{m}{n}\\char'50}%\n"
	"  \\penalty10000\\hskip0pt\\relax%\n"
	"}");

static docstring const guilsinglright_def = from_ascii(
	"\\ProvideTextCommandDefault{\\guilsinglright}{%\n"
	"  \\penalty10000\\hskip0pt%\n"
	"  {\\usefont{U}{lasy}{m}{n}\\char'51}%\n"
	"}");

static docstring const textquotedbl_def = from_ascii(
	"\\DeclareTextSymbolDefault{\\textquotedbl}{T1}");

static docstring const textquotedblp_xetex_def = from_ascii(
	"\\providecommand\\textquotedblplain{%\n"
	"  \\bgroup\\addfontfeatures{Mapping=}\\char34\\egroup}");

static docstring const textquotedblp_luatex_def = from_ascii(
	"\\providecommand\\textquotedblplain{%\n"
	"  \\bgroup\\addfontfeatures{RawFeature=-tlig}\\char34\\egroup}");

static docstring const textquotesinglep_xetex_def = from_ascii(
	"\\providecommand\\textquotesingleplain{%\n"
	"  \\bgroup\\addfontfeatures{Mapping=}\\char39\\egroup}");

static docstring const textquotesinglep_luatex_def = from_ascii(
	"\\providecommand\\textquotesingleplain{%\n"
	"  \\bgroup\\addfontfeatures{RawFeature=-tlig}\\char39\\egroup}");

static docstring const paragraphleftindent_def = from_ascii(
	"\\newenvironment{LyXParagraphLeftIndent}[1]%\n"
	"{\n"
	"  \\begin{list}{}{%\n"
	"    \\setlength{\\topsep}{0pt}%\n"
	"    \\addtolength{\\leftmargin}{#1}\n"
// ho hum, yet more things commented out with no hint as to why they
// weren't just removed
//	"%%    \\addtolength{\\leftmargin}{#1\\textwidth}\n"
//	"%%    \\setlength{\\textwidth}{#2\\textwidth}\n"
//	"%%    \\setlength\\listparindent\\parindent%\n"
//	"%%    \\setlength\\itemindent\\parindent%\n"
	"    \\setlength{\\parsep}{0pt plus 1pt}%\n"
	"  }\n"
	"  \\item[]\n"
	"}\n"
	"{\\end{list}}\n");

static docstring const floatingfootnote_def = from_ascii(
	"%% Special footnote code from the package 'stblftnt.sty'\n"
	"%% Author: Robin Fairbairns -- Last revised Dec 13 1996\n"
	"\\let\\SF@@footnote\\footnote\n"
	"\\def\\footnote{\\ifx\\protect\\@typeset@protect\n"
	"    \\expandafter\\SF@@footnote\n"
	"  \\else\n"
	"    \\expandafter\\SF@gobble@opt\n"
	"  \\fi\n"
	"}\n"
	"\\expandafter\\def\\csname SF@gobble@opt \\endcsname{\\@ifnextchar[%]\n"
	"  \\SF@gobble@twobracket\n"
	"  \\@gobble\n"
	"}\n"
	"\\edef\\SF@gobble@opt{\\noexpand\\protect\n"
	"  \\expandafter\\noexpand\\csname SF@gobble@opt \\endcsname}\n"
	"\\def\\SF@gobble@twobracket[#1]#2{}\n");

static docstring const binom_def = from_ascii(
	"%% Binom macro for standard LaTeX users\n"
	"\\newcommand{\\binom}[2]{{#1 \\choose #2}}\n");

static docstring const mathcircumflex_def = from_ascii(
	"%% For printing a cirumflex inside a formula\n"
	"\\newcommand{\\mathcircumflex}[0]{\\mbox{\\^{}}}\n");

static docstring const tabularnewline_def = from_ascii(
	"%% Because html converters don't know tabularnewline\n"
	"\\providecommand{\\tabularnewline}{\\\\}\n");

static docstring const lyxgreyedout_def = from_ascii(
	"%% The greyedout annotation environment\n"
	"\\newenvironment{lyxgreyedout}\n"
	"  {\\textcolor{note_fontcolor}\\bgroup\\ignorespaces}\n"
	"  {\\ignorespacesafterend\\egroup}\n");

// We want to omit the file extension for includegraphics, but this does not
// work when the filename contains other dots.
// Idea from http://www.tex.ac.uk/cgi-bin/texfaq2html?label=unkgrfextn
static docstring const lyxdot_def = from_ascii(
	"%% A simple dot to overcome graphicx limitations\n"
	"\\newcommand{\\lyxdot}{.}\n");

static docstring const changetracking_dvipost_def = from_ascii(
	"%% Change tracking with dvipost\n"
	"\\dvipostlayout\n"
	"\\dvipost{osstart color push Red}\n"
	"\\dvipost{osend color pop}\n"
	"\\dvipost{cbstart color push Blue}\n"
	"\\dvipost{cbend color pop}\n"
	"\\DeclareRobustCommand{\\lyxadded}[3]{\\changestart#3\\changeend}\n"
	"\\DeclareRobustCommand{\\lyxdeleted}[3]{%\n"
	"\\changestart\\overstrikeon#3\\overstrikeoff\\changeend}\n");

static docstring const changetracking_xcolor_ulem_def = from_ascii(
	"%% Change tracking with ulem\n"
	"\\DeclareRobustCommand{\\lyxadded}[3]{{\\color{lyxadded}{}#3}}\n"
	"\\DeclareRobustCommand{\\lyxdeleted}[3]{{\\color{lyxdeleted}\\lyxsout{#3}}}\n"
	"\\DeclareRobustCommand{\\lyxsout}[1]{\\ifx\\\\#1\\else\\sout{#1}\\fi}\n");

static docstring const changetracking_xcolor_ulem_hyperref_def = from_ascii(
	"%% Change tracking with ulem\n"
	"\\DeclareRobustCommand{\\lyxadded}[3]{{\\texorpdfstring{\\color{lyxadded}{}}{}#3}}\n"
	"\\DeclareRobustCommand{\\lyxdeleted}[3]{{\\texorpdfstring{\\color{lyxdeleted}\\lyxsout{#3}}{}}}\n"
	"\\DeclareRobustCommand{\\lyxsout}[1]{\\ifx\\\\#1\\else\\sout{#1}\\fi}\n");

static docstring const changetracking_tikz_math_sout_def = from_ascii(
	"%% Strike out display math with tikz\n"
	"\\usepackage{tikz}\n"
	"\\usetikzlibrary{calc}\n"
	"\\newcommand{\\lyxmathsout}[1]{%\n"
	"  \\tikz[baseline=(math.base)]{\n"
	"    \\node[inner sep=0pt,outer sep=0pt](math){#1};\n"
	"    \\draw($(math.south west)+(2em,.5em)$)--($(math.north east)-(2em,.5em)$);\n"
	"  }\n"
	"}\n");

static docstring const changetracking_none_def = from_ascii(
	"\\newcommand{\\lyxadded}[3]{#3}\n"
	"\\newcommand{\\lyxdeleted}[3]{}\n");

static docstring const textgreek_LGR_def = from_ascii(
	"\\DeclareFontEncoding{LGR}{}{}\n");
static docstring const textgreek_def = from_ascii(
	"\\DeclareRobustCommand{\\greektext}{%\n"
	"  \\fontencoding{LGR}\\selectfont\\def\\encodingdefault{LGR}}\n"
	"\\DeclareRobustCommand{\\textgreek}[1]{\\leavevmode{\\greektext #1}}\n"
        "\\ProvideTextCommand{\\~}{LGR}[1]{\\char126#1}\n");

static docstring const textcyr_T2A_def = from_ascii(
	"\\InputIfFileExists{t2aenc.def}{}{%\n"
        "  \\errmessage{File `t2aenc.def' not found: Cyrillic script not supported}}\n");
static docstring const textcyr_def = from_ascii(
	"\\DeclareRobustCommand{\\cyrtext}{%\n"
	"  \\fontencoding{T2A}\\selectfont\\def\\encodingdefault{T2A}}\n"
	"\\DeclareRobustCommand{\\textcyr}[1]{\\leavevmode{\\cyrtext #1}}\n");

static docstring const lyxmathsym_def = from_ascii(
	"\\newcommand{\\lyxmathsym}[1]{\\ifmmode\\begingroup\\def\\b@ld{bold}\n"
	"  \\text{\\ifx\\math@version\\b@ld\\bfseries\\fi#1}\\endgroup\\else#1\\fi}\n");

static docstring const papersizedvi_def = from_ascii(
	"\\special{papersize=\\the\\paperwidth,\\the\\paperheight}\n");

static docstring const papersizepdf_def = from_ascii(
	"\\pdfpageheight\\paperheight\n"
	"\\pdfpagewidth\\paperwidth\n");

static docstring const papersizepdflua_def = from_ascii(
	"% Backwards compatibility for LuaTeX < 0.90\n"
	"\\@ifundefined{pageheight}{\\let\\pageheight\\pdfpageheight}{}\n"
	"\\@ifundefined{pagewidth}{\\let\\pagewidth\\pdfpagewidth}{}\n"
	"\\pageheight\\paperheight\n"
	"\\pagewidth\\paperwidth\n");

static docstring const cedilla_def = from_ascii(
	"\\newcommand{\\docedilla}[2]{\\underaccent{#1\\mathchar'30}{#2}}\n"
	"\\newcommand{\\cedilla}[1]{\\mathpalette\\docedilla{#1}}\n");

static docstring const subring_def = from_ascii(
	"\\newcommand{\\dosubring}[2]{\\underaccent{#1\\mathchar'27}{#2}}\n"
	"\\newcommand{\\subring}[1]{\\mathpalette\\dosubring{#1}}\n");

static docstring const subdot_def = from_ascii(
	"\\newcommand{\\dosubdot}[2]{\\underaccent{#1.}{#2}}\n"
	"\\newcommand{\\subdot}[1]{\\mathpalette\\dosubdot{#1}}\n");

static docstring const subhat_def = from_ascii(
	"\\newcommand{\\dosubhat}[2]{\\underaccent{#1\\mathchar'136}{#2}}\n"
	"\\newcommand{\\subhat}[1]{\\mathpalette\\dosubhat{#1}}\n");

static docstring const subtilde_def = from_ascii(
	"\\newcommand{\\dosubtilde}[2]{\\underaccent{#1\\mathchar'176}{#2}}\n"
	"\\newcommand{\\subtilde}[1]{\\mathpalette\\dosubtilde{#1}}\n");

static docstring const dacute_def = from_ascii(
	"\\DeclareMathAccent{\\dacute}{\\mathalpha}{operators}{'175}\n");

static docstring const tipasymb_def = from_ascii(
	"\\DeclareFontEncoding{T3}{}{}\n"
	"\\DeclareSymbolFont{tipasymb}{T3}{cmr}{m}{n}\n");

static docstring const dgrave_def = from_ascii(
	"\\DeclareMathAccent{\\dgrave}{\\mathord}{tipasymb}{'15}\n");

static docstring const rcap_def = from_ascii(
	"\\DeclareMathAccent{\\rcap}{\\mathord}{tipasymb}{'20}\n");

static docstring const ogonek_def = from_ascii(
	"\\newcommand{\\doogonek}[2]{\\setbox0=\\hbox{$#1#2$}\\underaccent{#1\\mkern-6mu\n"
	"  \\ifx#2O\\hskip0.5\\wd0\\else\\ifx#2U\\hskip0.5\\wd0\\else\\hskip\\wd0\\fi\\fi\n"
	"  \\ifx#2o\\mkern-2mu\\else\\ifx#2e\\mkern-1mu\\fi\\fi\n"
	"  \\mathchar\"0\\hexnumber@\\symtipasymb0C}{#2}}\n"
	"\\newcommand{\\ogonek}[1]{\\mathpalette\\doogonek{#1}}\n");

static docstring const lyxaccent_def = from_ascii(
	"%% custom text accent \\LyxTextAccent[<rise value (length)>]{<accent>}{<base>}\n"
        "\\newcommand*{\\LyxTextAccent}[3][0ex]{%\n"
        "  \\hmode@bgroup\\ooalign{\\null#3\\crcr\\hidewidth\n"
        "  \\raise#1\\hbox{#2}\\hidewidth}\\egroup}\n"
        "%% select a font size smaller than the current font size:\n"
        "\\newcommand{\\LyxAccentSize}[1][\\sf@size]{%\n"
        "  \\check@mathfonts\\fontsize#1\\z@\\math@fontsfalse\\selectfont\n"
        "}\n");

static docstring const textcommabelow_def = from_ascii(
        "\\ProvideTextCommandDefault{\\textcommabelow}[1]{%%\n"
        "  \\LyxTextAccent[-.31ex]{\\LyxAccentSize,}{#1}}\n");

static docstring const textcommaabove_def = from_ascii(
        "\\ProvideTextCommandDefault{\\textcommaabove}[1]{%%\n"
        "  \\LyxTextAccent[.5ex]{\\LyxAccentSize`}{#1}}\n");

static docstring const textcommaaboveright_def = from_ascii(
        "\\ProvideTextCommandDefault{\\textcommaaboveright}[1]{%%\n"
        "  \\LyxTextAccent[.5ex]{\\LyxAccentSize\\ '}{#1}}\n");

// Baltic languages use a comma-accent instead of a cedilla
static docstring const textbaltic_def = from_ascii(
        "%% use comma accent instead of cedilla for these characters:\n"
        "\\DeclareTextCompositeCommand{\\c}{T1}{g}{\\textcommaabove{g}}\n"
        "\\DeclareTextCompositeCommand{\\c}{T1}{G}{\\textcommabelow{G}}\n"
        "\\DeclareTextCompositeCommand{\\c}{T1}{k}{\\textcommabelow{k}}\n"
        "\\DeclareTextCompositeCommand{\\c}{T1}{K}{\\textcommabelow{K}}\n"
        "\\DeclareTextCompositeCommand{\\c}{T1}{l}{\\textcommabelow{l}}\n"
        "\\DeclareTextCompositeCommand{\\c}{T1}{L}{\\textcommabelow{L}}\n"
        "\\DeclareTextCompositeCommand{\\c}{T1}{n}{\\textcommabelow{n}}\n"
        "\\DeclareTextCompositeCommand{\\c}{T1}{N}{\\textcommabelow{N}}\n"
        "\\DeclareTextCompositeCommand{\\c}{T1}{r}{\\textcommabelow{r}}\n"
        "\\DeclareTextCompositeCommand{\\c}{T1}{R}{\\textcommabelow{R}}\n");

// split-level fractions
static docstring const xfrac_def = from_ascii(
	   "\\usepackage{xfrac}\n");
static docstring const smallLetterFrac_def = from_ascii(
        "\\DeclareCollectionInstance{smallLetterFrac}{xfrac}{default}{text}\n"
		"  {phantom=c, scale-factor=1.0, slash-left-kern=-.05em}\n"
		"\\DeclareCollectionInstance{smallLetterFrac}{xfrac}{lmr}{text}\n"
		"  {slash-symbol-font=ptm, phantom=c, scale-factor=1, slash-left-kern=-.05em}\n"
		"\\DeclareCollectionInstance{smallLetterFrac}{xfrac}{lmss}{text}\n"
		"  {slash-symbol-font=ptm, phantom=c, scale-factor=1, slash-left-kern=-.05em}\n"
		"\\DeclareCollectionInstance{smallLetterFrac}{xfrac}{cmr}{text}\n"
		"  {slash-symbol-font=ptm, phantom=c, scale-factor=1, slash-left-kern=-.05em}\n"
		"\\DeclareCollectionInstance{smallLetterFrac}{xfrac}{cmss}{text}\n"
		"  {slash-symbol-font=ptm, phantom=c, scale-factor=1, slash-left-kern=-.05em}\n"
		"\\newcommand{\\smallLetterFrac}[2]{%\n"
		"  {\\UseCollection{xfrac}{smallLetterFrac}\\sfrac{#1}{#2}}}\n");

static docstring const lyxref_def = from_ascii(
		"\\RS@ifundefined{subsecref}\n"
		"  {\\newref{subsec}{name = \\RSsectxt}}\n"
		"  {}\n"
		"\\RS@ifundefined{thmref}\n"
		"  {\\def\\RSthmtxt{theorem~}\\newref{thm}{name = \\RSthmtxt}}\n"
		"  {}\n"
		"\\RS@ifundefined{lemref}\n"
		"  {\\def\\RSlemtxt{lemma~}\\newref{lem}{name = \\RSlemtxt}}\n"
		"  {}\n");

// Make sure the columns are also outputed as rtl
static docstring const rtloutputdblcol_def = from_ascii(
	"\\def\\@outputdblcol{%\n"
	"  \\if@firstcolumn\n"
	"    \\global \\@firstcolumnfalse\n"
	"    \\global \\setbox\\@leftcolumn \\box\\@outputbox\n"
	"  \\else\n"
	"    \\global \\@firstcolumntrue\n"
	"    \\setbox\\@outputbox \\vbox {%\n"
	"      \\hb@xt@\\textwidth {%\n"
	"      \\kern\\textwidth \\kern-\\columnwidth %**\n"
	"      \\hb@xt@\\columnwidth {%\n"
	"         \\box\\@leftcolumn \\hss}%\n"
	"      \\kern-\\textwidth %**\n"
	"      \\hfil\n"
	"      {\\normalcolor\\vrule \\@width\\columnseprule}%\n"
	"      \\hfil\n"
	"      \\kern-\\textwidth  %**\n"
	"      \\hb@xt@\\columnwidth {%\n"
	"         \\box\\@outputbox \\hss}%\n"
	"      \\kern-\\columnwidth \\kern\\textwidth %**\n"
	"    }%\n"
	"  }%\n"
	"  \\@combinedblfloats\n"
	"  \\@outputpage\n"
	"  \\begingroup\n"
	"  \\@dblfloatplacement\n"
	"  \\@startdblcolumn\n"
	"  \\@whilesw\\if@fcolmade \\fi\n"
	"  {\\@outputpage\n"
	"    \\@startdblcolumn}%\n"
	"  \\endgroup\n"
	"  \\fi\n"
	"}\n"
	"\\@mparswitchtrue\n");

static docstring const lyxmintcaption_def = from_ascii(
	"\\long\\def\\lyxmintcaption[#1]#2{%\n"
	"  \\ifx#1t\\vskip\\baselineskip\\fi%\n"
	"  \\refstepcounter{listing}\\noindent%\n"
	"  \\addcontentsline{lol}{listing}%\n"
	"  {\\protect\\numberline{\\thelisting}{\\ignorespaces #2}}%\n"
	"  \\setbox\\@tempboxa\\hbox{\\listingscaption~\\thelisting: #2}%\n"
	"  \\ifdim \\wd\\@tempboxa >\\linewidth%\n"
	"  \\parbox[t]{\\linewidth}{\\unhbox\\@tempboxa}\\else%\n"
	"  \\hbox to\\linewidth{\\hfil\\box\\@tempboxa\\hfil}\\fi%\n"
	"  \\ifx#1b\\vskip\\baselineskip\\fi\n"
	"}\n");


/////////////////////////////////////////////////////////////////////
//
// LyXHTML strings
//
/////////////////////////////////////////////////////////////////////

static docstring const lyxnoun_style = from_ascii(
	"dfn.lyxnoun {\n"
	"  font-variant: small-caps;\n"
	"}\n");


// this is how it normally renders, but it might not always do so.
static docstring const lyxstrikeout_style = from_ascii(
	"del.strikeout {\n"
	"  text-decoration: line-through;\n"
	"}\n");


/////////////////////////////////////////////////////////////////////
//
// LaTeXFeatures
//
/////////////////////////////////////////////////////////////////////


LaTeXFeatures::LaTeXFeatures(Buffer const & b, BufferParams const & p,
			     OutputParams const & r)
	: buffer_(&b), params_(p), runparams_(r), in_float_(false),
	  in_deleted_inset_(false)
{}


LaTeXFeatures::LangPackage LaTeXFeatures::langPackage() const
{
	string const local_lp = bufferParams().lang_package;

	// Locally, custom is just stored as a string
	// in bufferParams().lang_package.
	if (local_lp != "auto"
	    && local_lp != "babel"
	    && local_lp != "default"
	    && local_lp != "none")
		 return LANG_PACK_CUSTOM;

	if (local_lp == "none")
		return LANG_PACK_NONE;

	/* If "auto" is selected, we load polyglossia with non-TeX fonts,
	 * else we select babel.
	 * If babel is selected (either directly or via the "auto"
	 * mechanism), we really do only require it if we have
	 * a language that needs it.
	 */
	bool const polyglossia_required =
		params_.useNonTeXFonts
		&& isAvailable("polyglossia")
		&& !isProvided("babel")
		&& this->hasOnlyPolyglossiaLanguages();
	bool const babel_required =
		!bufferParams().language->babel().empty()
		|| !this->getBabelLanguages().empty();

	if (local_lp == "auto") {
		// polyglossia requirement has priority over babel
		if (polyglossia_required)
			return LANG_PACK_POLYGLOSSIA;
		else if (babel_required)
			return LANG_PACK_BABEL;
	}

	if (local_lp == "babel") {
		if (babel_required)
			return LANG_PACK_BABEL;
	}

	if (local_lp == "default") {
		switch (lyxrc.language_package_selection) {
		case LyXRC::LP_AUTO:
			// polyglossia requirement has priority over babel
			if (polyglossia_required)
				return LANG_PACK_POLYGLOSSIA;
			else if (babel_required)
				return LANG_PACK_BABEL;
			break;
		case LyXRC::LP_BABEL:
			if (babel_required)
				return LANG_PACK_BABEL;
			break;
		case LyXRC::LP_CUSTOM:
			return LANG_PACK_CUSTOM;
		case LyXRC::LP_NONE:
			return LANG_PACK_NONE;
		}
	}

	return LANG_PACK_NONE;
}


void LaTeXFeatures::require(string const & name)
{
	features_.insert(name);
}


void LaTeXFeatures::require(set<string> const & names)
{
	features_.insert(names.begin(), names.end());
}


void LaTeXFeatures::useLayout(docstring const & layoutname)
{
	useLayout(layoutname, 0);
}


void LaTeXFeatures::useLayout(docstring const & layoutname, int level)
{
	// Some code to avoid loops in dependency definition
	const int maxlevel = 30;
	if (level > maxlevel) {
		lyxerr << "LaTeXFeatures::useLayout: maximum level of "
		       << "recursion attained by layout "
		       << to_utf8(layoutname) << endl;
		return;
	}

	DocumentClass const & tclass = params_.documentClass();
	if (tclass.hasLayout(layoutname)) {
		// Is this layout already in usedLayouts?
		if (find(usedLayouts_.begin(), usedLayouts_.end(), layoutname)
		    != usedLayouts_.end())
			return;

		Layout const & layout = tclass[layoutname];
		require(layout.requires());

		if (!layout.depends_on().empty()) {
			useLayout(layout.depends_on(), level + 1);
		}
		usedLayouts_.push_back(layoutname);
	} else {
		lyxerr << "LaTeXFeatures::useLayout: layout `"
		       << to_utf8(layoutname) << "' does not exist in this class"
		       << endl;
	}
}


void LaTeXFeatures::useInsetLayout(InsetLayout const & lay)
{
	docstring const & lname = lay.name();
	DocumentClass const & tclass = params_.documentClass();

	// this is a default inset layout, nothing useful here
	if (!tclass.hasInsetLayout(lname))
		return;
	// Is this layout already in usedInsetLayouts?
	if (find(usedInsetLayouts_.begin(), usedInsetLayouts_.end(), lname)
			!= usedInsetLayouts_.end())
		return;

	require(lay.requires());
	usedInsetLayouts_.push_back(lname);
}


bool LaTeXFeatures::isRequired(string const & name) const
{
	return features_.find(name) != features_.end();
}


bool LaTeXFeatures::isProvided(string const & name) const
{
	if (params_.useNonTeXFonts)
		return params_.documentClass().provides(name);

	bool const ot1 = (params_.main_font_encoding() == "default"
		|| params_.main_font_encoding() == "OT1");
	bool const complete = (params_.fontsSans() == "default"
		&& params_.fontsTypewriter() == "default");
	bool const nomath = (params_.fontsMath() == "default");
	return params_.documentClass().provides(name)
		|| theLaTeXFonts().getLaTeXFont(
			from_ascii(params_.fontsRoman())).provides(name, ot1,
								  complete,
								  nomath)
		|| theLaTeXFonts().getLaTeXFont(
			from_ascii(params_.fontsSans())).provides(name, ot1,
								 complete,
								 nomath)
		|| theLaTeXFonts().getLaTeXFont(
			from_ascii(params_.fontsTypewriter())).provides(name, ot1,
								       complete,
								       nomath)
		|| theLaTeXFonts().getLaTeXFont(
			from_ascii(params_.fontsMath())).provides(name, ot1,
								       complete,
								       nomath);
	// TODO: "textbaltic" provided, if the font-encoding is "L7x"
	//       "textgreek" provided, if a language with font-encoding LGR is used in the document
	//       "textcyr" provided, if a language with font-encoding T2A is used in the document
}


bool LaTeXFeatures::mustProvide(string const & name) const
{
	return isRequired(name) && !isProvided(name);
}


bool LaTeXFeatures::isAvailable(string const & name)
{
	string::size_type const i = name.find("->");
	if (i != string::npos) {
		string const from = name.substr(0,i);
		string const to = name.substr(i+2);
		//LYXERR0("from=[" << from << "] to=[" << to << "]");
		return theConverters().isReachable(from, to);
	}
	return LaTeXPackages::isAvailable(name);
}


namespace {

void addSnippet(std::list<TexString> & list, TexString ts, bool allow_dupes)
{
	if (allow_dupes ||
	    // test the absense of duplicates, i.e. elements with same str
	    none_of(list.begin(), list.end(), [&](TexString const & ts2){
			    return ts.str == ts2.str;
		    })
	    )
		list.push_back(move(ts));
}


TexString getSnippets(std::list<TexString> const & list)
{
	otexstringstream snip;
	for (TexString const & ts : list)
		snip << TexString(ts) << '\n';
	return snip.release();
}

} // namespace


void LaTeXFeatures::addPreambleSnippet(TexString ts, bool allow_dupes)
{
	addSnippet(preamble_snippets_, move(ts), allow_dupes);
}


void LaTeXFeatures::addPreambleSnippet(docstring const & str, bool allow_dupes)
{
	addSnippet(preamble_snippets_, TexString(str), allow_dupes);
}


void LaTeXFeatures::addCSSSnippet(std::string const & snippet)
{
	addSnippet(css_snippets_, TexString(from_ascii(snippet)), false);
}


TexString LaTeXFeatures::getPreambleSnippets() const
{
	return getSnippets(preamble_snippets_);
}


docstring LaTeXFeatures::getCSSSnippets() const
{
	return getSnippets(css_snippets_).str;
}



void LaTeXFeatures::useFloat(string const & name, bool subfloat)
{
	if (!usedFloats_[name])
		usedFloats_[name] = subfloat;
	if (subfloat)
		require("subfig");
	// We only need float.sty if we use non builtin floats, or if we
	// use the "H" modifier. This includes modified table and
	// figure floats. (Lgb)
	Floating const & fl = params_.documentClass().floats().getType(name);
	if (!fl.floattype().empty() && fl.usesFloatPkg()) {
		require("float");
	}
}


void LaTeXFeatures::useLanguage(Language const * lang)
{
	if (!lang->babel().empty() || !lang->polyglossia().empty())
		UsedLanguages_.insert(lang);
	if (!lang->requires().empty())
		require(lang->requires());
	// CJK languages do not have a babel name.
	// They use the CJK package
	if (lang->encoding()->package() == Encoding::CJK)
		require("CJK");
	// japanese package is special
	if (lang->encoding()->package() == Encoding::japanese)
		require("japanese");
}


void LaTeXFeatures::includeFile(docstring const & key, string const & name)
{
	IncludedFiles_[key] = name;
}


bool LaTeXFeatures::hasLanguages() const
{
	return !UsedLanguages_.empty();
}


bool LaTeXFeatures::hasOnlyPolyglossiaLanguages() const
{
	// first the main language
	if (params_.language->polyglossia().empty())
		return false;
	// now the secondary languages
	LanguageList::const_iterator const begin = UsedLanguages_.begin();
	for (LanguageList::const_iterator cit = begin;
	     cit != UsedLanguages_.end();
	     ++cit) {
		if ((*cit)->polyglossia().empty())
			return false;
	}
	return true;
}


bool LaTeXFeatures::hasPolyglossiaExclusiveLanguages() const
{
	// first the main language
	if (params_.language->isPolyglossiaExclusive())
		return true;
	// now the secondary languages
	LanguageList::const_iterator const begin = UsedLanguages_.begin();
	for (LanguageList::const_iterator cit = begin;
	     cit != UsedLanguages_.end();
	     ++cit) {
		if ((*cit)->isPolyglossiaExclusive())
			return true;
	}
	return false;
}


vector<string> LaTeXFeatures::getPolyglossiaExclusiveLanguages() const
{
	vector<string> result;
	// first the main language
	if (params_.language->isPolyglossiaExclusive())
		result.push_back(params_.language->display());
	// now the secondary languages
	LanguageList::const_iterator const begin = UsedLanguages_.begin();
	for (LanguageList::const_iterator cit = begin;
	     cit != UsedLanguages_.end();
	     ++cit) {
		if ((*cit)->isPolyglossiaExclusive())
			result.push_back((*cit)->display());
	}
	return result;
}


vector<string> LaTeXFeatures::getBabelExclusiveLanguages() const
{
	vector<string> result;
	// first the main language
	if (params_.language->isBabelExclusive())
		result.push_back(params_.language->display());
	// now the secondary languages
	LanguageList::const_iterator const begin = UsedLanguages_.begin();
	for (LanguageList::const_iterator cit = begin;
	     cit != UsedLanguages_.end();
	     ++cit) {
		if ((*cit)->isBabelExclusive())
			result.push_back((*cit)->display());
	}
	return result;
}


string LaTeXFeatures::getBabelLanguages() const
{
	ostringstream languages;

	bool first = true;
	LanguageList::const_iterator const begin = UsedLanguages_.begin();
	for (LanguageList::const_iterator cit = begin;
	     cit != UsedLanguages_.end();
	     ++cit) {
		if ((*cit)->babel().empty())
			continue;
		if (!first)
			languages << ',';
		else
			first = false;
		languages << (*cit)->babel();
	}
	return languages.str();
}


set<string> LaTeXFeatures::getPolyglossiaLanguages() const
{
	set<string> languages;

	LanguageList::const_iterator const begin = UsedLanguages_.begin();
	for (LanguageList::const_iterator cit = begin;
	     cit != UsedLanguages_.end();
	     ++cit) {
		// We do not need the variants here
		languages.insert((*cit)->polyglossia());
	}
	return languages;
}


set<string> LaTeXFeatures::getEncodingSet(string const & doc_encoding) const
{
	// This does only find encodings of languages supported by babel, but
	// that does not matter since we don't have a language with an
	// encoding supported by inputenc but without babel support.
	set<string> encodings;
	LanguageList::const_iterator it  = UsedLanguages_.begin();
	LanguageList::const_iterator end = UsedLanguages_.end();
	for (; it != end; ++it)
		if ((*it)->encoding()->latexName() != doc_encoding &&
		    ((*it)->encoding()->package() == Encoding::inputenc
		     || (*it)->encoding()->package() == Encoding::japanese))
			encodings.insert((*it)->encoding()->latexName());
	return encodings;
}


void LaTeXFeatures::getFontEncodings(vector<string> & encodings) const
{
	// these must be loaded if glyphs of this script are used
	// unless a language providing them is used in the document
	if (mustProvide("textgreek")
	    && find(encodings.begin(), encodings.end(), "LGR") == encodings.end())
		encodings.insert(encodings.begin(), "LGR");
	if (mustProvide("textcyr")
	    && find(encodings.begin(), encodings.end(), "T2A") == encodings.end())
		encodings.insert(encodings.begin(), "T2A");

	LanguageList::const_iterator it  = UsedLanguages_.begin();
	LanguageList::const_iterator end = UsedLanguages_.end();
	for (; it != end; ++it)
		if (!(*it)->fontenc().empty()
		    && ascii_lowercase((*it)->fontenc()) != "none") {
			vector<string> extraencs = getVectorFromString((*it)->fontenc());
			vector<string>::const_iterator fit = extraencs.begin();
			for (; fit != extraencs.end(); ++fit) {
				if (find(encodings.begin(), encodings.end(), *fit) == encodings.end())
					encodings.insert(encodings.begin(), *fit);
			}
		}
}

namespace {

char const * simplefeatures[] = {
// note that the package order here will be the same in the LaTeX-output
	"array",
	"verbatim",
	"longtable",
	"rotating",
	"latexsym",
	"pifont",
	// subfig is handled in BufferParams.cpp
	"varioref",
	"prettyref",
	"refstyle",
	/*For a successful cooperation of the `wrapfig' package with the
	  `float' package you should load the `wrapfig' package *after*
	  the `float' package. See the caption package documentation
	  for explanation.*/
	"float",
	"rotfloat",
	"wrapfig",
	"booktabs",
	"dvipost",
	"fancybox",
	"calc",
	"units",
	"framed",
	"soul",
	"textcomp",
	"pmboxdraw",
	"bbding",
	"ifsym",
	"txfonts",
	"pxfonts",
	"mathdesign",
	"mathrsfs",
	"mathabx",
	"mathtools",
	// "cancel",
	"ascii",
	"url",
	"covington",
	"csquotes",
	"enumitem",
	"endnotes",
	"hhline",
	"ifthen",
	// listings is handled in BufferParams.cpp
	"bm",
	"pdfpages",
	"amscd",
	"slashed",
	"multicol",
	"multirow",
	"tfrupee",
	"shapepar",
	"rsphrase",
	"hpstatement",
	"algorithm2e",
	"sectionbox",
	"tcolorbox",
	"pdfcomment",
	"fixme",
	"todonotes",
	"forest",
	"varwidth",
	"footnote",
	"tablefootnote",
};

char const * bibliofeatures[] = {
	// Known bibliography packages (will be loaded before natbib)
	"achicago",
	"apacite",
	"apalike",
	"astron",
	"authordate1-4",
	"babelbib",
	"bibgerm",
	"chapterbib",
	"chicago",
	"chscite",
	"harvard",
	"mslapa",
	"named"
};

int const nb_bibliofeatures = sizeof(bibliofeatures) / sizeof(char const *);

int const nb_simplefeatures = sizeof(simplefeatures) / sizeof(char const *);

} // namespace


string const LaTeXFeatures::getColorOptions() const
{
	ostringstream colors;

	// Handling the color packages separately is needed to be able to load them
	// before babel when hyperref is loaded with the colorlinks option
	// for more info see Bufferparams.cpp

	// [x]color.sty
	if (mustProvide("color") || mustProvide("xcolor")) {
		string const package =
			(mustProvide("xcolor") ? "xcolor" : "color");
		if (params_.graphics_driver == "default"
			|| params_.graphics_driver == "none")
			colors << "\\usepackage{" << package << "}\n";
		else
			colors << "\\usepackage["
				 << params_.graphics_driver
				 << "]{" << package << "}\n";
	}

	// pdfcolmk must be loaded after color
	if (mustProvide("pdfcolmk"))
		colors << "\\usepackage{pdfcolmk}\n";

	// the following 3 color commands must be set after color
	// is loaded and before pdfpages, therefore add the command
	// here define the set color
	if (mustProvide("pagecolor")) {
		colors << "\\definecolor{page_backgroundcolor}{rgb}{";
		colors << outputLaTeXColor(params_.backgroundcolor) << "}\n";
		// set the page color
		colors << "\\pagecolor{page_backgroundcolor}\n";
	}

	if (mustProvide("fontcolor")) {
		colors << "\\definecolor{document_fontcolor}{rgb}{";
		colors << outputLaTeXColor(params_.fontcolor) << "}\n";
		// set the color
		colors << "\\color{document_fontcolor}\n";
	}

	if (mustProvide("lyxgreyedout")) {
		colors << "\\definecolor{note_fontcolor}{rgb}{";
		colors << outputLaTeXColor(params_.notefontcolor) << "}\n";
		// the color will be set together with the definition of
		// the lyxgreyedout environment (see lyxgreyedout_def)
	}

	// color for shaded boxes
	if (isRequired("framed") && mustProvide("color")) {
		colors << "\\definecolor{shadecolor}{rgb}{";
		colors << outputLaTeXColor(params_.boxbgcolor) << "}\n";
		// this color is automatically used by the LaTeX-package "framed"
	}

	return colors.str();
}


string const LaTeXFeatures::getPackageOptions() const
{
	ostringstream packageopts;
	// Output all the package option stuff we have been asked to do.
	map<string, string>::const_iterator it =
		params_.documentClass().packageOptions().begin();
	map<string, string>::const_iterator en =
		params_.documentClass().packageOptions().end();
	for (; it != en; ++it)
		if (mustProvide(it->first))
			packageopts << "\\PassOptionsToPackage{" << it->second << "}"
				 << "{" << it->first << "}\n";
	return packageopts.str();
}


string const LaTeXFeatures::getPackages() const
{
	ostringstream packages;

	// FIXME: currently, we can only load packages and macros known
	// to LyX.
	// However, with the Require tag of layouts/custom insets,
	// also unknown packages can be requested. They are silently
	// swallowed now. We should change this eventually.

	//  These are all the 'simple' includes.  i.e
	//  packages which we just \usepackage{package}
	for (int i = 0; i < nb_simplefeatures; ++i) {
		if (mustProvide(simplefeatures[i]))
			packages << "\\usepackage{" << simplefeatures[i] << "}\n";
	}

	// The rest of these packages are somewhat more complicated
	// than those above.

	// The tipa package and its extensions (tipx, tone) must not
	// be loaded with non-TeX fonts, since fontspec includes the
	// respective macros
	if (mustProvide("tipa") && !params_.useNonTeXFonts)
		packages << "\\usepackage{tipa}\n";
	if (mustProvide("tipx") && !params_.useNonTeXFonts)
		packages << "\\usepackage{tipx}\n";
	if (mustProvide("extraipa") && !params_.useNonTeXFonts)
		packages << "\\usepackage{extraipa}\n";
	if (mustProvide("tone") && !params_.useNonTeXFonts)
		packages << "\\usepackage{tone}\n";

	// if fontspec or newtxmath is used, AMS packages have to be loaded
	// before fontspec (in BufferParams)
	string const amsPackages = loadAMSPackages();
	bool const ot1 = (params_.main_font_encoding() == "default"
			  || params_.main_font_encoding() == "OT1");
	bool const use_newtxmath =
		theLaTeXFonts().getLaTeXFont(from_ascii(params_.fontsMath())).getUsedPackage(
			ot1, false, false) == "newtxmath";

	if (!params_.useNonTeXFonts && !use_newtxmath && !amsPackages.empty())
		packages << amsPackages;

	if (mustProvide("cancel") &&
	    params_.use_package("cancel") != BufferParams::package_off)
		packages << "\\usepackage{cancel}\n";

   	// marvosym and bbding both define the \Cross macro
   	if (mustProvide("marvosym")) {
	    if (mustProvide("bbding"))
		packages << "\\let\\Cross\\relax\n";
   	    packages << "\\usepackage{marvosym}\n";
	}

	// accents must be loaded after amsmath
	if (mustProvide("accents") &&
	    params_.use_package("accents") != BufferParams::package_off)
		packages << "\\usepackage{accents}\n";

	// mathdots must be loaded after amsmath
	if (mustProvide("mathdots") &&
		params_.use_package("mathdots") != BufferParams::package_off)
		packages << "\\usepackage{mathdots}\n";

	// yhmath must be loaded after amsmath
	if (mustProvide("yhmath") &&
	    params_.use_package("yhmath") != BufferParams::package_off)
		packages << "\\usepackage{yhmath}\n";

	// stmaryrd must be loaded after amsmath
	if (mustProvide("stmaryrd") &&
	    params_.use_package("stmaryrd") != BufferParams::package_off)
		packages << "\\usepackage{stmaryrd}\n";

	if (mustProvide("stackrel") &&
	    params_.use_package("stackrel") != BufferParams::package_off)
		packages << "\\usepackage{stackrel}\n";

	if (mustProvide("undertilde") &&
		params_.use_package("undertilde") != BufferParams::package_off)
		packages << "\\usepackage{undertilde}\n";

	// [x]color and pdfcolmk are handled in getColorOptions() above

	// makeidx.sty
	if (isRequired("makeidx") || isRequired("splitidx")) {
		if (!isProvided("makeidx") && !isRequired("splitidx"))
			packages << "\\usepackage{makeidx}\n";
		if (mustProvide("splitidx"))
			packages << "\\usepackage{splitidx}\n";
		packages << "\\makeindex\n";
	}

	// graphicx.sty
	if (mustProvide("graphicx") && params_.graphics_driver != "none") {
		if (params_.graphics_driver == "default")
			packages << "\\usepackage{graphicx}\n";
		else
			packages << "\\usepackage["
				 << params_.graphics_driver
				 << "]{graphicx}\n";
	}

	// lyxskak.sty --- newer chess support based on skak.sty
	if (mustProvide("chess"))
		packages << "\\usepackage[ps,mover]{lyxskak}\n";

	// setspace.sty
	if (mustProvide("setspace") && !isProvided("SetSpace"))
		packages << "\\usepackage{setspace}\n";

	// we need to assure that mhchem is loaded before esint and every other
	// package that redefines command of amsmath because mhchem loads amlatex
	// (this info is from the author of mhchem from June 2013)
	if (mustProvide("mhchem") &&
	    params_.use_package("mhchem") != BufferParams::package_off)
		packages << "\\PassOptionsToPackage{version=3}{mhchem}\n"
			    "\\usepackage{mhchem}\n";

	// wasysym is a simple feature, but it must be after amsmath if both
	// are used
	// wasysym redefines some integrals (e.g. iint) from amsmath. That
	// leads to inconsistent integrals. We only load this package if
	// the document does not contain integrals (then isRequired("esint")
	// is false) or if esint is used, since esint redefines all relevant
	// integral symbols from wasysym and amsmath.
	// See http://www.lyx.org/trac/ticket/1942
	if (mustProvide("wasysym") &&
	    params_.use_package("wasysym") != BufferParams::package_off &&
	    (params_.use_package("esint") != BufferParams::package_off || !isRequired("esint")))
		packages << "\\usepackage{wasysym}\n";

	// esint must be after amsmath (and packages requiring amsmath, like mhchem)
	// and wasysym, since it will redeclare inconsistent integral symbols
	if (mustProvide("esint") &&
	    params_.use_package("esint") != BufferParams::package_off)
		packages << "\\usepackage{esint}\n";

	// Known bibliography packages (simple \usepackage{package})
	for (int i = 0; i < nb_bibliofeatures; ++i) {
		if (mustProvide(bibliofeatures[i]))
			packages << "\\usepackage{"
				 << bibliofeatures[i] << "}\n";
	}

	// Compatibility between achicago and natbib
	if (mustProvide("achicago") && mustProvide("natbib"))
		packages << "\\let\\achicagobib\\thebibliography\n";

	// natbib.sty
	// Some classes load natbib themselves, but still allow (or even require)
	// plain numeric citations (ReVTeX is such a case, see bug 5182).
	// This special case is indicated by the "natbib-internal" key.
	if (mustProvide("natbib") && !isProvided("natbib-internal")) {
		packages << "\\usepackage[";
		if (params_.citeEngineType() == ENGINE_TYPE_NUMERICAL)
			packages << "numbers";
		else
			packages << "authoryear";
		if (!params_.biblio_opts.empty())
			packages << ',' << params_.biblio_opts;
		packages << "]{natbib}\n";
	}

	// Compatibility between achicago and natbib
	if (mustProvide("achicago") && mustProvide("natbib")) {
		packages << "\\let\\thebibliography\\achicagobib\n";
		packages << "\\let\\SCcite\\astroncite\n";
		packages << "\\let\\UnexpandableProtect\\protect\n";
	}

	// jurabib -- we need version 0.6 at least.
	if (mustProvide("jurabib")) {
		packages << "\\usepackage";
		if (!params_.biblio_opts.empty())
			packages << '[' << params_.biblio_opts << ']';
		packages << "{jurabib}[2004/01/25]\n";
	}

	// opcit -- we pass custombst as we output \bibliographystyle ourselves
	if (mustProvide("opcit")) {
		if (isRequired("hyperref"))
			packages << "\\usepackage[custombst,hyperref]{opcit}\n";
		else
			packages << "\\usepackage[custombst]{opcit}\n";
	}

	// xargs -- we need version 1.09 at least
	if (mustProvide("xargs"))
		packages << "\\usepackage{xargs}[2008/03/08]\n";

	if (mustProvide("xy"))
		packages << "\\usepackage[all]{xy}\n";

	if (mustProvide("feyn"))
		packages << "\\usepackage{feyn}\n"; //Diagram

	if (mustProvide("ulem"))
		packages << "\\PassOptionsToPackage{normalem}{ulem}\n"
			    "\\usepackage{ulem}\n";

	if (mustProvide("nomencl")) {
		// Make it work with the new and old version of the package,
		// but don't use the compatibility option since it is
		// incompatible to other packages.
		packages << "\\usepackage{nomencl}\n"
			    "% the following is useful when we have the old nomencl.sty package\n"
			    "\\providecommand{\\printnomenclature}{\\printglossary}\n"
			    "\\providecommand{\\makenomenclature}{\\makeglossary}\n"
			    "\\makenomenclature\n";
	}

	// fixltx2e provides subscript
	if (mustProvide("subscript") && !isRequired("fixltx2e"))
		packages << "\\usepackage{subscript}\n";

	// footmisc must be loaded after setspace
	// Set options here, load the package after the user preamble to
	// avoid problems with manual loaded footmisc.
	if (mustProvide("footmisc"))
		packages << "\\PassOptionsToPackage{stable}{footmisc}\n";

	if (mustProvide("microtype")){
		packages << "\\usepackage{microtype}\n";
	}

	return packages.str();
}


TexString LaTeXFeatures::getMacros() const
{
	otexstringstream macros;

	if (!preamble_snippets_.empty()) {
		macros << '\n';
		macros << getPreambleSnippets();
	}

	if (mustProvide("papersize")) {
		if (runparams_.flavor == OutputParams::LATEX
		    || runparams_.flavor == OutputParams::DVILUATEX)
			macros << papersizedvi_def << '\n';
		else if  (runparams_.flavor == OutputParams::LUATEX)
			macros << papersizepdflua_def << '\n';
		else
			macros << papersizepdf_def << '\n';
	}

	if (mustProvide("LyX")) {
		if (isRequired("hyperref"))
			macros << lyx_hyperref_def << '\n';
		else
			macros << lyx_def << '\n';
	}

	if (mustProvide("noun"))
		macros << noun_def << '\n';

	if (mustProvide("lyxarrow"))
		macros << lyxarrow_def << '\n';

	if (mustProvide("lyxzerowidthspace"))
		macros << lyxZWSP_def << '\n';

	if (!usePolyglossia() && mustProvide("textgreek")) {
	    // ensure LGR font encoding is defined also if fontenc is not loaded by LyX
	   	if (params_.main_font_encoding() == "default")
			macros << textgreek_LGR_def;
		macros << textgreek_def << '\n';
	}

	if (!usePolyglossia() && mustProvide("textcyr")) {
		// ensure T2A font encoding is set up also if fontenc is not loaded by LyX
		if (params_.main_font_encoding() == "default")
			macros << textcyr_T2A_def;
		macros << textcyr_def << '\n';
	}

	// non-standard text accents:
	if (mustProvide("textcommaabove") || mustProvide("textcommaaboveright") ||
	    mustProvide("textcommabelow") || mustProvide("textbaltic"))
		macros << lyxaccent_def;

	if (mustProvide("textcommabelow") || mustProvide("textbaltic"))
		macros << textcommabelow_def << '\n';

	if (mustProvide("textcommaabove") || mustProvide("textbaltic"))
		macros << textcommaabove_def << '\n';

	if (mustProvide("textcommaaboveright"))
		macros << textcommaaboveright_def << '\n';

	if (mustProvide("textbaltic"))
		macros << textbaltic_def << '\n';

	// split-level fractions
	if (mustProvide("xfrac") || mustProvide("smallLetterFrac"))
		macros << xfrac_def << '\n';

	if (mustProvide("smallLetterFrac"))
		macros << smallLetterFrac_def << '\n';

	if (mustProvide("lyxmathsym"))
		macros << lyxmathsym_def << '\n';

	if (mustProvide("cedilla"))
		macros << cedilla_def << '\n';

	if (mustProvide("subring"))
		macros << subring_def << '\n';

	if (mustProvide("subdot"))
		macros << subdot_def << '\n';

	if (mustProvide("subhat"))
		macros << subhat_def << '\n';

	if (mustProvide("subtilde"))
		macros << subtilde_def << '\n';

	if (mustProvide("dacute"))
		macros << dacute_def << '\n';

	if (mustProvide("tipasymb"))
		macros << tipasymb_def << '\n';

	if (mustProvide("dgrave"))
		macros << dgrave_def << '\n';

	if (mustProvide("rcap"))
		macros << rcap_def << '\n';

	if (mustProvide("ogonek"))
		macros << ogonek_def << '\n';

	// quotes.
	if (mustProvide("quotesinglbase"))
		macros << quotesinglbase_def << '\n';
	if (mustProvide("quotedblbase"))
		macros << quotedblbase_def << '\n';
	if (mustProvide("guilsinglleft"))
		macros << guilsinglleft_def << '\n';
	if (mustProvide("guilsinglright"))
		macros << guilsinglright_def << '\n';
	if (mustProvide("guillemotleft"))
		macros << guillemotleft_def << '\n';
	if (mustProvide("guillemotright"))
		macros << guillemotright_def << '\n';
	if (mustProvide("textquotedbl"))
		macros << textquotedbl_def << '\n';
	if (mustProvide("textquotesinglep")) {
		if (runparams_.flavor == OutputParams::XETEX)
			macros << textquotesinglep_xetex_def << '\n';
		else
			macros << textquotesinglep_luatex_def << '\n';
	}
	if (mustProvide("textquotedblp")) {
		if (runparams_.flavor == OutputParams::XETEX)
			macros << textquotedblp_xetex_def << '\n';
		else
			macros << textquotedblp_luatex_def << '\n';
	}

	// Math mode
	if (mustProvide("binom") && !isRequired("amsmath"))
		macros << binom_def << '\n';
	if (mustProvide("mathcircumflex"))
		macros << mathcircumflex_def << '\n';

	// other
	if (mustProvide("ParagraphLeftIndent"))
		macros << paragraphleftindent_def;
	if (mustProvide("NeedLyXFootnoteCode"))
		macros << floatingfootnote_def;

	// some problems with tex->html converters
	if (mustProvide("NeedTabularnewline"))
		macros << tabularnewline_def;

	// greyed-out environment (note inset)
	// the color is specified in the routine
	// getColorOptions() to avoid LaTeX-package clashes
	if (mustProvide("lyxgreyedout"))
		macros << lyxgreyedout_def;

	if (mustProvide("lyxdot"))
		macros << lyxdot_def << '\n';

	// floats
	getFloatDefinitions(macros);

	if (mustProvide("refstyle"))
		macros << lyxref_def << '\n';

	// change tracking
	if (mustProvide("ct-dvipost"))
		macros << changetracking_dvipost_def;

	if (mustProvide("ct-xcolor-ulem")) {
		streamsize const prec = macros.os().precision(2);

		RGBColor cadd = rgbFromHexName(lcolor.getX11Name(Color_addedtext));
		macros << "\\providecolor{lyxadded}{rgb}{"
		       << cadd.r / 255.0 << ',' << cadd.g / 255.0 << ',' << cadd.b / 255.0 << "}\n";

		RGBColor cdel = rgbFromHexName(lcolor.getX11Name(Color_deletedtext));
		macros << "\\providecolor{lyxdeleted}{rgb}{"
		       << cdel.r / 255.0 << ',' << cdel.g / 255.0 << ',' << cdel.b / 255.0 << "}\n";

		macros.os().precision(prec);

		if (isRequired("hyperref"))
			macros << changetracking_xcolor_ulem_hyperref_def;
		else
			macros << changetracking_xcolor_ulem_def;
	}

	if (mustProvide("ct-tikz-math-sout"))
			macros << changetracking_tikz_math_sout_def;

	if (mustProvide("ct-none"))
		macros << changetracking_none_def;

	if (mustProvide("rtloutputdblcol"))
		macros << rtloutputdblcol_def;

	if (mustProvide("lyxmintcaption"))
		macros << lyxmintcaption_def;

	return macros.release();
}


docstring const LaTeXFeatures::getBabelPresettings() const
{
	odocstringstream tmp;

	for (Language const * lang : UsedLanguages_)
		if (!lang->babel_presettings().empty())
			tmp << lang->babel_presettings() << '\n';
	if (!params_.language->babel_presettings().empty())
		tmp << params_.language->babel_presettings() << '\n';

	if (!contains(tmp.str(), '@'))
		return tmp.str();

	return "\\makeatletter\n" + tmp.str() + "\\makeatother\n";
}


docstring const LaTeXFeatures::getBabelPostsettings() const
{
	odocstringstream tmp;

	for (Language const * lang : UsedLanguages_)
		if (!lang->babel_postsettings().empty())
			tmp << lang->babel_postsettings() << '\n';
	if (!params_.language->babel_postsettings().empty())
		tmp << params_.language->babel_postsettings() << '\n';

	if (!contains(tmp.str(), '@'))
		return tmp.str();

	return "\\makeatletter\n" + tmp.str() + "\\makeatother\n";
}


bool LaTeXFeatures::needBabelLangOptions() const
{
	if (!lyxrc.language_global_options || params_.language->asBabelOptions())
		return true;

	LanguageList::const_iterator it  = UsedLanguages_.begin();
	LanguageList::const_iterator end = UsedLanguages_.end();
	for (; it != end; ++it)
		if ((*it)->asBabelOptions())
			return true;

	return false;
}


string const LaTeXFeatures::loadAMSPackages() const
{
	ostringstream tmp;

	if (mustProvide("amsmath")
	    && params_.use_package("amsmath") != BufferParams::package_off) {
		tmp << "\\usepackage{amsmath}\n";
	} else {
		// amsbsy and amstext are already provided by amsmath
		if (mustProvide("amsbsy"))
			tmp << "\\usepackage{amsbsy}\n";
		if (mustProvide("amstext"))
			tmp << "\\usepackage{amstext}\n";
	}

	if (mustProvide("amsthm"))
		tmp << "\\usepackage{amsthm}\n";

	if (mustProvide("amssymb")
	    && params_.use_package("amssymb") != BufferParams::package_off)
		tmp << "\\usepackage{amssymb}\n";

	return tmp.str();
}


docstring const LaTeXFeatures::getTClassPreamble() const
{
	// the text class specific preamble
	DocumentClass const & tclass = params_.documentClass();
	odocstringstream tcpreamble;

	tcpreamble << tclass.preamble();

	list<docstring>::const_iterator cit = usedLayouts_.begin();
	list<docstring>::const_iterator end = usedLayouts_.end();
	for (; cit != end; ++cit)
		// For InPreamble layouts, we output the preamble stuff earlier
		// (before the layouts). See Paragraph::Private::validate.
		if (!tclass[*cit].inpreamble)
			tcpreamble << tclass[*cit].preamble();

	cit = usedInsetLayouts_.begin();
	end = usedInsetLayouts_.end();
	TextClass::InsetLayouts const & ils = tclass.insetLayouts();
	for (; cit != end; ++cit) {
		TextClass::InsetLayouts::const_iterator it = ils.find(*cit);
		if (it == ils.end())
			continue;
		tcpreamble << it->second.preamble();
	}

	return tcpreamble.str();
}


docstring const LaTeXFeatures::getTClassHTMLPreamble() const
{
	DocumentClass const & tclass = params_.documentClass();
	odocstringstream tcpreamble;

	tcpreamble << tclass.htmlpreamble();

	list<docstring>::const_iterator cit = usedLayouts_.begin();
	list<docstring>::const_iterator end = usedLayouts_.end();
	for (; cit != end; ++cit)
		tcpreamble << tclass[*cit].htmlpreamble();

	cit = usedInsetLayouts_.begin();
	end = usedInsetLayouts_.end();
	TextClass::InsetLayouts const & ils = tclass.insetLayouts();
	for (; cit != end; ++cit) {
		TextClass::InsetLayouts::const_iterator it = ils.find(*cit);
		if (it == ils.end())
			continue;
		tcpreamble << it->second.htmlpreamble();
	}

	return tcpreamble.str();
}


docstring const LaTeXFeatures::getTClassHTMLStyles() const
{
	DocumentClass const & tclass = params_.documentClass();
	odocstringstream tcpreamble;

	if (mustProvide("noun"))
		tcpreamble << lyxnoun_style;
	// this isn't exact, but it won't hurt that much if it
	// wasn't for this.
	if (mustProvide("ulem"))
		tcpreamble << lyxstrikeout_style;

	tcpreamble << tclass.htmlstyles();

	list<docstring>::const_iterator cit = usedLayouts_.begin();
	list<docstring>::const_iterator end = usedLayouts_.end();
	for (; cit != end; ++cit)
		tcpreamble << tclass[*cit].htmlstyle();

	cit = usedInsetLayouts_.begin();
	end = usedInsetLayouts_.end();
	TextClass::InsetLayouts const & ils = tclass.insetLayouts();
	for (; cit != end; ++cit) {
		TextClass::InsetLayouts::const_iterator it = ils.find(*cit);
		if (it == ils.end())
			continue;
		tcpreamble << it->second.htmlstyle();
	}

	return tcpreamble.str();
}


namespace {

docstring const getFloatI18nPreamble(docstring const & type,
			docstring const & name, Language const * lang,
			Encoding const & enc, bool const polyglossia)
{
	// Check whether name can be encoded in the buffer encoding
	bool encodable = true;
	for (size_t i = 0; i < name.size(); ++i) {
		if (!enc.encodable(name[i])) {
			encodable = false;
			break;
		}
	}

	docstring const language = polyglossia ? from_ascii(lang->polyglossia())
					       : from_ascii(lang->babel());
	docstring const langenc = from_ascii(lang->encoding()->iconvName());
	docstring const texenc = from_ascii(lang->encoding()->latexName());
	docstring const bufenc = from_ascii(enc.iconvName());
	docstring const s1 = docstring(1, 0xF0000);
	docstring const s2 = docstring(1, 0xF0001);
	docstring const translated = encodable ? name
		: from_ascii("\\inputencoding{") + texenc + from_ascii("}")
			+ s1 + langenc + s2 + name + s1 + bufenc + s2;

	odocstringstream os;
	os << "\\addto\\captions" << language
	   << "{\\renewcommand{\\" << type << "name}{" << translated << "}}\n";
	return os.str();
}


docstring const i18npreamble(docstring const & templ, Language const * lang,
                             Encoding const & enc, bool const polyglossia,
                             bool const need_fixedwidth)
{
	if (templ.empty())
		return templ;

	string preamble = polyglossia ?
		subst(to_utf8(templ), "$$lang", lang->polyglossia()) :
		subst(to_utf8(templ), "$$lang", lang->babel());

	string const langenc = lang->encoding()->iconvName();
	string const texenc = lang->encoding()->latexName();
	string const bufenc = enc.iconvName();
	Encoding const * testenc(&enc);
	bool lang_fallback = false;
	bool ascii_fallback = false;
	if (need_fixedwidth && !enc.hasFixedWidth()) {
		if (lang->encoding()->hasFixedWidth()) {
			testenc = lang->encoding();
			lang_fallback = true;
		} else {
			// We need a fixed width encoding, but both the buffer
			// encoding and the language encoding are variable
			// width. As a last fallback, try to convert to pure
			// ASCII using the LaTeX commands defined in unicodesymbols.
			testenc = encodings.fromLyXName("ascii");
			if (!testenc)
				return docstring();
			ascii_fallback = true;
		}
	}
	// First and second character of plane 15 (Private Use Area)
	string const s1 = "\xf3\xb0\x80\x80"; // U+F0000
	string const s2 = "\xf3\xb0\x80\x81"; // U+F0001
	// FIXME UNICODE
	// lyx::regex is not unicode-safe.
	// Should use QRegExp or (boost::u32regex, but that requires ICU)
	static regex const reg("_\\(([^\\)]+)\\)");
	smatch sub;
	while (regex_search(preamble, sub, reg)) {
		string const key = sub.str(1);
		docstring const name = lang->translateLayout(key);
		// Check whether name can be encoded in the buffer encoding
		bool encodable = true;
		for (size_t i = 0; i < name.size() && encodable; ++i)
			if (!testenc->encodable(name[i]))
				encodable = false;
		string translated;
		if (encodable && !lang_fallback)
			translated = to_utf8(name);
		else if (ascii_fallback)
			translated = to_ascii(testenc->latexString(name).first);
		else
			translated = "\\inputencoding{" + texenc + "}"
				+ s1 + langenc + s2 + to_utf8(name)
				+ s1 + bufenc + s2;
		preamble = subst(preamble, sub.str(), translated);
	}
	return from_utf8(preamble);
}

} // namespace


docstring const LaTeXFeatures::getTClassI18nPreamble(bool use_babel,
				bool use_polyglossia, bool use_minted) const
{
	DocumentClass const & tclass = params_.documentClass();
	// collect preamble snippets in a set to prevent multiple identical
	// commands (would happen if e.g. both theorem and theorem* are used)
	set<docstring> snippets;
	typedef LanguageList::const_iterator lang_it;
	lang_it const lbeg = UsedLanguages_.begin();
	lang_it const lend =  UsedLanguages_.end();
	list<docstring>::const_iterator cit = usedLayouts_.begin();
	list<docstring>::const_iterator end = usedLayouts_.end();
	for (; cit != end; ++cit) {
		// language dependent commands (once per document)
		snippets.insert(i18npreamble(tclass[*cit].langpreamble(),
						buffer().language(),
						buffer().params().encoding(),
						use_polyglossia, false));
		// commands for language changing (for multilanguage documents)
		if ((use_babel || use_polyglossia) && !UsedLanguages_.empty()) {
			snippets.insert(i18npreamble(
						tclass[*cit].babelpreamble(),
						buffer().language(),
						buffer().params().encoding(),
						use_polyglossia, false));
			for (lang_it lit = lbeg; lit != lend; ++lit)
				snippets.insert(i18npreamble(
						tclass[*cit].babelpreamble(),
						*lit,
						buffer().params().encoding(),
						use_polyglossia, false));
		}
	}
	if ((use_babel || use_polyglossia) && !UsedLanguages_.empty()) {
		FloatList const & floats = params_.documentClass().floats();
		UsedFloats::const_iterator fit = usedFloats_.begin();
		UsedFloats::const_iterator fend = usedFloats_.end();
		for (; fit != fend; ++fit) {
			Floating const & fl = floats.getType(fit->first);
			// we assume builtin floats are translated
			if (fl.isPredefined())
				continue;
			docstring const type = from_ascii(fl.floattype());
			docstring const flname = from_utf8(fl.name());
			docstring name = buffer().language()->translateLayout(fl.name());
			// only request translation if we have a real translation
			// (that differs from the source)
			if (flname != name)
				snippets.insert(getFloatI18nPreamble(
						type, name, buffer().language(),
						buffer().params().encoding(),
						use_polyglossia));
			for (lang_it lit = lbeg; lit != lend; ++lit) {
				string const code = (*lit)->code();
				name = (*lit)->translateLayout(fl.name());
				// we assume we have a suitable translation if
				// either the language is English (we need to
				// translate into English if English is a secondary
				// language) or if translateIfPossible returns
				// something different to the English source.
				bool const have_translation =
					(flname != name || contains(code, "en"));
				if (have_translation)
					snippets.insert(getFloatI18nPreamble(
						type, name, *lit,
						buffer().params().encoding(),
						use_polyglossia));
			}
		}
	}

	cit = usedInsetLayouts_.begin();
	end = usedInsetLayouts_.end();
	TextClass::InsetLayouts const & ils = tclass.insetLayouts();
	for (; cit != end; ++cit) {
		TextClass::InsetLayouts::const_iterator it = ils.find(*cit);
		if (it == ils.end())
			continue;
		// The listings package does not work with variable width
		// encodings, only with fixed width encodings. Therefore we
		// need to force a fixed width encoding for
		// \lstlistlistingname and \lstlistingname (bug 9382).
		// This needs to be consistent with InsetListings::latex().
		bool const need_fixedwidth = !use_minted &&
					!runparams_.isFullUnicode() &&
					it->second.fixedwidthpreambleencoding();
		// language dependent commands (once per document)
		snippets.insert(i18npreamble(it->second.langpreamble(),
						buffer().language(),
						buffer().params().encoding(),
						use_polyglossia, need_fixedwidth));
		// commands for language changing (for multilanguage documents)
		if ((use_babel || use_polyglossia) && !UsedLanguages_.empty()) {
			snippets.insert(i18npreamble(
						it->second.babelpreamble(),
						buffer().language(),
						buffer().params().encoding(),
						use_polyglossia, need_fixedwidth));
			for (lang_it lit = lbeg; lit != lend; ++lit)
				snippets.insert(i18npreamble(
						it->second.babelpreamble(),
						*lit,
						buffer().params().encoding(),
						use_polyglossia, need_fixedwidth));
		}
	}

	odocstringstream tcpreamble;
	set<docstring>::const_iterator const send = snippets.end();
	set<docstring>::const_iterator it = snippets.begin();
	for (; it != send; ++it)
		tcpreamble << *it;
	return tcpreamble.str();
}


docstring const LaTeXFeatures::getLyXSGMLEntities() const
{
	// Definition of entities used in the document that are LyX related.
	odocstringstream entities;

	if (mustProvide("lyxarrow")) {
		entities << "<!ENTITY lyxarrow \"-&gt;\">" << '\n';
	}

	return entities.str();
}


docstring const LaTeXFeatures::getIncludedFiles(string const & fname) const
{
	odocstringstream sgmlpreamble;
	// FIXME UNICODE
	docstring const basename(from_utf8(onlyPath(fname)));

	FileMap::const_iterator end = IncludedFiles_.end();
	for (FileMap::const_iterator fi = IncludedFiles_.begin();
	     fi != end; ++fi)
		// FIXME UNICODE
		sgmlpreamble << "\n<!ENTITY " << fi->first
			     << (isSGMLFileName(fi->second) ? " SYSTEM \"" : " \"")
			     << makeRelPath(from_utf8(fi->second), basename) << "\">";

	return sgmlpreamble.str();
}


void LaTeXFeatures::showStruct() const
{
	lyxerr << "LyX needs the following commands when LaTeXing:"
	       << "\n***** Packages:" << getPackages()
	       << "\n***** Macros:" << to_utf8(getMacros().str)
	       << "\n***** Textclass stuff:" << to_utf8(getTClassPreamble())
	       << "\n***** done." << endl;
}


Buffer const & LaTeXFeatures::buffer() const
{
	return *buffer_;
}


void LaTeXFeatures::setBuffer(Buffer const & buffer)
{
	buffer_ = &buffer;
}


BufferParams const & LaTeXFeatures::bufferParams() const
{
	return params_;
}


void LaTeXFeatures::getFloatDefinitions(otexstream & os) const
{
	FloatList const & floats = params_.documentClass().floats();

	// Here we will output the code to create the needed float styles.
	// We will try to do this as minimal as possible.
	// \floatstyle{ruled}
	// \newfloat{algorithm}{htbp}{loa}
	// \providecommand{\algorithmname}{Algorithm}
	// \floatname{algorithm}{\protect\algorithmname}
	UsedFloats::const_iterator cit = usedFloats_.begin();
	UsedFloats::const_iterator end = usedFloats_.end();
	for (; cit != end; ++cit) {
		Floating const & fl = floats.getType(cit->first);

		// For builtin floats we do nothing.
		if (fl.isPredefined())
			continue;

		// We have to special case "table" and "figure"
		if (fl.floattype() == "tabular" || fl.floattype() == "figure") {
			// Output code to modify "table" or "figure"
			// but only if builtin == false
			// and that have to be true at this point in the
			// function.
			docstring const type = from_ascii(fl.floattype());
			docstring const placement = from_ascii(fl.placement());
			docstring const style = from_ascii(fl.style());
			if (!style.empty()) {
				os << "\\floatstyle{" << style << "}\n"
				   << "\\restylefloat{" << type << "}\n";
			}
			if (!placement.empty()) {
				os << "\\floatplacement{" << type << "}{"
				   << placement << "}\n";
			}
		} else {
			// The other non builtin floats.

			docstring const type = from_ascii(fl.floattype());
			docstring const placement = from_ascii(fl.placement());
			docstring const ext = from_ascii(fl.ext());
			docstring const within = from_ascii(fl.within());
			docstring const style = from_ascii(fl.style());
			docstring const name =
				buffer().language()->translateLayout(fl.name());
			os << "\\floatstyle{" << style << "}\n"
			   << "\\newfloat{" << type << "}{" << placement
			   << "}{" << ext << '}';
			if (!within.empty())
				os << '[' << within << ']';
			os << '\n'
			   << "\\providecommand{\\" << type << "name}{"
			   << name << "}\n"
			   << "\\floatname{" << type << "}{\\protect\\"
			   << type << "name}\n";

			// What missing here is to code to minimalize the code
			// output so that the same floatstyle will not be
			// used several times, when the same style is still in
			// effect. (Lgb)
		}
		if (cit->second)
			// The subfig package is loaded later
			os << "\n\\AtBeginDocument{\\newsubfloat{" << from_ascii(fl.floattype()) << "}}\n";
	}
}


void LaTeXFeatures::resolveAlternatives()
{
	for (Features::iterator it = features_.begin(); it != features_.end();) {
		if (contains(*it, '|')) {
			vector<string> const alternatives = getVectorFromString(*it, "|");
			vector<string>::const_iterator const end = alternatives.end();
			vector<string>::const_iterator ita = alternatives.begin();
			// Is any alternative already required? => use that
			for (; ita != end; ++ita) {
				if (isRequired(*ita))
					break;
			}
			// Is any alternative available? => use the first one
			// (bug 9498)
			if (ita == end) {
				for (ita = alternatives.begin(); ita != end; ++ita) {
					if (isAvailable(*ita)) {
						require(*ita);
						break;
					}
				}
			}
			// This will not work, but not requiring something
			// would be more confusing
			if (ita == end)
				require(alternatives.front());
			features_.erase(it);
			it = features_.begin();
		} else
			++it;
	}
}


void LaTeXFeatures::expandMultiples()
{
	for (Features::iterator it = features_.begin(); it != features_.end();) {
		if (contains(*it, ',')) {
			vector<string> const multiples = getVectorFromString(*it, ",");
			vector<string>::const_iterator const end = multiples.end();
			vector<string>::const_iterator itm = multiples.begin();
			// Do nothing if any multiple is already required
			for (; itm != end; ++itm) {
				if (!isRequired(*itm))
					require(*itm);
			}
			features_.erase(it);
			it = features_.begin();
		} else
			++it;
	}
}


} // namespace lyx
