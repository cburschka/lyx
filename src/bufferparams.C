/**
 * \file bufferparams.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author André Pönitz
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "bufferparams.h"

#include "author.h"
#include "BranchList.h"
#include "Bullet.h"
#include "debug.h"
#include "encoding.h"
#include "gettext.h"
#include "language.h"
#include "LaTeXFeatures.h"
#include "latexrunparams.h"
#include "LColor.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "lyxtextclasslist.h"
#include "tex-strings.h"
#include "Spacing.h"
#include "texrow.h"
#include "vspace.h"

#include "frontends/Alert.h"

#include "support/lyxalgo.h" // for lyx::count

#include <boost/array.hpp>

#include "support/std_sstream.h"

namespace support = lyx::support;
using lyx::support::bformat;
using lyx::support::rtrim;
using lyx::support::tokenPos;

using std::endl;

using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::pair;


struct BufferParams::Impl
{
	Impl();

	AuthorList authorlist;
	BranchList branchlist;
	boost::array<Bullet, 4> temp_bullets;
	boost::array<Bullet, 4> user_defined_bullets;
	Spacing spacing;
	/** This is the amount of space used for paragraph_separation "skip",
	 * and for detached paragraphs in "indented" documents.
	 */
	VSpace defskip;
};


BufferParams::Impl::Impl()
	: defskip(VSpace::MEDSKIP)
{
	// set initial author
	authorlist.record(Author(lyxrc.user_name, lyxrc.user_email));
}


BufferParams::Impl *
BufferParams::MemoryTraits::clone(BufferParams::Impl const * ptr)
{
	return new BufferParams::Impl(*ptr);
}


void BufferParams::MemoryTraits::destroy(BufferParams::Impl * ptr)
{
	delete ptr;
}


BufferParams::BufferParams()
	: // Initialize textclass to point to article. if `first' is
	  // true in the returned pair, then `second' is the textclass
	  // number; if it is false, second is 0. In both cases, second
	  // is what we want.
	textclass(textclasslist.NumberOfClass("article").second),
	pimpl_(new Impl)
{
	paragraph_separation = PARSEP_INDENT;
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
	compressed = false;
	for (int iter = 0; iter < 4; ++iter) {
		user_defined_bullet(iter) = ITEMIZE_DEFAULTS[iter];
		temp_bullet(iter) = ITEMIZE_DEFAULTS[iter];
	}
}


BufferParams::~BufferParams()
{}


AuthorList & BufferParams::authors()
{
	return pimpl_->authorlist;
}


AuthorList const & BufferParams::authors() const
{
	return pimpl_->authorlist;
}


BranchList & BufferParams::branchlist()
{
	return pimpl_->branchlist;
}


BranchList const & BufferParams::branchlist() const
{
	return pimpl_->branchlist;
}


Bullet & BufferParams::temp_bullet(lyx::size_type index)
{
	BOOST_ASSERT(index < 4);
	return pimpl_->temp_bullets[index];
}


Bullet const & BufferParams::temp_bullet(lyx::size_type index) const
{
	BOOST_ASSERT(index < 4);
	return pimpl_->temp_bullets[index];
}


Bullet & BufferParams::user_defined_bullet(lyx::size_type index)
{
	BOOST_ASSERT(index < 4);
	return pimpl_->user_defined_bullets[index];
}


Bullet const & BufferParams::user_defined_bullet(lyx::size_type index) const
{
	BOOST_ASSERT(index < 4);
	return pimpl_->user_defined_bullets[index];
}


Spacing & BufferParams::spacing()
{
	return pimpl_->spacing;
}


Spacing const & BufferParams::spacing() const
{
	return pimpl_->spacing;
}


VSpace const & BufferParams::getDefSkip() const
{
	return pimpl_->defskip;
}


void BufferParams::setDefSkip(VSpace const & vs)
{
	pimpl_->defskip = vs;
}


string const BufferParams::readToken(LyXLex & lex, string const & token)
{
	if (token == "\\textclass") {
		lex.eatLine();
		string const classname = lex.getString();
		pair<bool, lyx::textclass_type> pp =
			textclasslist.NumberOfClass(classname);
		if (pp.first) {
			textclass = pp.second;
		} else {
			textclass = 0;
			return classname;
		}
		if (!getLyXTextClass().isTeXClassAvailable()) {
			string msg = bformat(_("The document uses a missing "
				"TeX class \"%1$s\".\n"), classname);
			Alert::warning(_("Document class not available"),
			               msg + _("LyX will not be able to produce output."));
		}
	} else if (token == "\\begin_preamble") {
		readPreamble(lex);
	} else if (token == "\\options") {
		lex.eatLine();
		options = lex.getString();
	} else if (token == "\\language") {
		readLanguage(lex);
	} else if (token == "\\inputencoding") {
		lex.eatLine();
		inputenc = lex.getString();
	} else if (token == "\\graphics") {
		readGraphicsDriver(lex);
	} else if (token == "\\fontscheme") {
		lex.eatLine();
		fonts = lex.getString();
	} else if (token == "\\paragraph_separation") {
		int tmpret = lex.findToken(string_paragraph_separation);
		if (tmpret == -1)
			++tmpret;
		paragraph_separation =
			static_cast<PARSEP>(tmpret);
	} else if (token == "\\defskip") {
		lex.nextToken();
		pimpl_->defskip = VSpace(lex.getString());
	} else if (token == "\\quotes_language") {
		// FIXME: should be params.readQuotes()
		int tmpret = lex.findToken(string_quotes_language);
		if (tmpret == -1)
			++tmpret;
		InsetQuotes::quote_language tmpl =
			InsetQuotes::EnglishQ;
		switch (tmpret) {
		case 0:
			tmpl = InsetQuotes::EnglishQ;
			break;
		case 1:
			tmpl = InsetQuotes::SwedishQ;
			break;
		case 2:
			tmpl = InsetQuotes::GermanQ;
			break;
		case 3:
			tmpl = InsetQuotes::PolishQ;
			break;
		case 4:
			tmpl = InsetQuotes::FrenchQ;
			break;
		case 5:
			tmpl = InsetQuotes::DanishQ;
			break;
		}
		quotes_language = tmpl;
	} else if (token == "\\quotes_times") {
		// FIXME: should be params.readQuotes()
		lex.nextToken();
		switch (lex.getInteger()) {
		case 1:
			quotes_times = InsetQuotes::SingleQ;
			break;
		case 2:
			quotes_times = InsetQuotes::DoubleQ;
			break;
		}
	} else if (token == "\\papersize") {
		int tmpret = lex.findToken(string_papersize);
		if (tmpret == -1)
			++tmpret;
		else
			papersize2 = VMARGIN_PAPER_TYPE(tmpret);
	} else if (token == "\\paperpackage") {
		int tmpret = lex.findToken(string_paperpackages);
		if (tmpret == -1) {
			++tmpret;
			paperpackage = PACKAGE_NONE;
		} else
			paperpackage = PAPER_PACKAGES(tmpret);
	} else if (token == "\\use_geometry") {
		lex.nextToken();
		use_geometry = lex.getInteger();
	} else if (token == "\\use_amsmath") {
		lex.nextToken();
		use_amsmath = static_cast<AMS>(
			lex.getInteger());
	} else if (token == "\\use_natbib") {
		lex.nextToken();
		use_natbib = lex.getInteger();
	} else if (token == "\\use_numerical_citations") {
		lex.nextToken();
		use_numerical_citations = lex.getInteger();
	} else if (token == "\\tracking_changes") {
		lex.nextToken();
		tracking_changes = lex.getInteger();
	} else if (token == "\\branch") {
		lex.nextToken();
		string branch = lex.getString();
		branchlist().add(branch);
		while (true) {
			lex.nextToken();
			string const tok = lex.getString();
			if (tok == "\\end_branch")
				break;
			if (tok == "\\selected") {
				lex.nextToken();
				branchlist().setSelected(branch, lex.getInteger());
			}
			// not yet operational
			if (tok == "\\color") {
				lex.nextToken();
				string color = lex.getString();
				branchlist().setColor(branch, color);
				// Update also the LColor table:
				if (color == "none")
					color = lcolor.getX11Name(LColor::background);
				lcolor.fill(static_cast<LColor::color>(lcolor.size()),
						branch, color);
			}
		}
	} else if (token == "\\author") {
		lex.nextToken();
		istringstream ss(lex.getString());
		Author a;
		ss >> a;
		author_map.push_back(pimpl_->authorlist.record(a));
	} else if (token == "\\paperorientation") {
		int tmpret = lex.findToken(string_orientation);
		if (tmpret == -1)
			++tmpret;
		orientation =
			static_cast<PAPER_ORIENTATION>(tmpret);
	} else if (token == "\\paperwidth") {
		lex.next();
		paperwidth = lex.getString();
	} else if (token == "\\paperheight") {
		lex.next();
		paperheight = lex.getString();
	} else if (token == "\\leftmargin") {
		lex.next();
		leftmargin = lex.getString();
	} else if (token == "\\topmargin") {
		lex.next();
		topmargin = lex.getString();
	} else if (token == "\\rightmargin") {
		lex.next();
		rightmargin = lex.getString();
	} else if (token == "\\bottommargin") {
		lex.next();
		bottommargin = lex.getString();
	} else if (token == "\\headheight") {
		lex.next();
		headheight = lex.getString();
	} else if (token == "\\headsep") {
		lex.next();
		headsep = lex.getString();
	} else if (token == "\\footskip") {
		lex.next();
		footskip = lex.getString();
	} else if (token == "\\paperfontsize") {
		lex.nextToken();
		fontsize = rtrim(lex.getString());
	} else if (token == "\\papercolumns") {
		lex.nextToken();
		columns = lex.getInteger();
	} else if (token == "\\papersides") {
		lex.nextToken();
		switch (lex.getInteger()) {
		default:
		case 1: sides = LyXTextClass::OneSide; break;
		case 2: sides = LyXTextClass::TwoSides; break;
		}
	} else if (token == "\\paperpagestyle") {
		lex.nextToken();
		pagestyle = rtrim(lex.getString());
	} else if (token == "\\bullet") {
		// FIXME: should be params.readBullets()
		lex.nextToken();
		int const index = lex.getInteger();
		lex.nextToken();
		int temp_int = lex.getInteger();
		user_defined_bullet(index).setFont(temp_int);
		temp_bullet(index).setFont(temp_int);
		lex.nextToken();
		temp_int = lex.getInteger();
		user_defined_bullet(index).setCharacter(temp_int);
		temp_bullet(index).setCharacter(temp_int);
		lex.nextToken();
		temp_int = lex.getInteger();
		user_defined_bullet(index).setSize(temp_int);
		temp_bullet(index).setSize(temp_int);
		lex.nextToken();
		string const temp_str = lex.getString();
		if (temp_str != "\\end_bullet") {
				// this element isn't really necessary for
				// parsing but is easier for humans
				// to understand bullets. Put it back and
				// set a debug message?
			lex.printError("\\end_bullet expected, got" + temp_str);
				//how can I put it back?
		}
	} else if (token == "\\bulletLaTeX") {
		// The bullet class should be able to read this.
		lex.nextToken();
		int const index = lex.getInteger();
		lex.next();
		string temp_str = lex.getString();
		string sum_str;
		while (temp_str != "\\end_bullet") {
				// this loop structure is needed when user
				// enters an empty string since the first
				// thing returned will be the \\end_bullet
				// OR
				// if the LaTeX entry has spaces. Each element
				// therefore needs to be read in turn
			sum_str += temp_str;
			lex.next();
			temp_str = lex.getString();
		}

		user_defined_bullet(index).setText(sum_str);
		temp_bullet(index).setText(sum_str);
	} else if (token == "\\secnumdepth") {
		lex.nextToken();
		secnumdepth = lex.getInteger();
	} else if (token == "\\tocdepth") {
		lex.nextToken();
		tocdepth = lex.getInteger();
	} else if (token == "\\spacing") {
		lex.next();
		string const tmp = rtrim(lex.getString());
		Spacing::Space tmp_space = Spacing::Default;
		float tmp_val = 0.0;
		if (tmp == "single") {
			tmp_space = Spacing::Single;
		} else if (tmp == "onehalf") {
			tmp_space = Spacing::Onehalf;
		} else if (tmp == "double") {
			tmp_space = Spacing::Double;
		} else if (tmp == "other") {
			lex.next();
			tmp_space = Spacing::Other;
			tmp_val = lex.getFloat();
		} else {
			lex.printError("Unknown spacing token: '$$Token'");
		}
#if 0 // FIXME: Handled in lyx2lyx ?
		// Small hack so that files written with klyx will be
		// parsed correctly.
		if (first_par)
			par->params().spacing(Spacing(tmp_space, tmp_val));
#endif
		spacing().set(tmp_space, tmp_val);
	} else if (token == "\\float_placement") {
		lex.nextToken();
		float_placement = lex.getString();
	} else {
		return token;
	}

	return string();
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

	spacing().writeFile(os);

	os << "\\papersize " << string_papersize[papersize2]
	   << "\n\\paperpackage " << string_paperpackages[paperpackage]
	   << "\n\\use_geometry " << use_geometry
	   << "\n\\use_amsmath " << use_amsmath
	   << "\n\\use_natbib " << use_natbib
	   << "\n\\use_numerical_citations " << use_numerical_citations
	   << "\n\\paperorientation " << string_orientation[orientation]
	   << '\n';

	std::list<Branch>::const_iterator it = branchlist().begin();
	std::list<Branch>::const_iterator end = branchlist().end();
	for (; it != end; ++it) {
		os << "\\branch " << it->getBranch()
		   << "\n\\selected " << it->getSelected()
		   << "\n\\color " << it->getColor()
		   << "\n\\end_branch"
		   << "\n";
	}

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
	   << "\n\\defskip " << getDefSkip().asLyXCommand()
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
		if (user_defined_bullet(i) != ITEMIZE_DEFAULTS[i]) {
			if (user_defined_bullet(i).getFont() != -1) {
				os << "\\bullet " << i
				   << "\n\t"
				   << user_defined_bullet(i).getFont()
				   << "\n\t"
				   << user_defined_bullet(i).getCharacter()
				   << "\n\t"
				   << user_defined_bullet(i).getSize()
				   << "\n\\end_bullet\n";
			}
			else {
				os << "\\bulletLaTeX " << i
				   << "\n\t\""
				   << user_defined_bullet(i).getText()
				   << "\"\n\\end_bullet\n";
			}
		}
	}

	os << "\\tracking_changes " << tracking_changes << "\n";

	if (tracking_changes) {
		AuthorList::Authors::const_iterator it = pimpl_->authorlist.begin();
		AuthorList::Authors::const_iterator end = pimpl_->authorlist.end();
		for (; it != end; ++it) {
			os << "\\author " << it->second << "\n";
		}
	}
}


bool BufferParams::writeLaTeX(ostream & os, LaTeXFeatures & features,
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
		case PAPER_A3PAPER:
			clsoptions << "a3paper,";
			break;
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
		case PAPER_DEFAULT:
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
	if (language->babel() == "hebrew"
	    && default_language->babel() != "hebrew")
		// This seems necessary
		features.useLanguage(default_language);

	ostringstream language_options;
	bool const use_babel = features.useBabel();
	if (use_babel) {
		language_options << features.getLanguages();
		language_options << language->babel();
		if (lyxrc.language_global_options)
			clsoptions << language_options.str() << ',';
	}

	// the user-defined options
	if (!options.empty()) {
		clsoptions << options << ',';
	}

	string strOptions(clsoptions.str());
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
		case PACKAGE_NONE:
			break;
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
		switch (getDefSkip().kind()) {
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
			   << getDefSkip().length().asLatexString()
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
		if (user_defined_bullet(i) != ITEMIZE_DEFAULTS[i]) {
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
				user_defined_bullet(i).getText()
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
				language_options.str() +
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
	return use_babel;
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


string const BufferParams::paperSizeName() const
{
	char real_papersize = papersize;
	if (real_papersize == PAPER_DEFAULT)
		real_papersize = lyxrc.default_papersize;

	switch (real_papersize) {
	case PAPER_A3PAPER:
		return "a3";
	case PAPER_A4PAPER:
		return "a4";
	case PAPER_A5PAPER:
		return "a5";
	case PAPER_B5PAPER:
		return "b5";
	case PAPER_EXECUTIVEPAPER:
		return "foolscap";
	case PAPER_LEGALPAPER:
		return "legal";
	case PAPER_USLETTER:
	default:
		return "letter";
	}
}


string const BufferParams::dvips_options() const
{
	string result;

	if (use_geometry
	    && papersize2 == VM_PAPER_CUSTOM
	    && !lyxrc.print_paper_dimension_flag.empty()
	    && !paperwidth.empty()
	    && !paperheight.empty()) {
		// using a custom papersize
		result = lyxrc.print_paper_dimension_flag;
		result += ' ' + paperwidth;
		result += ',' + paperheight;
	} else {
		string const paper_option = paperSizeName();
		if (paper_option != "letter" ||
		    orientation != ORIENTATION_LANDSCAPE) {
			// dvips won't accept -t letter -t landscape.
			// In all other cases, include the paper size
			// explicitly.
			result = lyxrc.print_paper_flag;
			result += ' ' + paper_option;
		}
	}
	if (orientation == ORIENTATION_LANDSCAPE &&
	    papersize2 != VM_PAPER_CUSTOM)
		result += ' ' + lyxrc.print_landscape_flag;
	return result;
}
