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
#include "LColor.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "lyxtextclasslist.h"
#include "outputparams.h"
#include "tex-strings.h"
#include "Spacing.h"
#include "texrow.h"
#include "vspace.h"

#include "frontends/Alert.h"

#include "support/lyxalgo.h" // for lyx::count
#include "support/tostr.h"
#include "support/translator.h"

#include <boost/array.hpp>

#include <sstream>

namespace support = lyx::support;
using lyx::support::bformat;
using lyx::support::rtrim;
using lyx::support::tokenPos;

using std::endl;
using std::string;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::pair;

namespace biblio = lyx::biblio;


// Local translators
namespace {

// Paragraph separation
typedef Translator<string, BufferParams::PARSEP> ParSepTranslator;


ParSepTranslator const init_parseptranslator()
{
	ParSepTranslator translator(string_paragraph_separation[0], BufferParams::PARSEP_INDENT);
	translator.addPair(string_paragraph_separation[1], BufferParams::PARSEP_SKIP);
	return translator;
}


ParSepTranslator const & parseptranslator()
{
	static ParSepTranslator translator = init_parseptranslator();
	return translator;
}


// Quotes language
typedef Translator<string, InsetQuotes::quote_language> QuotesLangTranslator;


QuotesLangTranslator const init_quoteslangtranslator()
{
	QuotesLangTranslator translator(string_quotes_language[0], InsetQuotes::EnglishQ);
	translator.addPair(string_quotes_language[1], InsetQuotes::SwedishQ);
	translator.addPair(string_quotes_language[2], InsetQuotes::GermanQ);
	translator.addPair(string_quotes_language[3], InsetQuotes::PolishQ);
	translator.addPair(string_quotes_language[4], InsetQuotes::FrenchQ);
	translator.addPair(string_quotes_language[5], InsetQuotes::DanishQ);
	return translator;
}


QuotesLangTranslator const & quoteslangtranslator()
{
	static QuotesLangTranslator translator = init_quoteslangtranslator();
	return translator;
}


// Quote times
typedef Translator<int, InsetQuotes::quote_times> QuotesTimesTranslator;


QuotesTimesTranslator const init_quotestimestranslator()
{
	QuotesTimesTranslator translator(1, InsetQuotes::SingleQ);
	translator.addPair(2, InsetQuotes::DoubleQ);
	return translator;
}


QuotesTimesTranslator const & quotestimestranslator()
{
	static QuotesTimesTranslator translator = init_quotestimestranslator();
	return translator;
}


// Paper size
typedef Translator<std::string, VMARGIN_PAPER_TYPE> PaperSizeTranslator;


PaperSizeTranslator const init_papersizetranslator()
{
	PaperSizeTranslator translator(string_papersize[0], VM_PAPER_DEFAULT);
	translator.addPair(string_papersize[1], VM_PAPER_CUSTOM);
	translator.addPair(string_papersize[2], VM_PAPER_USLETTER);
	translator.addPair(string_papersize[3], VM_PAPER_USLEGAL);
	translator.addPair(string_papersize[4], VM_PAPER_USEXECUTIVE);
	translator.addPair(string_papersize[5], VM_PAPER_A3);
	translator.addPair(string_papersize[6], VM_PAPER_A4);
	translator.addPair(string_papersize[7], VM_PAPER_A5);
	translator.addPair(string_papersize[8], VM_PAPER_B3);
	translator.addPair(string_papersize[9], VM_PAPER_B4);
	translator.addPair(string_papersize[10], VM_PAPER_B5);
	return translator;
}


PaperSizeTranslator const & papersizetranslator()
{
	static PaperSizeTranslator translator = init_papersizetranslator();
	return translator;
}


// Paper packages
typedef Translator<string, PAPER_PACKAGES> PaperPackagesTranslator;


PaperPackagesTranslator const init_paperpackagestranslator()
{
	PaperPackagesTranslator translator(string_paperpackages[0], PACKAGE_NONE);
	translator.addPair(string_paperpackages[1], PACKAGE_A4);
	translator.addPair(string_paperpackages[2], PACKAGE_A4WIDE);
	translator.addPair(string_paperpackages[3], PACKAGE_WIDEMARGINSA4);
	return translator;
}


PaperPackagesTranslator const & paperpackagestranslator()
{
	static PaperPackagesTranslator translator = init_paperpackagestranslator();
	return translator;
}


// Paper orientation
typedef Translator<string, PAPER_ORIENTATION> PaperOrientationTranslator;


PaperOrientationTranslator const init_paperorientationtranslator()
{
	PaperOrientationTranslator translator(string_orientation[0], ORIENTATION_PORTRAIT);
	translator.addPair(string_orientation[1], ORIENTATION_LANDSCAPE);
	return translator;
}


PaperOrientationTranslator const & paperorientationtranslator()
{
	static PaperOrientationTranslator translator = init_paperorientationtranslator();
	return translator;
}


// Page sides
typedef Translator<int, LyXTextClass::PageSides> SidesTranslator;


SidesTranslator const init_sidestranslator()
{
	SidesTranslator translator(1, LyXTextClass::OneSide);
	translator.addPair(2, LyXTextClass::TwoSides);
	return translator;
}


SidesTranslator const & sidestranslator()
{
	static SidesTranslator translator = init_sidestranslator();
	return translator;
}



// AMS
typedef Translator<int, BufferParams::AMS> AMSTranslator;


AMSTranslator const init_amstranslator()
{
	AMSTranslator translator(0, BufferParams::AMS_OFF);
	translator.addPair(1, BufferParams::AMS_AUTO);
	translator.addPair(2, BufferParams::AMS_ON);
	return translator;
}


AMSTranslator const & amstranslator()
{
	static AMSTranslator translator = init_amstranslator();
	return translator;
}


// Cite engine
typedef Translator<string, biblio::CiteEngine> CiteEngineTranslator;


CiteEngineTranslator const init_citeenginetranslator()
{
	CiteEngineTranslator translator("basic", biblio::ENGINE_BASIC);
	translator.addPair("natbib_numerical", biblio::ENGINE_NATBIB_NUMERICAL);
	translator.addPair("natbib_authoryear", biblio::ENGINE_NATBIB_AUTHORYEAR);
	translator.addPair("jurabib", biblio::ENGINE_JURABIB);
	return translator;
}


CiteEngineTranslator const & citeenginetranslator()
{
	static CiteEngineTranslator translator = init_citeenginetranslator();
	return translator;
}


// Spacing
typedef Translator<string, Spacing::Space> SpaceTranslator;


SpaceTranslator const init_spacetranslator()
{
	SpaceTranslator translator("default", Spacing::Default);
	translator.addPair("single", Spacing::Single);
	translator.addPair("onehalf", Spacing::Onehalf);
	translator.addPair("double", Spacing::Double);
	return translator;
}


SpaceTranslator const & spacetranslator()
{
	static SpaceTranslator translator = init_spacetranslator();
	return translator;
}

// ends annonym namespace
}


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
	BOOST_ASSERT(ptr);

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
	cite_engine = biblio::ENGINE_BASIC;
	use_bibtopic = false;
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


Bullet & BufferParams::temp_bullet(lyx::size_type const index)
{
	BOOST_ASSERT(index < 4);
	return pimpl_->temp_bullets[index];
}


Bullet const & BufferParams::temp_bullet(lyx::size_type const index) const
{
	BOOST_ASSERT(index < 4);
	return pimpl_->temp_bullets[index];
}


Bullet & BufferParams::user_defined_bullet(lyx::size_type const index)
{
	BOOST_ASSERT(index < 4);
	return pimpl_->user_defined_bullets[index];
}


Bullet const & BufferParams::user_defined_bullet(lyx::size_type const index) const
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
		lex.next();
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
			string const msg =
				bformat(_("The document uses a missing "
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
		lex >> inputenc;
	} else if (token == "\\graphics") {
		readGraphicsDriver(lex);
	} else if (token == "\\fontscheme") {
		lex >> fonts;
	} else if (token == "\\paragraph_separation") {
		string parsep;
		lex >> parsep;
		paragraph_separation = parseptranslator().find(parsep);
	} else if (token == "\\defskip") {
		lex.next();
		pimpl_->defskip = VSpace(lex.getString());
	} else if (token == "\\quotes_language") {
		string quotes_lang;
		lex >> quotes_lang;
		quotes_language = quoteslangtranslator().find(quotes_lang);
	} else if (token == "\\quotes_times") {
		int qtimes;
		lex >> qtimes;
		quotes_times = quotestimestranslator().find(qtimes);
	} else if (token == "\\papersize") {
		string ppsize;
		lex >> ppsize;
		papersize2 = papersizetranslator().find(ppsize);
	} else if (token == "\\paperpackage") {
		string ppackage;
		lex >> ppackage;
		paperpackage = paperpackagestranslator().find(ppackage);
	} else if (token == "\\use_geometry") {
		lex >> use_geometry;
	} else if (token == "\\use_amsmath") {
		int use_ams;
		lex >> use_ams;
		use_amsmath = amstranslator().find(use_ams);
	} else if (token == "\\cite_engine") {
		string engine;
		lex >> engine;
		cite_engine = citeenginetranslator().find(engine);
	} else if (token == "\\use_bibtopic") {
		lex >> use_bibtopic;
	} else if (token == "\\tracking_changes") {
		lex >> tracking_changes;
	} else if (token == "\\branch") {
		lex.next();
		string branch = lex.getString();
		branchlist().add(branch);
		while (true) {
			lex.next();
			string const tok = lex.getString();
			if (tok == "\\end_branch")
				break;
			Branch * branch_ptr = branchlist().find(branch);
			if (tok == "\\selected") {
				lex.next();
				if (branch_ptr)
					branch_ptr->setSelected(lex.getInteger());
			}
			// not yet operational
			if (tok == "\\color") {
				lex.eatLine();
				string color = lex.getString();
				if (branch_ptr)
					branch_ptr->setColor(color);
				// Update also the LColor table:
				if (color == "none")
					color = lcolor.getX11Name(LColor::background);
				lcolor.setColor(branch, color);

			}
		}
	} else if (token == "\\author") {
		lex.eatLine();
		istringstream ss(lex.getString());
		Author a;
		ss >> a;
		author_map.push_back(pimpl_->authorlist.record(a));
	} else if (token == "\\paperorientation") {
		string orient;
		lex >> orient;
		orientation = paperorientationtranslator().find(orient);
	} else if (token == "\\paperwidth") {
		lex >> paperwidth;
	} else if (token == "\\paperheight") {
		lex >> paperheight;
	} else if (token == "\\leftmargin") {
		lex >> leftmargin;
	} else if (token == "\\topmargin") {
		lex >> topmargin;
	} else if (token == "\\rightmargin") {
		lex >> rightmargin;
	} else if (token == "\\bottommargin") {
		lex >> bottommargin;
	} else if (token == "\\headheight") {
		lex >> headheight;
	} else if (token == "\\headsep") {
		lex >> headsep;
	} else if (token == "\\footskip") {
		lex >> footskip;
	} else if (token == "\\paperfontsize") {
		lex >> fontsize;
	} else if (token == "\\papercolumns") {
		lex >> columns;
	} else if (token == "\\papersides") {
		int psides;
		lex >> psides;
		sides = sidestranslator().find(psides);
	} else if (token == "\\paperpagestyle") {
		lex >> pagestyle;
	} else if (token == "\\bullet") {
		readBullets(lex);
	} else if (token == "\\bulletLaTeX") {
		readBulletsLaTeX(lex);
	} else if (token == "\\secnumdepth") {
		lex >> secnumdepth;
	} else if (token == "\\tocdepth") {
		lex >> tocdepth;
	} else if (token == "\\spacing") {
		string nspacing;
		lex >> nspacing;
		float tmp_val = 0.0;
		if (nspacing == "other") {
			lex >> tmp_val;
		}
		spacing().set(spacetranslator().find(nspacing), tmp_val);
	} else if (token == "\\float_placement") {
		lex >> float_placement;
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

	// the options
	if (!options.empty()) {
		os << "\\options " << options << '\n';
	}

	// then the text parameters
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
	   << "\n\\use_geometry " << tostr(use_geometry)
	   << "\n\\use_amsmath " << use_amsmath
	   << "\n\\cite_engine " << citeenginetranslator().find(cite_engine)
	   << "\n\\use_bibtopic " << tostr(use_bibtopic)
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
	   << string_quotes_language[quotes_language] << '\n'
	   << "\\quotes_times "
	   << quotestimestranslator().find(quotes_times)
	   << "\n\\papercolumns " << columns
	   << "\n\\papersides " << sides
	   << "\n\\paperpagestyle " << pagestyle << '\n';
	for (int i = 0; i < 4; ++i) {
		if (user_defined_bullet(i) != ITEMIZE_DEFAULTS[i]) {
			if (user_defined_bullet(i).getFont() != -1) {
				os << "\\bullet " << i << " "
				   << user_defined_bullet(i).getFont() << " "
				   << user_defined_bullet(i).getCharacter() << " "
				   << user_defined_bullet(i).getSize() << "\n";
			}
			else {
				os << "\\bulletLaTeX " << i << " \""
				   << user_defined_bullet(i).getText()
				   << "\"\n";
			}
		}
	}

	os << "\\tracking_changes " << tostr(tracking_changes) << "\n";

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

	// If we use jurabib, we have to call babel here.
	if (use_babel && features.isRequired("jurabib")) {
		os << babelCall(language_options.str())
		   << '\n'
		   << features.getBabelOptions();
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
	// Jurabib has to be called after babel, though.
	if (use_babel && !features.isRequired("jurabib")) {
		lyxpreamble += babelCall(language_options.str()) + '\n';
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
			"\\dvipost{cbend color pop}\n";
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


void BufferParams::readBullets(LyXLex & lex)
{
	if (!lex.next()) return;

	int const index = lex.getInteger();
	lex.next();
	int temp_int = lex.getInteger();
	user_defined_bullet(index).setFont(temp_int);
	temp_bullet(index).setFont(temp_int);
	lex >> temp_int;
	user_defined_bullet(index).setCharacter(temp_int);
	temp_bullet(index).setCharacter(temp_int);
	lex >> temp_int;
	user_defined_bullet(index).setSize(temp_int);
	temp_bullet(index).setSize(temp_int);
}


void BufferParams::readBulletsLaTeX(LyXLex & lex)
{
	// The bullet class should be able to read this.
	if (!lex.next()) return;
	int const index = lex.getInteger();
	lex.next(true);
	string const temp_str = lex.getString();

	user_defined_bullet(index).setText(temp_str);
	temp_bullet(index).setText(temp_str);
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


string const BufferParams::babelCall(string const & lang_opts) const
{
	string tmp = lyxrc.language_package;
	if (!lyxrc.language_global_options && tmp == "\\usepackage{babel}")
		tmp = string("\\usepackage[") + lang_opts + "]{babel}";
	return tmp;
}
