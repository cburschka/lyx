/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include "bufferparams.h"
#include "tex-strings.h"
#include "encoding.h"
#include "layout.h"
#include "vspace.h"
#include "debug.h"
#include "lyxrc.h"
#include "language.h"
#include "lyxtextclasslist.h"
#include "lyxlex.h"
#include "Lsstream.h"

#include "support/lyxalgo.h" // for lyx::count
#include "support/lyxlib.h"
#include "support/lstrings.h"

#include <cstdlib>

using std::ostream;
using std::endl;

#ifdef WITH_WARNINGS
#warning Do we need this horrible thing? (JMarc)
#endif
bool use_babel;

BufferParams::BufferParams()
	// Initialize textclass to point to article. if `first' is
	// true in the returned pair, then `second' is the textclass
	// number; if it is false, second is 0. In both cases, second
	// is what we want.
	: textclass(textclasslist.NumberOfClass("article").second)
{
	paragraph_separation = PARSEP_INDENT;
	defskip = VSpace(VSpace::MEDSKIP);
	quotes_language = InsetQuotes::EnglishQ;
	quotes_times = InsetQuotes::DoubleQ;
	fontsize = "default";

	/*  PaperLayout */
	papersize = PAPER_DEFAULT;
	papersize2 = VM_PAPER_DEFAULT; /* DEFAULT */
	paperpackage = PACKAGE_NONE;
	orientation = ORIENTATION_PORTRAIT;
	use_geometry = false;
	use_amsmath = AMS_AUTO;
	use_natbib = false;
	use_numerical_citations = false;
	tracking_changes = false;
	secnumdepth = 3;
	tocdepth = 3;
	language = default_language;
	fonts = "default";
	inputenc = "auto";
	graphicsDriver = "default";
	sides = LyXTextClass::OneSide;
	columns = 1;
	pagestyle = "default";
	for (int iter = 0; iter < 4; ++iter) {
		user_defined_bullets[iter] = ITEMIZE_DEFAULTS[iter];
		temp_bullets[iter] = ITEMIZE_DEFAULTS[iter];
	}
}


void BufferParams::writeFile(ostream & os) const
{
	// The top of the file is written by the buffer.
	// Prints out the buffer info into the .lyx file given by file

	// the textclass
	os << "\\textclass " << textclasslist[textclass].name() << '\n';

	// then the the preamble
	if (!preamble.empty()) {
		// remove '\n' from the end of preamble
		string const tmppreamble = rtrim(preamble, "\n");
		os << "\\begin_preamble\n"
		   << tmppreamble
		   << "\n\\end_preamble\n";
	}

	/* the options */
	if (!options.empty()) {
		os << "\\options " << options << '\n';
	}

	/* then the text parameters */
	if (language != ignore_language)
		os << "\\language " << language->lang() << '\n';
	os << "\\inputencoding " << inputenc
	   << "\n\\fontscheme " << fonts
	   << "\n\\graphics " << graphicsDriver << '\n';

	if (!float_placement.empty()) {
		os << "\\float_placement " << float_placement << '\n';
	}
	os << "\\paperfontsize " << fontsize << '\n';

	spacing.writeFile(os);

	os << "\\papersize " << string_papersize[papersize2]
	   << "\n\\paperpackage " << string_paperpackages[paperpackage]
	   << "\n\\use_geometry " << use_geometry
	   << "\n\\use_amsmath " << use_amsmath
	   << "\n\\use_natbib " << use_natbib
	   << "\n\\use_numerical_citations " << use_numerical_citations
	   << "\n\\paperorientation " << string_orientation[orientation]
	   << '\n';
	if (!paperwidth.empty())
		os << "\\paperwidth "
		   << VSpace(paperwidth).asLyXCommand() << '\n';
	if (!paperheight.empty())
		os << "\\paperheight "
		   << VSpace(paperheight).asLyXCommand() << '\n';
	if (!leftmargin.empty())
		os << "\\leftmargin "
		   << VSpace(leftmargin).asLyXCommand() << '\n';
	if (!topmargin.empty())
		os << "\\topmargin "
		   << VSpace(topmargin).asLyXCommand() << '\n';
	if (!rightmargin.empty())
		os << "\\rightmargin "
		   << VSpace(rightmargin).asLyXCommand() << '\n';
	if (!bottommargin.empty())
		os << "\\bottommargin "
		   << VSpace(bottommargin).asLyXCommand() << '\n';
	if (!headheight.empty())
		os << "\\headheight "
		   << VSpace(headheight).asLyXCommand() << '\n';
	if (!headsep.empty())
		os << "\\headsep "
		   << VSpace(headsep).asLyXCommand() << '\n';
	if (!footskip.empty())
		os << "\\footskip "
		   << VSpace(footskip).asLyXCommand() << '\n';
	os << "\\secnumdepth " << secnumdepth
	   << "\n\\tocdepth " << tocdepth
	   << "\n\\paragraph_separation "
	   << string_paragraph_separation[paragraph_separation]
	   << "\n\\defskip " << defskip.asLyXCommand()
	   << "\n\\quotes_language "
	   << string_quotes_language[quotes_language] << '\n';
	switch (quotes_times) {
		// An output operator for insetquotes would be nice
	case InsetQuotes::SingleQ:
		os << "\\quotes_times 1\n"; break;
	case InsetQuotes::DoubleQ:
		os << "\\quotes_times 2\n"; break;
	}
	os << "\\papercolumns " << columns
	   << "\n\\papersides " << sides
	   << "\n\\paperpagestyle " << pagestyle << '\n';
	for (int i = 0; i < 4; ++i) {
		if (user_defined_bullets[i] != ITEMIZE_DEFAULTS[i]) {
			if (user_defined_bullets[i].getFont() != -1) {
				os << "\\bullet " << i
				   << "\n\t"
				   << user_defined_bullets[i].getFont()
				   << "\n\t"
				   << user_defined_bullets[i].getCharacter()
				   << "\n\t"
				   << user_defined_bullets[i].getSize()
				   << "\n\\end_bullet\n";
			}
			else {
				os << "\\bulletLaTeX " << i
				   << "\n\t\""
				   << user_defined_bullets[i].getText()
				   << "\"\n\\end_bullet\n";
			}
		}
	}

	os << "\\tracking_changes " << tracking_changes << "\n";
}


void BufferParams::writeLaTeX(ostream & os, LaTeXFeatures & features,
			      TexRow & texrow) const
{
	os << "\\documentclass";

	LyXTextClass const & tclass = getLyXTextClass();

	ostringstream clsoptions; // the document class options.

	if (tokenPos(tclass.opt_fontsize(),
		     '|', fontsize) >= 0) {
		// only write if existing in list (and not default)
		clsoptions << fontsize << "pt,";
	}


	if (!use_geometry &&
	    (paperpackage == PACKAGE_NONE)) {
		switch (papersize) {
		case PAPER_A4PAPER:
			clsoptions << "a4paper,";
			break;
		case PAPER_USLETTER:
			clsoptions << "letterpaper,";
			break;
		case PAPER_A5PAPER:
			clsoptions << "a5paper,";
			break;
		case PAPER_B5PAPER:
			clsoptions << "b5paper,";
			break;
		case PAPER_EXECUTIVEPAPER:
			clsoptions << "executivepaper,";
			break;
		case PAPER_LEGALPAPER:
			clsoptions << "legalpaper,";
			break;
		}
	}

	// if needed
	if (sides != tclass.sides()) {
		switch (sides) {
		case LyXTextClass::OneSide:
			clsoptions << "oneside,";
			break;
		case LyXTextClass::TwoSides:
			clsoptions << "twoside,";
			break;
		}
	}

	// if needed
	if (columns != tclass.columns()) {
		if (columns == 2)
			clsoptions << "twocolumn,";
		else
			clsoptions << "onecolumn,";
	}

	if (!use_geometry
	    && orientation == ORIENTATION_LANDSCAPE)
		clsoptions << "landscape,";

	// language should be a parameter to \documentclass
	use_babel = false;
	ostringstream language_options;
	if (language->babel() == "hebrew"
	    && default_language->babel() != "hebrew")
		// This seems necessary
		features.useLanguage(default_language);

	if (lyxrc.language_use_babel ||
	    language->lang() != lyxrc.default_language ||
	    features.hasLanguages()) {
		use_babel = true;
		language_options << features.getLanguages();
		language_options << language->babel();
		if (lyxrc.language_global_options)
			clsoptions << language_options.str() << ',';
	}

	// the user-defined options
	if (!options.empty()) {
		clsoptions << options << ',';
	}

	string strOptions(STRCONV(clsoptions.str()));
	if (!strOptions.empty()) {
		strOptions = rtrim(strOptions, ",");
		os << '[' << strOptions << ']';
	}

	os << '{' << tclass.latexname() << "}\n";
	texrow.newline();
	// end of \documentclass defs

	// font selection must be done before loading fontenc.sty
	// The ae package is not needed when using OT1 font encoding.
	if (fonts != "default" &&
	    (fonts != "ae" || lyxrc.fontenc != "default")) {
		os << "\\usepackage{" << fonts << "}\n";
		texrow.newline();
		if (fonts == "ae") {
			os << "\\usepackage{aecompl}\n";
			texrow.newline();
		}
	}
	// this one is not per buffer
	if (lyxrc.fontenc != "default") {
		os << "\\usepackage[" << lyxrc.fontenc
		   << "]{fontenc}\n";
		texrow.newline();
	}

	if (inputenc == "auto") {
		string const doc_encoding =
			language->encoding()->LatexName();

		// Create a list with all the input encodings used
		// in the document
		std::set<string> encodings =
			features.getEncodingSet(doc_encoding);

		os << "\\usepackage[";
		std::copy(encodings.begin(), encodings.end(),
			  std::ostream_iterator<string>(os, ","));
		os << doc_encoding << "]{inputenc}\n";
		texrow.newline();
	} else if (inputenc != "default") {
		os << "\\usepackage[" << inputenc
		   << "]{inputenc}\n";
		texrow.newline();
	}

	// At the very beginning the text parameters.
	if (paperpackage != PACKAGE_NONE) {
		switch (paperpackage) {
		case PACKAGE_A4:
			os << "\\usepackage{a4}\n";
			texrow.newline();
			break;
		case PACKAGE_A4WIDE:
			os << "\\usepackage{a4wide}\n";
			texrow.newline();
			break;
		case PACKAGE_WIDEMARGINSA4:
			os << "\\usepackage[widemargins]{a4}\n";
			texrow.newline();
			break;
		}
	}
	if (use_geometry) {
		os << "\\usepackage{geometry}\n";
		texrow.newline();
		os << "\\geometry{verbose";
		if (orientation == ORIENTATION_LANDSCAPE)
			os << ",landscape";
		switch (papersize2) {
		case VM_PAPER_CUSTOM:
			if (!paperwidth.empty())
				os << ",paperwidth="
				   << paperwidth;
			if (!paperheight.empty())
				os << ",paperheight="
				   << paperheight;
			break;
		case VM_PAPER_USLETTER:
			os << ",letterpaper";
			break;
		case VM_PAPER_USLEGAL:
			os << ",legalpaper";
			break;
		case VM_PAPER_USEXECUTIVE:
			os << ",executivepaper";
			break;
		case VM_PAPER_A3:
			os << ",a3paper";
			break;
		case VM_PAPER_A4:
			os << ",a4paper";
			break;
		case VM_PAPER_A5:
			os << ",a5paper";
			break;
		case VM_PAPER_B3:
			os << ",b3paper";
			break;
		case VM_PAPER_B4:
			os << ",b4paper";
			break;
		case VM_PAPER_B5:
			os << ",b5paper";
			break;
		default:
				// default papersize ie VM_PAPER_DEFAULT
			switch (lyxrc.default_papersize) {
			case PAPER_DEFAULT: // keep compiler happy
			case PAPER_USLETTER:
				os << ",letterpaper";
				break;
			case PAPER_LEGALPAPER:
				os << ",legalpaper";
				break;
			case PAPER_EXECUTIVEPAPER:
				os << ",executivepaper";
				break;
			case PAPER_A3PAPER:
				os << ",a3paper";
				break;
			case PAPER_A4PAPER:
				os << ",a4paper";
				break;
			case PAPER_A5PAPER:
				os << ",a5paper";
				break;
			case PAPER_B5PAPER:
				os << ",b5paper";
				break;
			}
		}
		if (!topmargin.empty())
			os << ",tmargin=" << topmargin;
		if (!bottommargin.empty())
			os << ",bmargin=" << bottommargin;
		if (!leftmargin.empty())
			os << ",lmargin=" << leftmargin;
		if (!rightmargin.empty())
			os << ",rmargin=" << rightmargin;
		if (!headheight.empty())
			os << ",headheight=" << headheight;
		if (!headsep.empty())
			os << ",headsep=" << headsep;
		if (!footskip.empty())
			os << ",footskip=" << footskip;
		os << "}\n";
		texrow.newline();
	}

	if (tokenPos(tclass.opt_pagestyle(),
		     '|', pagestyle) >= 0) {
		if (pagestyle == "fancy") {
			os << "\\usepackage{fancyhdr}\n";
			texrow.newline();
		}
		os << "\\pagestyle{" << pagestyle << "}\n";
		texrow.newline();
	}

	if (secnumdepth != tclass.secnumdepth()) {
		os << "\\setcounter{secnumdepth}{"
		   << secnumdepth
		   << "}\n";
		texrow.newline();
	}
	if (tocdepth != tclass.tocdepth()) {
		os << "\\setcounter{tocdepth}{"
		   << tocdepth
		   << "}\n";
		texrow.newline();
	}

	if (paragraph_separation) {
		switch (defskip.kind()) {
		case VSpace::SMALLSKIP:
			os << "\\setlength\\parskip{\\smallskipamount}\n";
			break;
		case VSpace::MEDSKIP:
			os << "\\setlength\\parskip{\\medskipamount}\n";
			break;
		case VSpace::BIGSKIP:
			os << "\\setlength\\parskip{\\bigskipamount}\n";
			break;
		case VSpace::LENGTH:
			os << "\\setlength\\parskip{"
			   << defskip.length().asLatexString()
			   << "}\n";
			break;
		default: // should never happen // Then delete it.
			os << "\\setlength\\parskip{\\medskipamount}\n";
			break;
		}
		texrow.newline();

		os << "\\setlength\\parindent{0pt}\n";
		texrow.newline();
	}

	// Now insert the LyX specific LaTeX commands...

	// The optional packages;
	string lyxpreamble(features.getPackages());

	// this might be useful...
	lyxpreamble += "\n\\makeatletter\n";

	// Some macros LyX will need
	string tmppreamble(features.getMacros());

	if (!tmppreamble.empty()) {
		lyxpreamble += "\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% "
			"LyX specific LaTeX commands.\n"
			+ tmppreamble + '\n';
	}

	// the text class specific preamble
	tmppreamble = features.getTClassPreamble();
	if (!tmppreamble.empty()) {
		lyxpreamble += "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% "
			"Textclass specific LaTeX commands.\n"
			+ tmppreamble + '\n';
	}

	/* the user-defined preamble */
	if (!preamble.empty()) {
		lyxpreamble += "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% "
			"User specified LaTeX commands.\n"
			+ preamble + '\n';
	}

	// Itemize bullet settings need to be last in case the user
	// defines their own bullets that use a package included
	// in the user-defined preamble -- ARRae
	// Actually it has to be done much later than that
	// since some packages like frenchb make modifications
	// at \begin{document} time -- JMarc
	string bullets_def;
	for (int i = 0; i < 4; ++i) {
		if (user_defined_bullets[i] != ITEMIZE_DEFAULTS[i]) {
			if (bullets_def.empty())
				bullets_def="\\AtBeginDocument{\n";
			bullets_def += "  \\renewcommand{\\labelitemi";
			switch (i) {
				// `i' is one less than the item to modify
			case 0:
				break;
			case 1:
				bullets_def += 'i';
				break;
			case 2:
				bullets_def += "ii";
				break;
			case 3:
				bullets_def += 'v';
				break;
			}
			bullets_def += "}{" +
				user_defined_bullets[i].getText()
				+ "}\n";
		}
	}

	if (!bullets_def.empty())
		lyxpreamble += bullets_def + "}\n\n";

	// We try to load babel late, in case it interferes
	// with other packages.
	if (use_babel) {
		string tmp = lyxrc.language_package;
		if (!lyxrc.language_global_options
		    && tmp == "\\usepackage{babel}")
			tmp = string("\\usepackage[") +
				STRCONV(language_options.str()) +
				"]{babel}";
		lyxpreamble += tmp + "\n";
		lyxpreamble += features.getBabelOptions();
	}

	lyxpreamble += "\\makeatother\n";

	// dvipost settings come after everything else
	if (tracking_changes) {
		lyxpreamble +=
			"\\dvipostlayout\n"
			"\\dvipost{osstart color push Red}\n"
			"\\dvipost{osend color pop}\n"
			"\\dvipost{cbstart color push Blue}\n"
			"\\dvipost{cbend color pop} \n";
	}

	int const nlines =
		int(lyx::count(lyxpreamble.begin(), lyxpreamble.end(), '\n'));
	for (int j = 0; j != nlines; ++j) {
		texrow.newline();
	}

	os << lyxpreamble;
}

void BufferParams::setPaperStuff()
{
	papersize = PAPER_DEFAULT;
	char const c1 = paperpackage;
	if (c1 == PACKAGE_NONE) {
		char const c2 = papersize2;
		if (c2 == VM_PAPER_USLETTER)
			papersize = PAPER_USLETTER;
		else if (c2 == VM_PAPER_USLEGAL)
			papersize = PAPER_LEGALPAPER;
		else if (c2 == VM_PAPER_USEXECUTIVE)
			papersize = PAPER_EXECUTIVEPAPER;
		else if (c2 == VM_PAPER_A3)
			papersize = PAPER_A3PAPER;
		else if (c2 == VM_PAPER_A4)
			papersize = PAPER_A4PAPER;
		else if (c2 == VM_PAPER_A5)
			papersize = PAPER_A5PAPER;
		else if ((c2 == VM_PAPER_B3) || (c2 == VM_PAPER_B4) ||
			 (c2 == VM_PAPER_B5))
			papersize = PAPER_B5PAPER;
	} else if ((c1 == PACKAGE_A4) || (c1 == PACKAGE_A4WIDE) ||
		   (c1 == PACKAGE_WIDEMARGINSA4))
		papersize = PAPER_A4PAPER;
}


void BufferParams::useClassDefaults()
{
	LyXTextClass const & tclass = textclasslist[textclass];

	sides = tclass.sides();
	columns = tclass.columns();
	pagestyle = tclass.pagestyle();
	options = tclass.options();
	secnumdepth = tclass.secnumdepth();
	tocdepth = tclass.tocdepth();
}


bool BufferParams::hasClassDefaults() const
{
	LyXTextClass const & tclass = textclasslist[textclass];

	return (sides == tclass.sides()
		&& columns == tclass.columns()
		&& pagestyle == tclass.pagestyle()
		&& options == tclass.options()
		&& secnumdepth == tclass.secnumdepth()
		&& tocdepth == tclass.tocdepth());
}


LyXTextClass const & BufferParams::getLyXTextClass() const
{
	return textclasslist[textclass];
}


void BufferParams::readPreamble(LyXLex & lex)
{
	if (lex.getString() != "\\begin_preamble")
		lyxerr << "Error (BufferParams::readPreamble):"
			"consistency check failed." << endl;

	preamble = lex.getLongString("\\end_preamble");
}


void BufferParams::readLanguage(LyXLex & lex)
{
	if (!lex.next()) return;

	string const tmptok = lex.getString();

	// check if tmptok is part of tex_babel in tex-defs.h
	language = languages.getLanguage(tmptok);
	if (!language) {
		// Language tmptok was not found
		language = default_language;
		lyxerr << "Warning: Setting language `"
		       << tmptok << "' to `" << language->lang()
		       << "'." << endl;
	}
}


void BufferParams::readGraphicsDriver(LyXLex & lex)
{
	if (!lex.next()) return;

	string const tmptok = lex.getString();
	// check if tmptok is part of tex_graphics in tex_defs.h
	int n = 0;
	while (true) {
		string const test = tex_graphics[n++];

		if (test == tmptok) {
			graphicsDriver = tmptok;
			break;
		} else if (test == "last_item") {
			lex.printError(
				"Warning: graphics driver `$$Token' not recognized!\n"
				"         Setting graphics driver to `default'.\n");
			graphicsDriver = "default";
			break;
		}
	}
}
