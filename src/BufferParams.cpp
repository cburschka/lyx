/**
 * \file BufferParams.cpp
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

#include "BufferParams.h"

#include "Author.h"
#include "LayoutFile.h"
#include "BranchList.h"
#include "buffer_funcs.h"
#include "Bullet.h"
#include "Color.h"
#include "Encoding.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "ModuleList.h"
#include "Font.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "OutputParams.h"
#include "Spacing.h"
#include "TexRow.h"
#include "VSpace.h"
#include "PDFOptions.h"

#include "frontends/alert.h"

#include "insets/InsetListingsParams.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/Messages.h"
#include "support/Translator.h"
#include "support/lstrings.h"

#include <algorithm>
#include <sstream>

using namespace std;
using namespace lyx::support;


static char const * const string_paragraph_separation[] = {
	"indent", "skip", ""
};


static char const * const string_quotes_language[] = {
	"english", "swedish", "german", "polish", "french", "danish", ""
};


static char const * const string_papersize[] = {
	"default", "custom", "letterpaper", "legalpaper", "executivepaper",
	"a3paper", "a4paper", "a5paper", "b3paper", "b4paper", "b5paper", ""
};


static char const * const string_orientation[] = {
	"portrait", "landscape", ""
};


static char const * const string_footnotekinds[] = {
	"footnote", "margin", "fig", "tab", "alg", "wide-fig", "wide-tab", ""
};


static char const * const tex_graphics[] = {
	"default", "dvips", "dvitops", "emtex",
	"ln", "oztex", "textures", "none", ""
};


namespace lyx {

// Local translators
namespace {

// Paragraph separation
typedef Translator<string, BufferParams::ParagraphSeparation> ParSepTranslator;


ParSepTranslator const init_parseptranslator()
{
	ParSepTranslator translator
		(string_paragraph_separation[0], BufferParams::ParagraphIndentSeparation);
	translator.addPair(string_paragraph_separation[1], BufferParams::ParagraphSkipSeparation);
	return translator;
}


ParSepTranslator const & parseptranslator()
{
	static ParSepTranslator translator = init_parseptranslator();
	return translator;
}


// Quotes language
typedef Translator<string, InsetQuotes::QuoteLanguage> QuotesLangTranslator;


QuotesLangTranslator const init_quoteslangtranslator()
{
	QuotesLangTranslator translator
		(string_quotes_language[0], InsetQuotes::EnglishQuotes);
	translator.addPair(string_quotes_language[1], InsetQuotes::SwedishQuotes);
	translator.addPair(string_quotes_language[2], InsetQuotes::GermanQuotes);
	translator.addPair(string_quotes_language[3], InsetQuotes::PolishQuotes);
	translator.addPair(string_quotes_language[4], InsetQuotes::FrenchQuotes);
	translator.addPair(string_quotes_language[5], InsetQuotes::DanishQuotes);
	return translator;
}


QuotesLangTranslator const & quoteslangtranslator()
{
	static QuotesLangTranslator translator = init_quoteslangtranslator();
	return translator;
}


// Paper size
typedef Translator<string, PAPER_SIZE> PaperSizeTranslator;


static PaperSizeTranslator initPaperSizeTranslator()
{
	PaperSizeTranslator translator(string_papersize[0], PAPER_DEFAULT);
	translator.addPair(string_papersize[1], PAPER_CUSTOM);
	translator.addPair(string_papersize[2], PAPER_USLETTER);
	translator.addPair(string_papersize[3], PAPER_USLEGAL);
	translator.addPair(string_papersize[4], PAPER_USEXECUTIVE);
	translator.addPair(string_papersize[5], PAPER_A3);
	translator.addPair(string_papersize[6], PAPER_A4);
	translator.addPair(string_papersize[7], PAPER_A5);
	translator.addPair(string_papersize[8], PAPER_B3);
	translator.addPair(string_papersize[9], PAPER_B4);
	translator.addPair(string_papersize[10], PAPER_B5);
	return translator;
}


PaperSizeTranslator const & papersizetranslator()
{
	static PaperSizeTranslator translator = initPaperSizeTranslator();
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
typedef Translator<int, PageSides> SidesTranslator;


SidesTranslator const init_sidestranslator()
{
	SidesTranslator translator(1, OneSide);
	translator.addPair(2, TwoSides);
	return translator;
}


SidesTranslator const & sidestranslator()
{
	static SidesTranslator translator = init_sidestranslator();
	return translator;
}


// LaTeX packages
typedef Translator<int, BufferParams::Package> PackageTranslator;


PackageTranslator const init_packagetranslator()
{
	PackageTranslator translator(0, BufferParams::package_off);
	translator.addPair(1, BufferParams::package_auto);
	translator.addPair(2, BufferParams::package_on);
	return translator;
}


PackageTranslator const & packagetranslator()
{
	static PackageTranslator translator = init_packagetranslator();
	return translator;
}


// Cite engine
typedef Translator<string, CiteEngine> CiteEngineTranslator;


CiteEngineTranslator const init_citeenginetranslator()
{
	CiteEngineTranslator translator("basic", ENGINE_BASIC);
	translator.addPair("natbib_numerical", ENGINE_NATBIB_NUMERICAL);
	translator.addPair("natbib_authoryear", ENGINE_NATBIB_AUTHORYEAR);
	translator.addPair("jurabib", ENGINE_JURABIB);
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
	translator.addPair("other", Spacing::Other);
	return translator;
}


SpaceTranslator const & spacetranslator()
{
	static SpaceTranslator translator = init_spacetranslator();
	return translator;
}


} // anon namespace


class BufferParams::Impl
{
public:
	Impl();

	AuthorList authorlist;
	BranchList branchlist;
	Bullet temp_bullets[4];
	Bullet user_defined_bullets[4];
	Spacing spacing;
	/** This is the amount of space used for paragraph_separation "skip",
	 * and for detached paragraphs in "indented" documents.
	 */
	VSpace defskip;
	PDFOptions pdfoptions;
	LayoutFileIndex baseClass_;
};


BufferParams::Impl::Impl()
	: defskip(VSpace::MEDSKIP), baseClass_(string(""))
{
	// set initial author
	// FIXME UNICODE
	authorlist.record(Author(from_utf8(lyxrc.user_name), from_utf8(lyxrc.user_email)));
}


BufferParams::Impl *
BufferParams::MemoryTraits::clone(BufferParams::Impl const * ptr)
{
	LASSERT(ptr, /**/);

	return new BufferParams::Impl(*ptr);
}


void BufferParams::MemoryTraits::destroy(BufferParams::Impl * ptr)
{
	delete ptr;
}


BufferParams::BufferParams()
	: pimpl_(new Impl)
{
	setBaseClass(defaultBaseclass());
	makeDocumentClass();
	paragraph_separation = ParagraphIndentSeparation;
	quotes_language = InsetQuotes::EnglishQuotes;
	fontsize = "default";

	/*  PaperLayout */
	papersize = PAPER_DEFAULT;
	orientation = ORIENTATION_PORTRAIT;
	use_geometry = false;
	use_amsmath = package_auto;
	use_esint = package_auto;
	cite_engine_ = ENGINE_BASIC;
	use_bibtopic = false;
	trackChanges = false;
	outputChanges = false;
	secnumdepth = 3;
	tocdepth = 3;
	language = default_language;
	fontsRoman = "default";
	fontsSans = "default";
	fontsTypewriter = "default";
	fontsDefaultFamily = "default";
	fontsSC = false;
	fontsOSF = false;
	fontsSansScale = 100;
	fontsTypewriterScale = 100;
	inputenc = "auto";
	graphicsDriver = "default";
	sides = OneSide;
	columns = 1;
	listings_params = string();
	pagestyle = "default";
	compressed = false;
	for (int iter = 0; iter < 4; ++iter) {
		user_defined_bullet(iter) = ITEMIZE_DEFAULTS[iter];
		temp_bullet(iter) = ITEMIZE_DEFAULTS[iter];
	}
}


docstring BufferParams::B_(string const & l10n) const
{
	LASSERT(language, /**/);
	return getMessages(language->code()).get(l10n);
}


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
	LASSERT(index < 4, /**/);
	return pimpl_->temp_bullets[index];
}


Bullet const & BufferParams::temp_bullet(lyx::size_type const index) const
{
	LASSERT(index < 4, /**/);
	return pimpl_->temp_bullets[index];
}


Bullet & BufferParams::user_defined_bullet(lyx::size_type const index)
{
	LASSERT(index < 4, /**/);
	return pimpl_->user_defined_bullets[index];
}


Bullet const & BufferParams::user_defined_bullet(lyx::size_type const index) const
{
	LASSERT(index < 4, /**/);
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


PDFOptions & BufferParams::pdfoptions()
{
	return pimpl_->pdfoptions;
}


PDFOptions const & BufferParams::pdfoptions() const
{
	return pimpl_->pdfoptions;
}


VSpace const & BufferParams::getDefSkip() const
{
	return pimpl_->defskip;
}


void BufferParams::setDefSkip(VSpace const & vs)
{
	pimpl_->defskip = vs;
}


string BufferParams::readToken(Lexer & lex, string const & token,
	FileName const & filepath)
{
	if (token == "\\textclass") {
		lex.next();
		string const classname = lex.getString();
		// if there exists a local layout file, ignore the system one
		// NOTE: in this case, the textclass (.cls file) is assumed to be available.
		string tcp;
		LayoutFileList & bcl = LayoutFileList::get();
		if (tcp.empty() && !filepath.empty())
			tcp = bcl.addLocalLayout(classname, filepath.absFilename());
		if (!tcp.empty())
			setBaseClass(tcp);
		else
			setBaseClass(classname);
		// We assume that a tex class exists for local or unknown layouts so this warning
		// will only be given for system layouts.
		if (!baseClass()->isTeXClassAvailable()) {
			docstring const msg =
				bformat(_("The layout file requested by this document,\n"
						 "%1$s.layout,\n"
						 "is not usable. This is probably because a LaTeX\n"
						 "class or style file required by it is not\n"
						 "available. See the Customization documentation\n"
						 "for more information.\n"), from_utf8(classname));
			frontend::Alert::warning(_("Document class not available"),
				       msg + _("LyX will not be able to produce output."));
		} 
	} else if (token == "\\begin_preamble") {
		readPreamble(lex);
	} else if (token == "\\begin_local_layout") {
		readLocalLayout(lex);
	} else if (token == "\\begin_modules") {
		readModules(lex);
	} else if (token == "\\begin_removed_modules") {
		readRemovedModules(lex);
	} else if (token == "\\options") {
		lex.eatLine();
		options = lex.getString();
	} else if (token == "\\master") {
		lex.eatLine();
		master = lex.getString();
	} else if (token == "\\language") {
		readLanguage(lex);
	} else if (token == "\\inputencoding") {
		lex >> inputenc;
	} else if (token == "\\graphics") {
		readGraphicsDriver(lex);
	} else if (token == "\\font_roman") {
		lex >> fontsRoman;
	} else if (token == "\\font_sans") {
		lex >> fontsSans;
	} else if (token == "\\font_typewriter") {
		lex >> fontsTypewriter;
	} else if (token == "\\font_default_family") {
		lex >> fontsDefaultFamily;
	} else if (token == "\\font_sc") {
		lex >> fontsSC;
	} else if (token == "\\font_osf") {
		lex >> fontsOSF;
	} else if (token == "\\font_sf_scale") {
		lex >> fontsSansScale;
	} else if (token == "\\font_tt_scale") {
		lex >> fontsTypewriterScale;
	} else if (token == "\\font_cjk") {
		lex >> fontsCJK;
	} else if (token == "\\paragraph_separation") {
		string parsep;
		lex >> parsep;
		paragraph_separation = parseptranslator().find(parsep);
	} else if (token == "\\defskip") {
		lex.next();
		string defskip = lex.getString();
		if (defskip == "defskip")
			// this is invalid
			defskip = "medskip";
		pimpl_->defskip = VSpace(defskip);
	} else if (token == "\\quotes_language") {
		string quotes_lang;
		lex >> quotes_lang;
		quotes_language = quoteslangtranslator().find(quotes_lang);
	} else if (token == "\\papersize") {
		string ppsize;
		lex >> ppsize;
		papersize = papersizetranslator().find(ppsize);
	} else if (token == "\\use_geometry") {
		lex >> use_geometry;
	} else if (token == "\\use_amsmath") {
		int use_ams;
		lex >> use_ams;
		use_amsmath = packagetranslator().find(use_ams);
	} else if (token == "\\use_esint") {
		int useesint;
		lex >> useesint;
		use_esint = packagetranslator().find(useesint);
	} else if (token == "\\cite_engine") {
		string engine;
		lex >> engine;
		cite_engine_ = citeenginetranslator().find(engine);
	} else if (token == "\\use_bibtopic") {
		lex >> use_bibtopic;
	} else if (token == "\\tracking_changes") {
		lex >> trackChanges;
	} else if (token == "\\output_changes") {
		lex >> outputChanges;
	} else if (token == "\\branch") {
		lex.next();
		docstring branch = lex.getDocString();
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
				// Update also the Color table:
				if (color == "none")
					color = lcolor.getX11Name(Color_background);
				// FIXME UNICODE
				lcolor.setColor(to_utf8(branch), color);

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
	} else if (token == "\\columnsep") {
		lex >> columnsep;
	} else if (token == "\\paperfontsize") {
		lex >> fontsize;
	} else if (token == "\\papercolumns") {
		lex >> columns;
	} else if (token == "\\listings_params") {
		string par;
		lex >> par;
		listings_params = InsetListingsParams(par).params();
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
		string tmp_val;
		if (nspacing == "other") {
			lex >> tmp_val;
		}
		spacing().set(spacetranslator().find(nspacing), tmp_val);
	} else if (token == "\\float_placement") {
		lex >> float_placement;

	} else if (prefixIs(token, "\\pdf_") || token == "\\use_hyperref") {
		string toktmp = pdfoptions().readToken(lex, token);
		if (!toktmp.empty()) {
			lyxerr << "PDFOptions::readToken(): Unknown token: " <<
				toktmp << endl;
			return toktmp;
		}
	} else {
		lyxerr << "BufferParams::readToken(): Unknown token: " << 
			token << endl;
		return token;
	}

	return string();
}


void BufferParams::writeFile(ostream & os) const
{
	// The top of the file is written by the buffer.
	// Prints out the buffer info into the .lyx file given by file

	// the textclass
	os << "\\textclass " << baseClass()->name() << '\n';

	// then the preamble
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

	// the master document
	if (!master.empty()) {
		os << "\\master " << master << '\n';
	}
	
	// removed modules
	if (!removedModules_.empty()) {
		os << "\\begin_removed_modules" << '\n';
		set<string>::const_iterator it = removedModules_.begin();
		set<string>::const_iterator en = removedModules_.end();
		for (; it != en; it++)
			os << *it << '\n';
		os << "\\end_removed_modules" << '\n';
	}

	// the modules
	if (!layoutModules_.empty()) {
		os << "\\begin_modules" << '\n';
		LayoutModuleList::const_iterator it = layoutModules_.begin();
		LayoutModuleList::const_iterator en = layoutModules_.end();
		for (; it != en; it++)
			os << *it << '\n';
		os << "\\end_modules" << '\n';
	}
	
	// local layout information
	if (!local_layout.empty()) {
		// remove '\n' from the end 
		string const tmplocal = rtrim(local_layout, "\n");
		os << "\\begin_local_layout\n"
		   << tmplocal
		   << "\n\\end_local_layout\n";
	}

	// then the text parameters
	if (language != ignore_language)
		os << "\\language " << language->lang() << '\n';
	os << "\\inputencoding " << inputenc
	   << "\n\\font_roman " << fontsRoman
	   << "\n\\font_sans " << fontsSans
	   << "\n\\font_typewriter " << fontsTypewriter
	   << "\n\\font_default_family " << fontsDefaultFamily
	   << "\n\\font_sc " << convert<string>(fontsSC)
	   << "\n\\font_osf " << convert<string>(fontsOSF)
	   << "\n\\font_sf_scale " << fontsSansScale
	   << "\n\\font_tt_scale " << fontsTypewriterScale
	   << '\n';
	if (!fontsCJK.empty()) {
		os << "\\font_cjk " << fontsCJK << '\n';
	}
	os << "\n\\graphics " << graphicsDriver << '\n';

	if (!float_placement.empty()) {
		os << "\\float_placement " << float_placement << '\n';
	}
	os << "\\paperfontsize " << fontsize << '\n';

	spacing().writeFile(os);
	pdfoptions().writeFile(os);

	os << "\\papersize " << string_papersize[papersize]
	   << "\n\\use_geometry " << convert<string>(use_geometry)
	   << "\n\\use_amsmath " << use_amsmath
	   << "\n\\use_esint " << use_esint
	   << "\n\\cite_engine " << citeenginetranslator().find(cite_engine_)
	   << "\n\\use_bibtopic " << convert<string>(use_bibtopic)
	   << "\n\\paperorientation " << string_orientation[orientation]
	   << '\n';

	BranchList::const_iterator it = branchlist().begin();
	BranchList::const_iterator end = branchlist().end();
	for (; it != end; ++it) {
		os << "\\branch " << to_utf8(it->getBranch())
		   << "\n\\selected " << it->getSelected()
		   << "\n\\color " << lyx::X11hexname(it->getColor())
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
	if (!columnsep.empty())
		os << "\\columnsep " 
			 << VSpace(columnsep).asLyXCommand() << '\n';
	os << "\\secnumdepth " << secnumdepth
	   << "\n\\tocdepth " << tocdepth
	   << "\n\\paragraph_separation "
	   << string_paragraph_separation[paragraph_separation]
	   << "\n\\defskip " << getDefSkip().asLyXCommand()
	   << "\n\\quotes_language "
	   << string_quotes_language[quotes_language]
	   << "\n\\papercolumns " << columns
	   << "\n\\papersides " << sides
	   << "\n\\paperpagestyle " << pagestyle << '\n';
	if (!listings_params.empty())
		os << "\\listings_params \"" <<
			InsetListingsParams(listings_params).encodedString() << "\"\n";
	for (int i = 0; i < 4; ++i) {
		if (user_defined_bullet(i) != ITEMIZE_DEFAULTS[i]) {
			if (user_defined_bullet(i).getFont() != -1) {
				os << "\\bullet " << i << " "
				   << user_defined_bullet(i).getFont() << " "
				   << user_defined_bullet(i).getCharacter() << " "
				   << user_defined_bullet(i).getSize() << "\n";
			}
			else {
				// FIXME UNICODE
				os << "\\bulletLaTeX " << i << " \""
				   << lyx::to_ascii(user_defined_bullet(i).getText())
				   << "\"\n";
			}
		}
	}

	os << "\\tracking_changes " << convert<string>(trackChanges) << "\n";
	os << "\\output_changes " << convert<string>(outputChanges) << "\n";

	AuthorList::Authors::const_iterator a_it = pimpl_->authorlist.begin();
	AuthorList::Authors::const_iterator a_end = pimpl_->authorlist.end();
	for (; a_it != a_end; ++a_it) {
		if (a_it->second.used())
			os << "\\author " << a_it->second << "\n";
		else
			os << "\\author " << Author() << "\n";
	}
}


void BufferParams::validate(LaTeXFeatures & features) const
{
	features.require(documentClass().requires());

	if (outputChanges) {
		bool dvipost    = LaTeXFeatures::isAvailable("dvipost");
		bool xcolorsoul = LaTeXFeatures::isAvailable("soul") &&
				  LaTeXFeatures::isAvailable("xcolor");

		switch (features.runparams().flavor) {
		case OutputParams::LATEX:
			if (dvipost) {
				features.require("ct-dvipost");
				features.require("dvipost");
			} else if (xcolorsoul) {
				features.require("ct-xcolor-soul");
				features.require("soul");
				features.require("xcolor");
			} else {
				features.require("ct-none");
			}
			break;
		case OutputParams::PDFLATEX:
			if (xcolorsoul) {
				features.require("ct-xcolor-soul");
				features.require("soul");
				features.require("xcolor");
				// improves color handling in PDF output
				features.require("pdfcolmk"); 
			} else {
				features.require("ct-none");
			}
			break;
		default:
			break;
		}
	}

	// Floats with 'Here definitely' as default setting.
	if (float_placement.find('H') != string::npos)
		features.require("float");

	// AMS Style is at document level
	if (use_amsmath == package_on
	    || documentClass().provides("amsmath"))
		features.require("amsmath");
	if (use_esint == package_on)
		features.require("esint");

	// Document-level line spacing
	if (spacing().getSpace() != Spacing::Single && !spacing().isDefault())
		features.require("setspace");

	// the bullet shapes are buffer level not paragraph level
	// so they are tested here
	for (int i = 0; i < 4; ++i) {
		if (user_defined_bullet(i) == ITEMIZE_DEFAULTS[i]) 
			continue;
		int const font = user_defined_bullet(i).getFont();
		if (font == 0) {
			int const c = user_defined_bullet(i).getCharacter();
			if (c == 16
			    || c == 17
			    || c == 25
			    || c == 26
			    || c == 31) {
				features.require("latexsym");
			}
		} else if (font == 1) {
			features.require("amssymb");
		} else if (font >= 2 && font <= 5) {
			features.require("pifont");
		}
	}

	if (pdfoptions().use_hyperref)
		features.require("hyperref");

	if (language->lang() == "vietnamese")
		features.require("vietnamese");
	else if (language->lang() == "japanese")
		features.require("japanese");
}


bool BufferParams::writeLaTeX(odocstream & os, LaTeXFeatures & features,
			      TexRow & texrow) const
{
	os << "\\documentclass";

	DocumentClass const & tclass = documentClass();

	ostringstream clsoptions; // the document class options.

	if (tokenPos(tclass.opt_fontsize(),
		     '|', fontsize) >= 0) {
		// only write if existing in list (and not default)
		clsoptions << fontsize << "pt,";
	}

	// custom, A3, B3 and B4 paper sizes need geometry
	bool nonstandard_papersize = papersize == PAPER_B3
		|| papersize == PAPER_B4
		|| papersize == PAPER_A3
		|| papersize == PAPER_CUSTOM;

	if (!use_geometry) {
		switch (papersize) {
		case PAPER_A4:
			clsoptions << "a4paper,";
			break;
		case PAPER_USLETTER:
			clsoptions << "letterpaper,";
			break;
		case PAPER_A5:
			clsoptions << "a5paper,";
			break;
		case PAPER_B5:
			clsoptions << "b5paper,";
			break;
		case PAPER_USEXECUTIVE:
			clsoptions << "executivepaper,";
			break;
		case PAPER_USLEGAL:
			clsoptions << "legalpaper,";
			break;
		case PAPER_DEFAULT:
		case PAPER_A3:
		case PAPER_B3:
		case PAPER_B4:
		case PAPER_CUSTOM:
			break;
		}
	}

	// if needed
	if (sides != tclass.sides()) {
		switch (sides) {
		case OneSide:
			clsoptions << "oneside,";
			break;
		case TwoSides:
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
		if (!language->babel().empty()) {
			if (!language_options.str().empty())
				language_options << ',';
			language_options << language->babel();
		}
		// if Vietnamese is used, babel must directly be loaded
		// with language options, not in the class options, see
		// http://www.mail-archive.com/lyx-devel@lists.lyx.org/msg129417.html
		size_t viet = language_options.str().find("vietnam");
		// viet = string::npos when not found
		// if Japanese is used, babel must directly be loaded
		// with language options, not in the class options, see
		// http://bugzilla.lyx.org/show_bug.cgi?id=4597#c4
		size_t japan = language_options.str().find("japanese");
		// japan = string::npos when not found
		if (lyxrc.language_global_options
			&& !language_options.str().empty()
			&& viet == string::npos && japan == string::npos)
			clsoptions << language_options.str() << ',';
	}

	// the user-defined options
	if (!options.empty()) {
		clsoptions << options << ',';
	}

	string strOptions(clsoptions.str());
	if (!strOptions.empty()) {
		strOptions = rtrim(strOptions, ",");
		// FIXME UNICODE
		os << '[' << from_utf8(strOptions) << ']';
	}

	os << '{' << from_ascii(tclass.latexname()) << "}\n";
	texrow.newline();
	// end of \documentclass defs

	// font selection must be done before loading fontenc.sty
	string const fonts =
		loadFonts(fontsRoman, fontsSans,
			  fontsTypewriter, fontsSC, fontsOSF,
			  fontsSansScale, fontsTypewriterScale);
	if (!fonts.empty()) {
		os << from_ascii(fonts);
		texrow.newline();
	}
	if (fontsDefaultFamily != "default")
		os << "\\renewcommand{\\familydefault}{\\"
		   << from_ascii(fontsDefaultFamily) << "}\n";

	// set font encoding
	// this one is not per buffer
	// for arabic_arabi and farsi we also need to load the LAE and
	// LFE encoding
	if (lyxrc.fontenc != "default" && language->lang() != "japanese") {
		if (language->lang() == "arabic_arabi"
		    || language->lang() == "farsi") {
			os << "\\usepackage[" << from_ascii(lyxrc.fontenc)
			   << ",LFE,LAE]{fontenc}\n";
			texrow.newline();
		} else {
			os << "\\usepackage[" << from_ascii(lyxrc.fontenc)
			   << "]{fontenc}\n";
			texrow.newline();
		}
	}

	// handle inputenc etc.
	writeEncodingPreamble(os, features, texrow);

	if (!listings_params.empty() || features.isRequired("listings")) {
		os << "\\usepackage{listings}\n";
		texrow.newline();
	}
	if (!listings_params.empty()) {
		os << "\\lstset{";
		// do not test validity because listings_params is 
		// supposed to be valid
		string par =
			InsetListingsParams(listings_params).separatedParams(true);
		// we can't support all packages, but we should load the color package
		if (par.find("\\color", 0) != string::npos)
			features.require("color");
		os << from_utf8(par);
		// count the number of newlines
		for (size_t i = 0; i < par.size(); ++i)
			if (par[i] == '\n')
				texrow.newline();
		os << "}\n";
		texrow.newline();
	}
	if (use_geometry || nonstandard_papersize) {
		os << "\\usepackage{geometry}\n";
		texrow.newline();
		os << "\\geometry{verbose";
		if (orientation == ORIENTATION_LANDSCAPE)
			os << ",landscape";
		switch (papersize) {
		case PAPER_CUSTOM:
			if (!paperwidth.empty())
				os << ",paperwidth="
				   << from_ascii(paperwidth);
			if (!paperheight.empty())
				os << ",paperheight="
				   << from_ascii(paperheight);
			break;
		case PAPER_USLETTER:
			os << ",letterpaper";
			break;
		case PAPER_USLEGAL:
			os << ",legalpaper";
			break;
		case PAPER_USEXECUTIVE:
			os << ",executivepaper";
			break;
		case PAPER_A3:
			os << ",a3paper";
			break;
		case PAPER_A4:
			os << ",a4paper";
			break;
		case PAPER_A5:
			os << ",a5paper";
			break;
		case PAPER_B3:
			os << ",b3paper";
			break;
		case PAPER_B4:
			os << ",b4paper";
			break;
		case PAPER_B5:
			os << ",b5paper";
			break;
		default:
			// default papersize ie PAPER_DEFAULT
			switch (lyxrc.default_papersize) {
			case PAPER_DEFAULT: // keep compiler happy
			case PAPER_USLETTER:
				os << ",letterpaper";
				break;
			case PAPER_USLEGAL:
				os << ",legalpaper";
				break;
			case PAPER_USEXECUTIVE:
				os << ",executivepaper";
				break;
			case PAPER_A3:
				os << ",a3paper";
				break;
			case PAPER_A4:
				os << ",a4paper";
				break;
			case PAPER_A5:
				os << ",a5paper";
				break;
			case PAPER_B5:
				os << ",b5paper";
				break;
			case PAPER_B3:
			case PAPER_B4:
			case PAPER_CUSTOM:
				break;
			}
		}
		if (!topmargin.empty())
			os << ",tmargin=" << from_ascii(Length(topmargin).asLatexString());
		if (!bottommargin.empty())
			os << ",bmargin=" << from_ascii(Length(bottommargin).asLatexString());
		if (!leftmargin.empty())
			os << ",lmargin=" << from_ascii(Length(leftmargin).asLatexString());
		if (!rightmargin.empty())
			os << ",rmargin=" << from_ascii(Length(rightmargin).asLatexString());
		if (!headheight.empty())
			os << ",headheight=" << from_ascii(Length(headheight).asLatexString());
		if (!headsep.empty())
			os << ",headsep=" << from_ascii(Length(headsep).asLatexString());
		if (!footskip.empty())
			os << ",footskip=" << from_ascii(Length(footskip).asLatexString());
		if (!columnsep.empty())
			os << ",columnsep=" << from_ascii(Length(columnsep).asLatexString());
		os << "}\n";
		texrow.newline();
	} else if (orientation == ORIENTATION_LANDSCAPE) {
		features.require("papersize");
	}

	if (tokenPos(tclass.opt_pagestyle(),
		     '|', pagestyle) >= 0) {
		if (pagestyle == "fancy") {
			os << "\\usepackage{fancyhdr}\n";
			texrow.newline();
		}
		os << "\\pagestyle{" << from_ascii(pagestyle) << "}\n";
		texrow.newline();
	}

	// Only if class has a ToC hierarchy
	if (tclass.hasTocLevels()) {
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
	}

	if (paragraph_separation) {
		switch (getDefSkip().kind()) {
		case VSpace::SMALLSKIP:
			os << "\\setlength{\\parskip}{\\smallskipamount}\n";
			break;
		case VSpace::MEDSKIP:
			os << "\\setlength{\\parskip}{\\medskipamount}\n";
			break;
		case VSpace::BIGSKIP:
			os << "\\setlength{\\parskip}{\\bigskipamount}\n";
			break;
		case VSpace::LENGTH:
			os << "\\setlength{\\parskip}{"
			   << from_utf8(getDefSkip().length().asLatexString())
			   << "}\n";
			break;
		default: // should never happen // Then delete it.
			os << "\\setlength{\\parskip}{\\medskipamount}\n";
			break;
		}
		texrow.newline();

		os << "\\setlength{\\parindent}{0pt}\n";
		texrow.newline();
	}

	// If we use hyperref, jurabib, japanese, or vietnamese, we have to call babel here.
	if (use_babel
		&& (features.isRequired("jurabib")
			|| features.isRequired("hyperref")
			|| features.isRequired("vietnamese")
			|| features.isRequired("japanese") ) ) {
		// FIXME UNICODE
		os << from_utf8(babelCall(language_options.str()))
		   << '\n'
		   << from_utf8(features.getBabelOptions());
		texrow.newline();
	}

	// Now insert the LyX specific LaTeX commands...

	// The optional packages;
	docstring lyxpreamble(from_ascii(features.getPackages()));

	// Line spacing
	lyxpreamble += from_utf8(spacing().writePreamble(tclass.provides("SetSpace")));

	// PDF support.
	// * Hyperref manual: "Make sure it comes last of your loaded
	//   packages, to give it a fighting chance of not being over-written,
	//   since its job is to redefine many LATEX commands."
	// * Email from Heiko Oberdiek: "It is usually better to load babel
	//   before hyperref. Then hyperref has a chance to detect babel.
	// * Has to be loaded before the "LyX specific LaTeX commands" to
	//   avoid errors with algorithm floats.
	// use hyperref explicitely when it is required
	if (features.isRequired("hyperref")) {
		odocstringstream oss;
		pdfoptions().writeLaTeX(oss, documentClass().provides("hyperref"));
		lyxpreamble += oss.str();
	}

	// Will be surrounded by \makeatletter and \makeatother when needed
	docstring atlyxpreamble;

	// Some macros LyX will need
	docstring tmppreamble(from_ascii(features.getMacros()));

	if (!tmppreamble.empty())
		atlyxpreamble += "\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% "
			"LyX specific LaTeX commands.\n"
			+ tmppreamble + '\n';

	// the text class specific preamble
	tmppreamble = features.getTClassPreamble();
	if (!tmppreamble.empty())
		atlyxpreamble += "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% "
			"Textclass specific LaTeX commands.\n"
			+ tmppreamble + '\n';

	/* the user-defined preamble */
	if (!preamble.empty())
		// FIXME UNICODE
		atlyxpreamble += "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% "
			"User specified LaTeX commands.\n"
			+ from_utf8(preamble) + '\n';

	// subfig loads internally the LaTeX package "caption". As
	// caption is a very popular package, users will load it in
	// the preamble. Therefore we must load subfig behind the
	// user-defined preamble and check if the caption package was
	// loaded or not. For the case that caption is loaded before
	// subfig, there is the subfig option "caption=false". This
	// option also works when a koma-script class is used and
	// koma's own caption commands are used instead of caption. We
	// use \PassOptionsToPackage here because the user could have
	// already loaded subfig in the preamble.
	if (features.isRequired("subfig")) {
		atlyxpreamble += "\\@ifundefined{showcaptionsetup}{}{%\n"
			" \\PassOptionsToPackage{caption=false}{subfig}}\n"
			"\\usepackage{subfig}\n";
	}

	// Itemize bullet settings need to be last in case the user
	// defines their own bullets that use a package included
	// in the user-defined preamble -- ARRae
	// Actually it has to be done much later than that
	// since some packages like frenchb make modifications
	// at \begin{document} time -- JMarc
	docstring bullets_def;
	for (int i = 0; i < 4; ++i) {
		if (user_defined_bullet(i) != ITEMIZE_DEFAULTS[i]) {
			if (bullets_def.empty())
				bullets_def += "\\AtBeginDocument{\n";
			bullets_def += "  \\def\\labelitemi";
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
			bullets_def += '{' +
				user_defined_bullet(i).getText()
				+ "}\n";
		}
	}

	if (!bullets_def.empty())
		atlyxpreamble += bullets_def + "}\n\n";

	if (atlyxpreamble.find(from_ascii("@")) != docstring::npos)
		lyxpreamble += "\n\\makeatletter\n"
			+ atlyxpreamble + "\\makeatother\n\n";
	else
		lyxpreamble += '\n' + atlyxpreamble;

	// We try to load babel late, in case it interferes
	// with other packages.
	// Jurabib and Hyperref have to be called after babel, though.
	if (use_babel && !features.isRequired("jurabib")
	    && !features.isRequired("hyperref")
	    && !features.isRequired("vietnamese")
	    && !features.isRequired("japanese")) {
		// FIXME UNICODE
		lyxpreamble += from_utf8(babelCall(language_options.str())) + '\n';
		lyxpreamble += from_utf8(features.getBabelOptions()) + '\n';
	}

	int const nlines =
		int(count(lyxpreamble.begin(), lyxpreamble.end(), '\n'));
	for (int j = 0; j != nlines; ++j) {
		texrow.newline();
	}

	os << lyxpreamble;
	return use_babel;
}


void BufferParams::useClassDefaults()
{
	DocumentClass const & tclass = documentClass();

	sides = tclass.sides();
	columns = tclass.columns();
	pagestyle = tclass.pagestyle();
	options = tclass.options();
	// Only if class has a ToC hierarchy
	if (tclass.hasTocLevels()) {
		secnumdepth = tclass.secnumdepth();
		tocdepth = tclass.tocdepth();
	}
}


bool BufferParams::hasClassDefaults() const
{
	DocumentClass const & tclass = documentClass();

	return sides == tclass.sides()
		&& columns == tclass.columns()
		&& pagestyle == tclass.pagestyle()
		&& options == tclass.options()
		&& secnumdepth == tclass.secnumdepth()
		&& tocdepth == tclass.tocdepth();
}


DocumentClass const & BufferParams::documentClass() const
{
	return *doc_class_;
}


DocumentClass const * BufferParams::documentClassPtr() const {
	return doc_class_;
}


void BufferParams::setDocumentClass(DocumentClass const * const tc) {
	// evil, but this function is evil
	doc_class_ = const_cast<DocumentClass *>(tc);
}


bool BufferParams::setBaseClass(string const & classname)
{
	LYXERR(Debug::TCLASS, "setBaseClass: " << classname);
	LayoutFileList & bcl = LayoutFileList::get();
	if (!bcl.haveClass(classname)) {
		docstring s = 
			bformat(_("The document class %1$s could not be found. "
				"A default textclass with default layouts will be used. "
				"LyX might not be able to produce output unless a correct "
				"textclass is selected from the document settings dialog."),
			from_utf8(classname));
		frontend::Alert::error(_("Document class not found"), s);
		bcl.addEmptyClass(classname);
	}

	bool const success = bcl[classname].load();
	if (!success) {	
		docstring s = 
			bformat(_("The document class %1$s could not be loaded."),
			from_utf8(classname));
		frontend::Alert::error(_("Could not load class"), s);
		return false;
	}

	pimpl_->baseClass_ = classname;

	// add any required modules not already in use
	set<string> const & mods = baseClass()->defaultModules();
	set<string>::const_iterator mit = mods.begin();
	set<string>::const_iterator men = mods.end();
	for (; mit != men; mit++) {
		string const & modName = *mit;
		// see if we're already in use
		if (find(layoutModules_.begin(), layoutModules_.end(), modName) != 
		    layoutModules_.end()) {
			LYXERR(Debug::TCLASS, "Default module `" << modName << 
					"' not added because already used.");
			continue;
		}
		// make sure the user hasn't removed it
		if (find(removedModules_.begin(), removedModules_.end(), modName) != 
		    removedModules_.end()) {
			LYXERR(Debug::TCLASS, "Default module `" << modName << 
					"' not added because removed by user.");
			continue;
		}
		// Now we want to check the list of selected modules to see if any of them
		// exclude this one.
		bool foundit = false;
		// so iterate over the selected modules...
		LayoutModuleList::const_iterator lit = layoutModules_.begin();
		LayoutModuleList::const_iterator len = layoutModules_.end();
		for (; lit != len; lit++) {
			LyXModule * lm = moduleList[*lit];
			if (!lm)
				continue;
			vector<string> const & exc = lm->getExcludedModules();
			// ...and see if this one excludes us.
			if (find(exc.begin(), exc.end(), modName) != exc.end()) {
				foundit = true;
				LYXERR(Debug::TCLASS, "Default module `" << modName << 
						"' not added because excluded by loaded module `" << 
						*lit << "'.");
				break;
			}
		}
		if (!foundit) {
			LYXERR(Debug::TCLASS, "Default module `" << modName << "' added.");
			layoutModules_.push_back(modName);
		}
	}
	return true;
}


LayoutFile const * BufferParams::baseClass() const
{
	if (LayoutFileList::get().haveClass(pimpl_->baseClass_))
		return &(LayoutFileList::get()[pimpl_->baseClass_]);
	else 
		return 0;
}


LayoutFileIndex const & BufferParams::baseClassID() const
{
	return pimpl_->baseClass_;
}


void BufferParams::makeDocumentClass()
{
	if (!baseClass())
		return;

	doc_class_ = &(DocumentClassBundle::get().newClass(*baseClass()));

	// FIXME It might be worth loading the children's modules here,
	// just as we load their bibliographies and such, instead of just 
	// doing a check in InsetInclude.
	LayoutModuleList::const_iterator it = layoutModules_.begin();
	for (; it != layoutModules_.end(); it++) {
		string const modName = *it;
		LyXModule * lm = moduleList[modName];
		if (!lm) {
			docstring const msg =
				bformat(_("The module %1$s has been requested by\n"
					"this document but has not been found in the list of\n"
					"available modules. If you recently installed it, you\n"
					"probably need to reconfigure LyX.\n"), from_utf8(modName));
			frontend::Alert::warning(_("Module not available"),
					msg + _("Some layouts may not be available."));
			LYXERR0("BufferParams::makeDocumentClass(): Module " <<
					modName << " requested but not found in module list.");
			continue;
		}
		if (!lm->isAvailable()) {
			docstring const msg =
						bformat(_("The module %1$s requires a package that is\n"
						"not available in your LaTeX installation. LaTeX output\n"
						"may not be possible.\n"), from_utf8(modName));
			frontend::Alert::warning(_("Package not available"), msg);
		}
		FileName layout_file = libFileSearch("layouts", lm->getFilename());
		if (!doc_class_->read(layout_file, TextClass::MODULE)) {
			docstring const msg =
				bformat(_("Error reading module %1$s\n"), from_utf8(modName));
			frontend::Alert::warning(_("Read Error"), msg);
		}
	}
	if (!local_layout.empty()) {
		if (!doc_class_->read(local_layout, TextClass::MODULE)) {
			docstring const msg = _("Error reading internal layout information");
			frontend::Alert::warning(_("Read Error"), msg);
		}
	}
}


bool BufferParams::addLayoutModule(string const & modName) 
{
	LayoutModuleList::const_iterator it = layoutModules_.begin();
	LayoutModuleList::const_iterator end = layoutModules_.end();
	for (; it != end; it++)
		if (*it == modName) 
			return false;
	layoutModules_.push_back(modName);
	return true;
}


Font const BufferParams::getFont() const
{
	FontInfo f = documentClass().defaultfont();
	if (fontsDefaultFamily == "rmdefault")
		f.setFamily(ROMAN_FAMILY);
	else if (fontsDefaultFamily == "sfdefault")
		f.setFamily(SANS_FAMILY);
	else if (fontsDefaultFamily == "ttdefault")
		f.setFamily(TYPEWRITER_FAMILY);
	return Font(f, language);
}


void BufferParams::readPreamble(Lexer & lex)
{
	if (lex.getString() != "\\begin_preamble")
		lyxerr << "Error (BufferParams::readPreamble):"
			"consistency check failed." << endl;

	preamble = lex.getLongString("\\end_preamble");
}


void BufferParams::readLocalLayout(Lexer & lex)
{
	if (lex.getString() != "\\begin_local_layout")
		lyxerr << "Error (BufferParams::readLocalLayout):"
			"consistency check failed." << endl;

	local_layout = lex.getLongString("\\end_local_layout");
}


void BufferParams::readLanguage(Lexer & lex)
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


void BufferParams::readGraphicsDriver(Lexer & lex)
{
	if (!lex.next()) 
		return;

	string const tmptok = lex.getString();
	// check if tmptok is part of tex_graphics in tex_defs.h
	int n = 0;
	while (true) {
		string const test = tex_graphics[n++];

		if (test == tmptok) {
			graphicsDriver = tmptok;
			break;
		}
		if (test.empty()) {
			lex.printError(
				"Warning: graphics driver `$$Token' not recognized!\n"
				"         Setting graphics driver to `default'.\n");
			graphicsDriver = "default";
			break;
		}
	}
}


void BufferParams::readBullets(Lexer & lex)
{
	if (!lex.next()) 
		return;

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


void BufferParams::readBulletsLaTeX(Lexer & lex)
{
	// The bullet class should be able to read this.
	if (!lex.next()) 
		return;
	int const index = lex.getInteger();
	lex.next(true);
	docstring const temp_str = lex.getDocString();

	user_defined_bullet(index).setText(temp_str);
	temp_bullet(index).setText(temp_str);
}


void BufferParams::readModules(Lexer & lex)
{
	if (!lex.eatLine()) {
		lyxerr << "Error (BufferParams::readModules):"
				"Unexpected end of input." << endl;
		return;
	}
	while (true) {
		string mod = lex.getString();
		if (mod == "\\end_modules")
			break;
		addLayoutModule(mod);
		lex.eatLine();
	}
}


void BufferParams::readRemovedModules(Lexer & lex)
{
	if (!lex.eatLine()) {
		lyxerr << "Error (BufferParams::readRemovedModules):"
				"Unexpected end of input." << endl;
		return;
	}
	while (true) {
		string mod = lex.getString();
		if (mod == "\\end_removed_modules")
			break;
		removedModules_.insert(mod);
		lex.eatLine();
	}
	// now we want to remove any removed modules that were previously 
	// added. normally, that will be because default modules were added in 
	// setBaseClass(), which gets called when \textclass is read at the 
	// start of the read.
	set<string>::const_iterator rit = removedModules_.begin();
	set<string>::const_iterator const ren = removedModules_.end();
	for (; rit != ren; rit++) {
		LayoutModuleList::iterator const mit = layoutModules_.begin();
		LayoutModuleList::iterator const men = layoutModules_.end();
		LayoutModuleList::iterator found = find(mit, men, *rit);
		if (found == men)
			continue;
		layoutModules_.erase(found);
	}
}


string BufferParams::paperSizeName(PapersizePurpose purpose) const
{
	char real_papersize = papersize;
	if (real_papersize == PAPER_DEFAULT)
		real_papersize = lyxrc.default_papersize;

	switch (real_papersize) {
	case PAPER_DEFAULT:
		// could be anything, so don't guess
		return string();
	case PAPER_CUSTOM: {
		if (purpose == XDVI && !paperwidth.empty() &&
		    !paperheight.empty()) {
			// heightxwidth<unit>
			string first = paperwidth;
			string second = paperheight;
			if (orientation == ORIENTATION_LANDSCAPE)
				first.swap(second);
			// cut off unit.
			return first.erase(first.length() - 2)
				+ "x" + second;
		}
		return string();
	}
	case PAPER_A3:
		return "a3";
	case PAPER_A4:
		return "a4";
	case PAPER_A5:
		return "a5";
	case PAPER_B3:
		// dvips and dvipdfm do not know this
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "b3";
	case PAPER_B4:
		// dvipdfm does not know this
		if (purpose == DVIPDFM)
			return string();
		return "b4";
	case PAPER_B5:
		// dvipdfm does not know this
		if (purpose == DVIPDFM)
			return string();
		return "b5";
	case PAPER_USEXECUTIVE:
		// dvipdfm does not know this
		if (purpose == DVIPDFM)
			return string();
		return "foolscap";
	case PAPER_USLEGAL:
		return "legal";
	case PAPER_USLETTER:
	default:
		if (purpose == XDVI)
			return "us";
		return "letter";
	}
}


string const BufferParams::dvips_options() const
{
	string result;

	if (use_geometry
	    && papersize == PAPER_CUSTOM
	    && !lyxrc.print_paper_dimension_flag.empty()
	    && !paperwidth.empty()
	    && !paperheight.empty()) {
		// using a custom papersize
		result = lyxrc.print_paper_dimension_flag;
		result += ' ' + paperwidth;
		result += ',' + paperheight;
	} else {
		string const paper_option = paperSizeName(DVIPS);
		if (!paper_option.empty() && (paper_option != "letter" ||
		    orientation != ORIENTATION_LANDSCAPE)) {
			// dvips won't accept -t letter -t landscape.
			// In all other cases, include the paper size
			// explicitly.
			result = lyxrc.print_paper_flag;
			result += ' ' + paper_option;
		}
	}
	if (orientation == ORIENTATION_LANDSCAPE &&
	    papersize != PAPER_CUSTOM)
		result += ' ' + lyxrc.print_landscape_flag;
	return result;
}


string BufferParams::babelCall(string const & lang_opts) const
{
	string lang_pack = lyxrc.language_package;
	if (lang_pack != "\\usepackage{babel}")
		return lang_pack;
	// suppress the babel call when there is no babel language defined
	// for the document language in the lib/languages file and if no
	// other languages are used (lang_opts is then empty)
	if (lang_opts.empty())
		return string();
	// If Vietnamese is used, babel must directly be loaded with the
	// language options, see
	// http://www.mail-archive.com/lyx-devel@lists.lyx.org/msg129417.html
	size_t viet = lang_opts.find("vietnam");
	// viet = string::npos when not found
	// If Japanese is used, babel must directly be loaded with the
	// language options, see
	// http://bugzilla.lyx.org/show_bug.cgi?id=4597#c4
	size_t japan = lang_opts.find("japanese");
	// japan = string::npos when not found
	if (!lyxrc.language_global_options || viet != string::npos || japan != string::npos)
		return "\\usepackage[" + lang_opts + "]{babel}";
	return lang_pack;
}


void BufferParams::writeEncodingPreamble(odocstream & os,
		LaTeXFeatures & features, TexRow & texrow) const
{
	if (inputenc == "auto") {
		string const doc_encoding =
			language->encoding()->latexName();
		Encoding::Package const package =
			language->encoding()->package();

		// Create a list with all the input encodings used
		// in the document
		set<string> encodings =
			features.getEncodingSet(doc_encoding);

		// If the "japanese" package (i.e. pLaTeX) is used,
		// inputenc must be omitted.
		// see http://www.mail-archive.com/lyx-devel@lists.lyx.org/msg129680.html
		if (package == Encoding::japanese)
		     features.require("japanese");

		if ((!encodings.empty() || package == Encoding::inputenc)
		    && !features.isRequired("japanese")) {
			os << "\\usepackage[";
			set<string>::const_iterator it = encodings.begin();
			set<string>::const_iterator const end = encodings.end();
			if (it != end) {
				os << from_ascii(*it);
				++it;
			}
			for (; it != end; ++it)
				os << ',' << from_ascii(*it);
			if (package == Encoding::inputenc) {
				if (!encodings.empty())
					os << ',';
				os << from_ascii(doc_encoding);
			}
			os << "]{inputenc}\n";
			texrow.newline();
		}
		if (package == Encoding::CJK || features.mustProvide("CJK")) {
			os << "\\usepackage{CJK}\n";
			texrow.newline();
		}
	} else if (inputenc != "default") {
		switch (encoding().package()) {
		case Encoding::none:
		case Encoding::japanese:
			break;
		case Encoding::inputenc:
			// do not load inputenc if japanese is used
			if (features.isRequired("japanese"))
				break;
			os << "\\usepackage[" << from_ascii(inputenc)
			   << "]{inputenc}\n";
			texrow.newline();
			break;
		case Encoding::CJK:
			os << "\\usepackage{CJK}\n";
			texrow.newline();
			break;
		}
	}

	// The encoding "armscii8" (for Armenian) is only available when
	// the package "armtex" is loaded.
	if (language->encoding()->latexName() == "armscii8"
	    || inputenc == "armscii8") {
		os << "\\usepackage{armtex}\n";
		texrow.newline();
	}
}


string const BufferParams::loadFonts(string const & rm,
				     string const & sf, string const & tt,
				     bool const & sc, bool const & osf,
				     int const & sfscale, int const & ttscale) const
{
	/* The LaTeX font world is in a flux. In the PSNFSS font interface,
	   several packages have been replaced by others, that might not
	   be installed on every system. We have to take care for that
	   (see psnfss.pdf). We try to support all psnfss fonts as well
	   as the fonts that have become de facto standard in the LaTeX
	   world (e.g. Latin Modern). We do not support obsolete fonts
	   (like PSLatex). In general, it should be possible to mix any
	   rm font with any sf or tt font, respectively. (JSpitzm)
	   TODO:
		-- separate math fonts.
	*/

	if (rm == "default" && sf == "default" && tt == "default")
		//nothing to do
		return string();

	ostringstream os;

	// ROMAN FONTS
	// Computer Modern (must be explicitely selectable -- there might be classes
	// that define a different default font!
	if (rm == "cmr") {
		os << "\\renewcommand{\\rmdefault}{cmr}\n";
		// osf for Computer Modern needs eco.sty
		if (osf)
			os << "\\usepackage{eco}\n";
	}
	// Latin Modern Roman
	else if (rm == "lmodern")
		os << "\\usepackage{lmodern}\n";
	// AE
	else if (rm == "ae") {
		// not needed when using OT1 font encoding.
		if (lyxrc.fontenc != "default")
			os << "\\usepackage{ae,aecompl}\n";
	}
	// Times
	else if (rm == "times") {
		// try to load the best available package
		if (LaTeXFeatures::isAvailable("mathptmx"))
			os << "\\usepackage{mathptmx}\n";
		else if (LaTeXFeatures::isAvailable("mathptm"))
			os << "\\usepackage{mathptm}\n";
		else
			os << "\\usepackage{times}\n";
	}
	// Palatino
	else if (rm == "palatino") {
		// try to load the best available package
		if (LaTeXFeatures::isAvailable("mathpazo")) {
			os << "\\usepackage";
			if (osf || sc) {
				os << '[';
				if (!osf)
					os << "sc";
				else
					// "osf" includes "sc"!
					os << "osf";
				os << ']';
			}
			os << "{mathpazo}\n";
		}
		else if (LaTeXFeatures::isAvailable("mathpple"))
			os << "\\usepackage{mathpple}\n";
		else
			os << "\\usepackage{palatino}\n";
	}
	// Utopia
	else if (rm == "utopia") {
		// fourier supersedes utopia.sty, but does
		// not work with OT1 encoding.
		if (LaTeXFeatures::isAvailable("fourier")
		    && lyxrc.fontenc != "default") {
			os << "\\usepackage";
			if (osf || sc) {
				os << '[';
				if (sc)
					os << "expert";
				if (osf && sc)
					os << ',';
				if (osf)
					os << "oldstyle";
				os << ']';
			}
			os << "{fourier}\n";
		}
		else
			os << "\\usepackage{utopia}\n";
	}
	// Bera (complete fontset)
	else if (rm == "bera" && sf == "default" && tt == "default")
		os << "\\usepackage{bera}\n";
	// everything else
	else if (rm != "default")
		os << "\\usepackage" << "{" << rm << "}\n";

	// SANS SERIF
	// Helvetica, Bera Sans
	if (sf == "helvet" || sf == "berasans") {
		if (sfscale != 100)
			os << "\\usepackage[scaled=" << float(sfscale) / 100
			   << "]{" << sf << "}\n";
		else
			os << "\\usepackage{" << sf << "}\n";
	}
	// Avant Garde
	else if (sf == "avant")
		os << "\\usepackage{" << sf << "}\n";
	// Computer Modern, Latin Modern, CM Bright
	else if (sf != "default")
		os << "\\renewcommand{\\sfdefault}{" << sf << "}\n";

	// monospaced/typewriter
	// Courier, LuxiMono
	if (tt == "luximono" || tt == "beramono") {
		if (ttscale != 100)
			os << "\\usepackage[scaled=" << float(ttscale) / 100
			   << "]{" << tt << "}\n";
		else
			os << "\\usepackage{" << tt << "}\n";
	}
	// Courier
	else if (tt == "courier" )
		os << "\\usepackage{" << tt << "}\n";
	// Computer Modern, Latin Modern, CM Bright
	else if (tt != "default")
		os << "\\renewcommand{\\ttdefault}{" << tt << "}\n";

	return os.str();
}


Encoding const & BufferParams::encoding() const
{
	if (inputenc == "auto" || inputenc == "default")
		return *language->encoding();
	Encoding const * const enc = encodings.fromLaTeXName(inputenc);
	if (enc)
		return *enc;
	LYXERR0("Unknown inputenc value `" << inputenc
	       << "'. Using `auto' instead.");
	return *language->encoding();
}


CiteEngine BufferParams::citeEngine() const
{
	// FIXME the class should provide the numerical/
	// authoryear choice
	if (documentClass().provides("natbib")
	    && cite_engine_ != ENGINE_NATBIB_NUMERICAL)
		return ENGINE_NATBIB_AUTHORYEAR;
	return cite_engine_;
}


void BufferParams::setCiteEngine(CiteEngine cite_engine)
{
	cite_engine_ = cite_engine;
}

} // namespace lyx
