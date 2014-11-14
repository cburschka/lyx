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
#include "ColorSet.h"
#include "Converter.h"
#include "Encoding.h"
#include "HSpace.h"
#include "IndicesList.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "LaTeXFonts.h"
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
#include "support/mutex.h"
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
	"a0paper", "a1paper", "a2paper", "a3paper",	"a4paper", "a5paper",
	"a6paper", "b0paper", "b1paper", "b2paper","b3paper", "b4paper",
	"b5paper", "b6paper", "c0paper", "c1paper", "c2paper", "c3paper",
	"c4paper", "c5paper", "c6paper", "b0j", "b1j", "b2j", "b3j", "b4j", "b5j",
	"b6j", ""
};


static char const * const string_orientation[] = {
	"portrait", "landscape", ""
};


static char const * const tex_graphics[] = {
	"default", "dvialw", "dvilaser", "dvipdf", "dvipdfm", "dvipdfmx",
	"dvips", "dvipsone", "dvitops", "dviwin", "dviwindo", "dvi2ps", "emtex",
	"ln", "oztex", "pctexhp", "pctexps", "pctexwin", "pctex32", "pdftex",
	"psprint", "pubps", "tcidvi", "textures", "truetex", "vtex", "xdvi",
	"xetex", "none", ""
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
	static ParSepTranslator const translator =
		init_parseptranslator();
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
	static QuotesLangTranslator const translator =
		init_quoteslangtranslator();
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
	translator.addPair(string_papersize[5], PAPER_A0);
	translator.addPair(string_papersize[6], PAPER_A1);
	translator.addPair(string_papersize[7], PAPER_A2);
	translator.addPair(string_papersize[8], PAPER_A3);
	translator.addPair(string_papersize[9], PAPER_A4);
	translator.addPair(string_papersize[10], PAPER_A5);
	translator.addPair(string_papersize[11], PAPER_A6);
	translator.addPair(string_papersize[12], PAPER_B0);
	translator.addPair(string_papersize[13], PAPER_B1);
	translator.addPair(string_papersize[14], PAPER_B2);
	translator.addPair(string_papersize[15], PAPER_B3);
	translator.addPair(string_papersize[16], PAPER_B4);
	translator.addPair(string_papersize[17], PAPER_B5);
	translator.addPair(string_papersize[18], PAPER_B6);
	translator.addPair(string_papersize[19], PAPER_C0);
	translator.addPair(string_papersize[20], PAPER_C1);
	translator.addPair(string_papersize[21], PAPER_C2);
	translator.addPair(string_papersize[22], PAPER_C3);
	translator.addPair(string_papersize[23], PAPER_C4);
	translator.addPair(string_papersize[24], PAPER_C5);
	translator.addPair(string_papersize[25], PAPER_C6);
	translator.addPair(string_papersize[26], PAPER_JISB0);
	translator.addPair(string_papersize[27], PAPER_JISB1);
	translator.addPair(string_papersize[28], PAPER_JISB2);
	translator.addPair(string_papersize[29], PAPER_JISB3);
	translator.addPair(string_papersize[30], PAPER_JISB4);
	translator.addPair(string_papersize[31], PAPER_JISB5);
	translator.addPair(string_papersize[32], PAPER_JISB6);
	return translator;
}


PaperSizeTranslator const & papersizetranslator()
{
	static PaperSizeTranslator const translator =
		initPaperSizeTranslator();
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
	static PaperOrientationTranslator const translator =
	    init_paperorientationtranslator();
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
	static SidesTranslator const translator = init_sidestranslator();
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
	static PackageTranslator const translator =
		init_packagetranslator();
	return translator;
}


// Cite engine
typedef Translator<string, CiteEngineType> CiteEngineTypeTranslator;


CiteEngineTypeTranslator const init_citeenginetypetranslator()
{
	CiteEngineTypeTranslator translator("authoryear", ENGINE_TYPE_AUTHORYEAR);
	translator.addPair("numerical", ENGINE_TYPE_NUMERICAL);
	translator.addPair("default", ENGINE_TYPE_DEFAULT);
	return translator;
}


CiteEngineTypeTranslator const & citeenginetypetranslator()
{
	static CiteEngineTypeTranslator const translator =
		init_citeenginetypetranslator();
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
	static SpaceTranslator const translator = init_spacetranslator();
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
	IndicesList indiceslist;
	Spacing spacing;
	/** This is the amount of space used for paragraph_separation "skip",
	 * and for detached paragraphs in "indented" documents.
	 */
	HSpace indentation;
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
	LBUFERR(ptr);
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
	cite_engine_.push_back("basic");
	cite_engine_type_ = ENGINE_TYPE_DEFAULT;
	biblio_style = "plain";
	use_bibtopic = false;
	use_indices = false;
	track_changes = false;
	output_changes = false;
	use_default_options = true;
	maintain_unincluded_children = false;
	secnumdepth = 3;
	tocdepth = 3;
	language = default_language;
	fontenc = "global";
	fonts_roman = "default";
	fonts_sans = "default";
	fonts_typewriter = "default";
	fonts_math = "auto";
	fonts_default_family = "default";
	useNonTeXFonts = false;
	fonts_expert_sc = false;
	fonts_old_figures = false;
	fonts_sans_scale = 100;
	fonts_typewriter_scale = 100;
	inputenc = "auto";
	lang_package = "default";
	graphics_driver = "default";
	default_output_format = "default";
	bibtex_command = "default";
	index_command = "default";
	sides = OneSide;
	columns = 1;
	listings_params = string();
	pagestyle = "default";
	suppress_date = false;
	justification = true;
	// no color is the default (white)
	backgroundcolor = lyx::rgbFromHexName("#ffffff");
	isbackgroundcolor = false;
	// no color is the default (black)
	fontcolor = lyx::rgbFromHexName("#000000");
	isfontcolor = false;
	// light gray is the default font color for greyed-out notes
	notefontcolor = lyx::rgbFromHexName("#cccccc");
	boxbgcolor = lyx::rgbFromHexName("#ff0000");
	compressed = lyxrc.save_compressed;
	for (int iter = 0; iter < 4; ++iter) {
		user_defined_bullet(iter) = ITEMIZE_DEFAULTS[iter];
		temp_bullet(iter) = ITEMIZE_DEFAULTS[iter];
	}
	// default index
	indiceslist().addDefault(B_("Index"));
	html_be_strict = false;
	html_math_output = MathML;
	html_math_img_scale = 1.0;
	html_css_as_file = false;
	display_pixel_ratio = 1.0;

	output_sync = false;
	use_refstyle = true;
}


docstring BufferParams::B_(string const & l10n) const
{
	LASSERT(language, return from_utf8(l10n));
	return getMessages(language->code()).get(l10n);
}


BufferParams::Package BufferParams::use_package(std::string const & p) const
{
	PackageMap::const_iterator it = use_packages.find(p);
	if (it == use_packages.end())
		return package_auto;
	return it->second;
}


void BufferParams::use_package(std::string const & p, BufferParams::Package u)
{
	use_packages[p] = u;
}


map<string, string> const & BufferParams::auto_packages()
{
	static map<string, string> packages;
	if (packages.empty()) {
		// We could have a race condition here that two threads
		// discover an empty map at the same time and want to fill
		// it, but that is no problem, since the same contents is
		// filled in twice then. Having the locker inside the
		// packages.empty() condition has the advantage that we
		// don't need the mutex overhead for simple reading.
		static Mutex mutex;
		Mutex::Locker locker(&mutex);
		// adding a package here implies a file format change!
		packages["amsmath"] =
			N_("The LaTeX package amsmath is only used if AMS formula types or symbols from the AMS math toolbars are inserted into formulas");
		packages["amssymb"] =
			N_("The LaTeX package amssymb is only used if symbols from the AMS math toolbars are inserted into formulas");
		packages["cancel"] =
			N_("The LaTeX package cancel is only used if \\cancel commands are used in formulas");
		packages["esint"] =
			N_("The LaTeX package esint is only used if special integral symbols are inserted into formulas");
		packages["mathdots"] =
			N_("The LaTeX package mathdots is only used if the command \\iddots is inserted into formulas");
		packages["mathtools"] =
			N_("The LaTeX package mathtools is only used if some mathematical relations are inserted into formulas");
		packages["mhchem"] =
			N_("The LaTeX package mhchem is only used if either the command \\ce or \\cf is inserted into formulas");
		packages["stackrel"] =
			N_("The LaTeX package stackrel is only used if the command \\stackrel with subscript is inserted into formulas");
		packages["stmaryrd"] =
			N_("The LaTeX package stmaryrd is only used if symbols from the St Mary's Road symbol font for theoretical computer science are inserted into formulas");
		packages["undertilde"] =
			N_("The LaTeX package undertilde is only used if you use the math frame decoration 'utilde'");
	}
	return packages;
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


IndicesList & BufferParams::indiceslist()
{
	return pimpl_->indiceslist;
}


IndicesList const & BufferParams::indiceslist() const
{
	return pimpl_->indiceslist;
}


Bullet & BufferParams::temp_bullet(lyx::size_type const index)
{
	LASSERT(index < 4, return pimpl_->temp_bullets[0]);
	return pimpl_->temp_bullets[index];
}


Bullet const & BufferParams::temp_bullet(lyx::size_type const index) const
{
	LASSERT(index < 4, return pimpl_->temp_bullets[0]);
	return pimpl_->temp_bullets[index];
}


Bullet & BufferParams::user_defined_bullet(lyx::size_type const index)
{
	LASSERT(index < 4, return pimpl_->temp_bullets[0]);
	return pimpl_->user_defined_bullets[index];
}


Bullet const & BufferParams::user_defined_bullet(lyx::size_type const index) const
{
	LASSERT(index < 4, return pimpl_->temp_bullets[0]);
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


HSpace const & BufferParams::getIndentation() const
{
	return pimpl_->indentation;
}


void BufferParams::setIndentation(HSpace const & indent)
{
	pimpl_->indentation = indent;
}


VSpace const & BufferParams::getDefSkip() const
{
	return pimpl_->defskip;
}


void BufferParams::setDefSkip(VSpace const & vs)
{
	// DEFSKIP will cause an infinite loop
	LASSERT(vs.kind() != VSpace::DEFSKIP, return);
	pimpl_->defskip = vs;
}


string BufferParams::readToken(Lexer & lex, string const & token,
	FileName const & filepath)
{
	if (token == "\\textclass") {
		lex.next();
		string const classname = lex.getString();
		// if there exists a local layout file, ignore the system one
		// NOTE: in this case, the textclass (.cls file) is assumed to
		// be available.
		string tcp;
		LayoutFileList & bcl = LayoutFileList::get();
		if (!filepath.empty())
			tcp = bcl.addLocalLayout(classname, filepath.absFileName());
		// that returns non-empty if a "local" layout file is found.
		if (!tcp.empty())
			setBaseClass(tcp);
		else
			setBaseClass(classname);
		// We assume that a tex class exists for local or unknown
		// layouts so this warning, will only be given for system layouts.
		if (!baseClass()->isTeXClassAvailable()) {
			docstring const desc =
				translateIfPossible(from_utf8(baseClass()->description()));
			docstring const prereqs =
				from_utf8(baseClass()->prerequisites());
			docstring const msg =
				bformat(_("The selected document class\n"
						 "\t%1$s\n"
						 "requires external files that are not available.\n"
						 "The document class can still be used, but the\n"
						 "document cannot be compiled until the following\n"
						 "prerequisites are installed:\n"
						 "\t%2$s\n"
						 "See section 3.1.2.2 (Class Availability) of the\n"
						 "User's Guide for more information."), desc, prereqs);
			frontend::Alert::warning(_("Document class not available"),
				       msg);
		}
	} else if (token == "\\begin_preamble") {
		readPreamble(lex);
	} else if (token == "\\begin_local_layout") {
		readLocalLayout(lex, false);
	} else if (token == "\\begin_forced_local_layout") {
		readLocalLayout(lex, true);
	} else if (token == "\\begin_modules") {
		readModules(lex);
	} else if (token == "\\begin_removed_modules") {
		readRemovedModules(lex);
	} else if (token == "\\begin_includeonly") {
		readIncludeonly(lex);
	} else if (token == "\\maintain_unincluded_children") {
		lex >> maintain_unincluded_children;
	} else if (token == "\\options") {
		lex.eatLine();
		options = lex.getString();
	} else if (token == "\\use_default_options") {
		lex >> use_default_options;
	} else if (token == "\\master") {
		lex.eatLine();
		master = lex.getString();
	} else if (token == "\\suppress_date") {
		lex >> suppress_date;
	} else if (token == "\\justification") {
		lex >> justification;
	} else if (token == "\\language") {
		readLanguage(lex);
	} else if (token == "\\language_package") {
		lex.eatLine();
		lang_package = lex.getString();
	} else if (token == "\\inputencoding") {
		lex >> inputenc;
	} else if (token == "\\graphics") {
		readGraphicsDriver(lex);
	} else if (token == "\\default_output_format") {
		lex >> default_output_format;
	} else if (token == "\\bibtex_command") {
		lex.eatLine();
		bibtex_command = lex.getString();
	} else if (token == "\\index_command") {
		lex.eatLine();
		index_command = lex.getString();
	} else if (token == "\\fontencoding") {
		lex.eatLine();
		fontenc = lex.getString();
	} else if (token == "\\font_roman") {
		lex.eatLine();
		fonts_roman = lex.getString();
	} else if (token == "\\font_sans") {
		lex.eatLine();
		fonts_sans = lex.getString();
	} else if (token == "\\font_typewriter") {
		lex.eatLine();
		fonts_typewriter = lex.getString();
	} else if (token == "\\font_math") {
		lex.eatLine();
		fonts_math = lex.getString();
	} else if (token == "\\font_default_family") {
		lex >> fonts_default_family;
	} else if (token == "\\use_non_tex_fonts") {
		lex >> useNonTeXFonts;
	} else if (token == "\\font_sc") {
		lex >> fonts_expert_sc;
	} else if (token == "\\font_osf") {
		lex >> fonts_old_figures;
	} else if (token == "\\font_sf_scale") {
		lex >> fonts_sans_scale;
	} else if (token == "\\font_tt_scale") {
		lex >> fonts_typewriter_scale;
	} else if (token == "\\font_cjk") {
		lex >> fonts_cjk;
	} else if (token == "\\paragraph_separation") {
		string parsep;
		lex >> parsep;
		paragraph_separation = parseptranslator().find(parsep);
	} else if (token == "\\paragraph_indentation") {
		lex.next();
		string indentation = lex.getString();
		pimpl_->indentation = HSpace(indentation);
	} else if (token == "\\defskip") {
		lex.next();
		string const defskip = lex.getString();
		pimpl_->defskip = VSpace(defskip);
		if (pimpl_->defskip.kind() == VSpace::DEFSKIP)
			// that is invalid
			pimpl_->defskip = VSpace(VSpace::MEDSKIP);
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
	} else if (token == "\\use_package") {
		string package;
		int use;
		lex >> package;
		lex >> use;
		use_package(package, packagetranslator().find(use));
	} else if (token == "\\cite_engine") {
		lex.eatLine();
		vector<string> engine = getVectorFromString(lex.getString());
		setCiteEngine(engine);
	} else if (token == "\\cite_engine_type") {
		string engine_type;
		lex >> engine_type;
		cite_engine_type_ = citeenginetypetranslator().find(engine_type);
	} else if (token == "\\biblio_style") {
		lex.eatLine();
		biblio_style = lex.getString();
	} else if (token == "\\use_bibtopic") {
		lex >> use_bibtopic;
	} else if (token == "\\use_indices") {
		lex >> use_indices;
	} else if (token == "\\tracking_changes") {
		lex >> track_changes;
	} else if (token == "\\output_changes") {
		lex >> output_changes;
	} else if (token == "\\branch") {
		lex.eatLine();
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
			if (tok == "\\filename_suffix") {
				lex.next();
				if (branch_ptr)
					branch_ptr->setFileNameSuffix(lex.getInteger());
			}
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
	} else if (token == "\\index") {
		lex.eatLine();
		docstring index = lex.getDocString();
		docstring shortcut;
		indiceslist().add(index);
		while (true) {
			lex.next();
			string const tok = lex.getString();
			if (tok == "\\end_index")
				break;
			Index * index_ptr = indiceslist().find(index);
			if (tok == "\\shortcut") {
				lex.next();
				shortcut = lex.getDocString();
				if (index_ptr)
					index_ptr->setShortcut(shortcut);
			}
			if (tok == "\\color") {
				lex.eatLine();
				string color = lex.getString();
				if (index_ptr)
					index_ptr->setColor(color);
				// Update also the Color table:
				if (color == "none")
					color = lcolor.getX11Name(Color_background);
				// FIXME UNICODE
				if (!shortcut.empty())
					lcolor.setColor(to_utf8(shortcut), color);
			}
		}
	} else if (token == "\\author") {
		lex.eatLine();
		istringstream ss(lex.getString());
		Author a;
		ss >> a;
		author_map[a.bufferId()] = pimpl_->authorlist.record(a);
	} else if (token == "\\paperorientation") {
		string orient;
		lex >> orient;
		orientation = paperorientationtranslator().find(orient);
	} else if (token == "\\backgroundcolor") {
		lex.eatLine();
		backgroundcolor = lyx::rgbFromHexName(lex.getString());
		isbackgroundcolor = true;
	} else if (token == "\\fontcolor") {
		lex.eatLine();
		fontcolor = lyx::rgbFromHexName(lex.getString());
		isfontcolor = true;
	} else if (token == "\\notefontcolor") {
		lex.eatLine();
		string color = lex.getString();
		notefontcolor = lyx::rgbFromHexName(color);
		lcolor.setColor("notefontcolor", color);
	} else if (token == "\\boxbgcolor") {
		lex.eatLine();
		string color = lex.getString();
		boxbgcolor = lyx::rgbFromHexName(color);
		lcolor.setColor("boxbgcolor", color);
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
	} else if (token == "\\html_math_output") {
		int temp;
		lex >> temp;
		html_math_output = static_cast<MathOutput>(temp);
	} else if (token == "\\html_be_strict") {
		lex >> html_be_strict;
	} else if (token == "\\html_css_as_file") {
		lex >> html_css_as_file;
	} else if (token == "\\html_math_img_scale") {
		lex >> html_math_img_scale;
	} else if (token == "\\html_latex_start") {
		lex.eatLine();
		html_latex_start = lex.getString();
	} else if (token == "\\html_latex_end") {
		lex.eatLine();
		html_latex_end = lex.getString();
	} else if (token == "\\output_sync") {
		lex >> output_sync;
	} else if (token == "\\output_sync_macro") {
		lex >> output_sync_macro;
	} else if (token == "\\use_refstyle") {
		lex >> use_refstyle;
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

	// use the class options defined in the layout?
	os << "\\use_default_options "
	   << convert<string>(use_default_options) << "\n";

	// the master document
	if (!master.empty()) {
		os << "\\master " << master << '\n';
	}

	// removed modules
	if (!removed_modules_.empty()) {
		os << "\\begin_removed_modules" << '\n';
		list<string>::const_iterator it = removed_modules_.begin();
		list<string>::const_iterator en = removed_modules_.end();
		for (; it != en; ++it)
			os << *it << '\n';
		os << "\\end_removed_modules" << '\n';
	}

	// the modules
	if (!layout_modules_.empty()) {
		os << "\\begin_modules" << '\n';
		LayoutModuleList::const_iterator it = layout_modules_.begin();
		LayoutModuleList::const_iterator en = layout_modules_.end();
		for (; it != en; ++it)
			os << *it << '\n';
		os << "\\end_modules" << '\n';
	}

	// includeonly
	if (!included_children_.empty()) {
		os << "\\begin_includeonly" << '\n';
		list<string>::const_iterator it = included_children_.begin();
		list<string>::const_iterator en = included_children_.end();
		for (; it != en; ++it)
			os << *it << '\n';
		os << "\\end_includeonly" << '\n';
	}
	os << "\\maintain_unincluded_children "
	   << convert<string>(maintain_unincluded_children) << '\n';

	// local layout information
	string const local_layout = getLocalLayout(false);
	if (!local_layout.empty()) {
		// remove '\n' from the end
		string const tmplocal = rtrim(local_layout, "\n");
		os << "\\begin_local_layout\n"
		   << tmplocal
		   << "\n\\end_local_layout\n";
	}
	string const forced_local_layout = getLocalLayout(true);
	if (!forced_local_layout.empty()) {
		// remove '\n' from the end
		string const tmplocal = rtrim(forced_local_layout, "\n");
		os << "\\begin_forced_local_layout\n"
		   << tmplocal
		   << "\n\\end_forced_local_layout\n";
	}

	// then the text parameters
	if (language != ignore_language)
		os << "\\language " << language->lang() << '\n';
	os << "\\language_package " << lang_package
	   << "\n\\inputencoding " << inputenc
	   << "\n\\fontencoding " << fontenc
	   << "\n\\font_roman " << fonts_roman
	   << "\n\\font_sans " << fonts_sans
	   << "\n\\font_typewriter " << fonts_typewriter
	   << "\n\\font_math " << fonts_math
	   << "\n\\font_default_family " << fonts_default_family
	   << "\n\\use_non_tex_fonts " << convert<string>(useNonTeXFonts)
	   << "\n\\font_sc " << convert<string>(fonts_expert_sc)
	   << "\n\\font_osf " << convert<string>(fonts_old_figures)
	   << "\n\\font_sf_scale " << fonts_sans_scale
	   << "\n\\font_tt_scale " << fonts_typewriter_scale
	   << '\n';
	if (!fonts_cjk.empty()) {
		os << "\\font_cjk " << fonts_cjk << '\n';
	}
	os << "\\graphics " << graphics_driver << '\n';
	os << "\\default_output_format " << default_output_format << '\n';
	os << "\\output_sync " << output_sync << '\n';
	if (!output_sync_macro.empty())
		os << "\\output_sync_macro \"" << output_sync_macro << "\"\n";
	os << "\\bibtex_command " << bibtex_command << '\n';
	os << "\\index_command " << index_command << '\n';

	if (!float_placement.empty()) {
		os << "\\float_placement " << float_placement << '\n';
	}
	os << "\\paperfontsize " << fontsize << '\n';

	spacing().writeFile(os);
	pdfoptions().writeFile(os);

	os << "\\papersize " << string_papersize[papersize]
	   << "\n\\use_geometry " << convert<string>(use_geometry);
	map<string, string> const & packages = auto_packages();
	for (map<string, string>::const_iterator it = packages.begin();
	     it != packages.end(); ++it)
		os << "\n\\use_package " << it->first << ' '
		   << use_package(it->first);

	os << "\n\\cite_engine ";

	if (!cite_engine_.empty()) {
		LayoutModuleList::const_iterator be = cite_engine_.begin();
		LayoutModuleList::const_iterator en = cite_engine_.end();
		for (LayoutModuleList::const_iterator it = be; it != en; ++it) {
			if (it != be)
				os << ',';
			os << *it;
		}
	} else {
		os << "basic";
	}

	os << "\n\\cite_engine_type " << citeenginetypetranslator().find(cite_engine_type_)
	   << "\n\\biblio_style " << biblio_style
	   << "\n\\use_bibtopic " << convert<string>(use_bibtopic)
	   << "\n\\use_indices " << convert<string>(use_indices)
	   << "\n\\paperorientation " << string_orientation[orientation]
	   << "\n\\suppress_date " << convert<string>(suppress_date)
	   << "\n\\justification " << convert<string>(justification)
	   << "\n\\use_refstyle " << use_refstyle
	   << '\n';
	if (isbackgroundcolor == true)
		os << "\\backgroundcolor " << lyx::X11hexname(backgroundcolor) << '\n';
	if (isfontcolor == true)
		os << "\\fontcolor " << lyx::X11hexname(fontcolor) << '\n';
	if (notefontcolor != lyx::rgbFromHexName("#cccccc"))
		os << "\\notefontcolor " << lyx::X11hexname(notefontcolor) << '\n';
	if (boxbgcolor != lyx::rgbFromHexName("#ff0000"))
		os << "\\boxbgcolor " << lyx::X11hexname(boxbgcolor) << '\n';

	BranchList::const_iterator it = branchlist().begin();
	BranchList::const_iterator end = branchlist().end();
	for (; it != end; ++it) {
		os << "\\branch " << to_utf8(it->branch())
		   << "\n\\selected " << it->isSelected()
		   << "\n\\filename_suffix " << it->hasFileNameSuffix()
		   << "\n\\color " << lyx::X11hexname(it->color())
		   << "\n\\end_branch"
		   << "\n";
	}

	IndicesList::const_iterator iit = indiceslist().begin();
	IndicesList::const_iterator iend = indiceslist().end();
	for (; iit != iend; ++iit) {
		os << "\\index " << to_utf8(iit->index())
		   << "\n\\shortcut " << to_utf8(iit->shortcut())
		   << "\n\\color " << lyx::X11hexname(iit->color())
		   << "\n\\end_index"
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
	   << string_paragraph_separation[paragraph_separation];
	if (!paragraph_separation)
		os << "\n\\paragraph_indentation " << getIndentation().asLyXCommand();
	else
		os << "\n\\defskip " << getDefSkip().asLyXCommand();
	os << "\n\\quotes_language "
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

	os << "\\tracking_changes " << convert<string>(track_changes) << '\n'
	   << "\\output_changes " << convert<string>(output_changes) << '\n'
	   << "\\html_math_output " << html_math_output << '\n'
	   << "\\html_css_as_file " << html_css_as_file << '\n'
	   << "\\html_be_strict " << convert<string>(html_be_strict) << '\n';

	if (html_math_img_scale != 1.0)
		os << "\\html_math_img_scale " << convert<string>(html_math_img_scale) << '\n';
	if (!html_latex_start.empty())
		os << "\\html_latex_start " << html_latex_start << '\n';
	if (!html_latex_end.empty())
		 os << "\\html_latex_end " << html_latex_end << '\n';

	os << pimpl_->authorlist;
}


void BufferParams::validate(LaTeXFeatures & features) const
{
	features.require(documentClass().requires());

	if (columns > 1 && language->rightToLeft())
		features.require("rtloutputdblcol");

	if (output_changes) {
		bool dvipost    = LaTeXFeatures::isAvailable("dvipost");
		bool xcolorulem = LaTeXFeatures::isAvailable("ulem") &&
				  LaTeXFeatures::isAvailable("xcolor");

		switch (features.runparams().flavor) {
		case OutputParams::LATEX:
		case OutputParams::DVILUATEX:
			if (dvipost) {
				features.require("ct-dvipost");
				features.require("dvipost");
			} else if (xcolorulem) {
				features.require("ct-xcolor-ulem");
				features.require("ulem");
				features.require("xcolor");
			} else {
				features.require("ct-none");
			}
			break;
		case OutputParams::LUATEX:
		case OutputParams::PDFLATEX:
		case OutputParams::XETEX:
			if (xcolorulem) {
				features.require("ct-xcolor-ulem");
				features.require("ulem");
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

	for (PackageMap::const_iterator it = use_packages.begin();
	     it != use_packages.end(); ++it) {
		if (it->first == "amsmath") {
			// AMS Style is at document level
			if (it->second == package_on ||
			    features.isProvided("amsmath"))
				features.require(it->first);
		} else if (it->second == package_on)
			features.require(it->first);
	}

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

	if (pdfoptions().use_hyperref) {
		features.require("hyperref");
		// due to interferences with babel and hyperref, the color package has to
		// be loaded after hyperref when hyperref is used with the colorlinks
		// option, see http://www.lyx.org/trac/ticket/5291
		if (pdfoptions().colorlinks)
			features.require("color");
	}
	if (!listings_params.empty()) {
		// do not test validity because listings_params is
		// supposed to be valid
		string par =
			InsetListingsParams(listings_params).separatedParams(true);
		// we can't support all packages, but we should load the color package
		if (par.find("\\color", 0) != string::npos)
			features.require("color");
	}

	// some languages are only available via polyglossia
	if (features.runparams().flavor == OutputParams::XETEX
	    && (features.hasPolyglossiaExclusiveLanguages()
		|| useNonTeXFonts))
		features.require("polyglossia");

	if (useNonTeXFonts && fonts_math != "auto")
		features.require("unicode-math");

	if (!language->requires().empty())
		features.require(language->requires());
}


bool BufferParams::writeLaTeX(otexstream & os, LaTeXFeatures & features,
			      FileName const & filepath) const
{
	// http://www.tug.org/texmf-dist/doc/latex/base/fixltx2e.pdf
	// !! To use the Fix-cm package, load it before \documentclass, and use the command
	// \RequirePackage to do so, rather than the normal \usepackage
	// Do not try to load any other package before the document class, unless you
	// have a thorough understanding of the LATEX internals and know exactly what you
	// are doing!
	if (features.mustProvide("fix-cm"))
		os << "\\RequirePackage{fix-cm}\n";

	os << "\\documentclass";

	DocumentClass const & tclass = documentClass();

	ostringstream clsoptions; // the document class options.

	if (tokenPos(tclass.opt_fontsize(),
		     '|', fontsize) >= 0) {
		// only write if existing in list (and not default)
		clsoptions << fontsize << "pt,";
	}

	// all paper sizes except of A4, A5, B5 and the US sizes need the
	// geometry package
	bool nonstandard_papersize = papersize != PAPER_DEFAULT
		&& papersize != PAPER_USLETTER
		&& papersize != PAPER_USLEGAL
		&& papersize != PAPER_USEXECUTIVE
		&& papersize != PAPER_A4
		&& papersize != PAPER_A5
		&& papersize != PAPER_B5;

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
		case PAPER_A0:
		case PAPER_A1:
		case PAPER_A2:
		case PAPER_A3:
		case PAPER_A6:
		case PAPER_B0:
		case PAPER_B1:
		case PAPER_B2:
		case PAPER_B3:
		case PAPER_B4:
		case PAPER_B6:
		case PAPER_C0:
		case PAPER_C1:
		case PAPER_C2:
		case PAPER_C3:
		case PAPER_C4:
		case PAPER_C5:
		case PAPER_C6:
		case PAPER_JISB0:
		case PAPER_JISB1:
		case PAPER_JISB2:
		case PAPER_JISB3:
		case PAPER_JISB4:
		case PAPER_JISB5:
		case PAPER_JISB6:
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
	bool const use_babel = features.useBabel() && !features.isProvided("babel");
	bool const use_polyglossia = features.usePolyglossia();
	bool const global = lyxrc.language_global_options;
	if (use_babel || (use_polyglossia && global)) {
		language_options << features.getBabelLanguages();
		if (!language->babel().empty()) {
			if (!language_options.str().empty())
				language_options << ',';
			language_options << language->babel();
		}
		if (global && !features.needBabelLangOptions()
		    && !language_options.str().empty())
			clsoptions << language_options.str() << ',';
	}

	// the predefined options from the layout
	if (use_default_options && !tclass.options().empty())
		clsoptions << tclass.options() << ',';

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
	// end of \documentclass defs

	// if we use fontspec or newtxmath, we have to load the AMS packages here
	string const ams = features.loadAMSPackages();
	bool const ot1 = (font_encoding() == "default" || font_encoding() == "OT1");
	bool const use_newtxmath =
		theLaTeXFonts().getLaTeXFont(from_ascii(fonts_math)).getUsedPackage(
			ot1, false, false) == "newtxmath";
	if ((useNonTeXFonts || use_newtxmath) && !ams.empty())
		os << from_ascii(ams);

	if (useNonTeXFonts) {
		os << "\\usepackage{fontspec}\n";
		if (features.mustProvide("unicode-math")
		    && features.isAvailable("unicode-math"))
			os << "\\usepackage{unicode-math}\n";
	}

	// font selection must be done before loading fontenc.sty
	string const fonts = loadFonts(features);
	if (!fonts.empty())
		os << from_utf8(fonts);

	if (fonts_default_family != "default")
		os << "\\renewcommand{\\familydefault}{\\"
		   << from_ascii(fonts_default_family) << "}\n";

	// set font encoding
	// for arabic_arabi and farsi we also need to load the LAE and
	// LFE encoding
	// XeTeX and LuaTeX (with OS fonts) work without fontenc
	if (font_encoding() != "default" && language->lang() != "japanese"
	    && !useNonTeXFonts && !features.isProvided("fontenc")) {
		docstring extra_encoding;
		if (features.mustProvide("textgreek"))
			extra_encoding += from_ascii("LGR");
		if (features.mustProvide("textcyr")) {
			if (!extra_encoding.empty())
				extra_encoding.push_back(',');
			extra_encoding += from_ascii("T2A");
		}
		if (!extra_encoding.empty() && !font_encoding().empty())
			extra_encoding.push_back(',');
		size_t fars = language_options.str().find("farsi");
		size_t arab = language_options.str().find("arabic");
		if (language->lang() == "arabic_arabi"
			|| language->lang() == "farsi" || fars != string::npos
			|| arab != string::npos) {
			os << "\\usepackage[" << extra_encoding
			   << from_ascii(font_encoding())
			   << ",LFE,LAE]{fontenc}\n";
		} else {
			os << "\\usepackage[" << extra_encoding
			   << from_ascii(font_encoding())
			   << "]{fontenc}\n";
		}
	}

	// handle inputenc etc.
	writeEncodingPreamble(os, features);

	// includeonly
	if (!features.runparams().includeall && !included_children_.empty()) {
		os << "\\includeonly{";
		list<string>::const_iterator it = included_children_.begin();
		list<string>::const_iterator en = included_children_.end();
		bool first = true;
		for (; it != en; ++it) {
			string incfile = *it;
			FileName inc = makeAbsPath(incfile, filepath.absFileName());
			string mangled = DocFileName(changeExtension(inc.absFileName(), ".tex")).
			mangledFileName();
			if (!features.runparams().nice)
				incfile = mangled;
			// \includeonly doesn't want an extension
			incfile = changeExtension(incfile, string());
			incfile = support::latex_path(incfile);
			if (!incfile.empty()) {
				if (!first)
					os << ",";
				os << from_utf8(incfile);
			}
			first = false;
		}
		os << "}\n";
	}

	if (!features.isProvided("geometry")
	    && (use_geometry || nonstandard_papersize)) {
		odocstringstream ods;
		if (!getGraphicsDriver("geometry").empty())
			ods << getGraphicsDriver("geometry");
		if (orientation == ORIENTATION_LANDSCAPE)
			ods << ",landscape";
		switch (papersize) {
		case PAPER_CUSTOM:
			if (!paperwidth.empty())
				ods << ",paperwidth="
				   << from_ascii(paperwidth);
			if (!paperheight.empty())
				ods << ",paperheight="
				   << from_ascii(paperheight);
			break;
		case PAPER_USLETTER:
			ods << ",letterpaper";
			break;
		case PAPER_USLEGAL:
			ods << ",legalpaper";
			break;
		case PAPER_USEXECUTIVE:
			ods << ",executivepaper";
			break;
		case PAPER_A0:
			ods << ",a0paper";
			break;
		case PAPER_A1:
			ods << ",a1paper";
			break;
		case PAPER_A2:
			ods << ",a2paper";
			break;
		case PAPER_A3:
			ods << ",a3paper";
			break;
		case PAPER_A4:
			ods << ",a4paper";
			break;
		case PAPER_A5:
			ods << ",a5paper";
			break;
		case PAPER_A6:
			ods << ",a6paper";
			break;
		case PAPER_B0:
			ods << ",b0paper";
			break;
		case PAPER_B1:
			ods << ",b1paper";
			break;
		case PAPER_B2:
			ods << ",b2paper";
			break;
		case PAPER_B3:
			ods << ",b3paper";
			break;
		case PAPER_B4:
			ods << ",b4paper";
			break;
		case PAPER_B5:
			ods << ",b5paper";
			break;
		case PAPER_B6:
			ods << ",b6paper";
			break;
		case PAPER_C0:
			ods << ",c0paper";
			break;
		case PAPER_C1:
			ods << ",c1paper";
			break;
		case PAPER_C2:
			ods << ",c2paper";
			break;
		case PAPER_C3:
			ods << ",c3paper";
			break;
		case PAPER_C4:
			ods << ",c4paper";
			break;
		case PAPER_C5:
			ods << ",c5paper";
			break;
		case PAPER_C6:
			ods << ",c6paper";
			break;
		case PAPER_JISB0:
			ods << ",b0j";
			break;
		case PAPER_JISB1:
			ods << ",b1j";
			break;
		case PAPER_JISB2:
			ods << ",b2j";
			break;
		case PAPER_JISB3:
			ods << ",b3j";
			break;
		case PAPER_JISB4:
			ods << ",b4j";
			break;
		case PAPER_JISB5:
			ods << ",b5j";
			break;
		case PAPER_JISB6:
			ods << ",b6j";
			break;
		case PAPER_DEFAULT:
			break;
		}
		docstring const g_options = trim(ods.str(), ",");
		os << "\\usepackage";
		if (!g_options.empty())
			os << '[' << g_options << ']';
		os << "{geometry}\n";
		// output this only if use_geometry is true
		if (use_geometry) {
			os << "\\geometry{verbose";
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
		}
	} else if (orientation == ORIENTATION_LANDSCAPE
		   || papersize != PAPER_DEFAULT) {
		features.require("papersize");
	}

	if (tokenPos(tclass.opt_pagestyle(), '|', pagestyle) >= 0) {
		if (pagestyle == "fancy")
			os << "\\usepackage{fancyhdr}\n";
		os << "\\pagestyle{" << from_ascii(pagestyle) << "}\n";
	}

	// only output when the background color is not default
	if (isbackgroundcolor == true) {
		// only require color here, the background color will be defined
		// in LaTeXFeatures.cpp to avoid interferences with the LaTeX
		// package pdfpages
		features.require("color");
		features.require("pagecolor");
	}

	// only output when the font color is not default
	if (isfontcolor == true) {
		// only require color here, the font color will be defined
		// in LaTeXFeatures.cpp to avoid interferences with the LaTeX
		// package pdfpages
		features.require("color");
		features.require("fontcolor");
	}

	// Only if class has a ToC hierarchy
	if (tclass.hasTocLevels()) {
		if (secnumdepth != tclass.secnumdepth()) {
			os << "\\setcounter{secnumdepth}{"
			   << secnumdepth
			   << "}\n";
		}
		if (tocdepth != tclass.tocdepth()) {
			os << "\\setcounter{tocdepth}{"
			   << tocdepth
			   << "}\n";
		}
	}

	if (paragraph_separation) {
		// when skip separation
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
		os << "\\setlength{\\parindent}{0pt}\n";
	} else {
		// when separation by indentation
		// only output something when a width is given
		if (getIndentation().asLyXCommand() != "default") {
			os << "\\setlength{\\parindent}{"
			   << from_utf8(getIndentation().asLatexCommand())
			   << "}\n";
		}
	}

	// Now insert the LyX specific LaTeX commands...
	docstring lyxpreamble;
	features.resolveAlternatives();

	if (output_sync) {
		if (!output_sync_macro.empty())
			lyxpreamble += from_utf8(output_sync_macro) +"\n";
		else if (features.runparams().flavor == OutputParams::LATEX)
			lyxpreamble += "\\usepackage[active]{srcltx}\n";
		else if (features.runparams().flavor == OutputParams::PDFLATEX)
			lyxpreamble += "\\synctex=-1\n";
	}

	// due to interferences with babel and hyperref, the color package has to
	// be loaded (when it is not already loaded) before babel when hyperref
	// is used with the colorlinks option, see
	// http://www.lyx.org/trac/ticket/5291
	// we decided therefore to load color always before babel, see
	// http://www.mail-archive.com/lyx-devel@lists.lyx.org/msg144349.html
	lyxpreamble += from_ascii(features.getColorOptions());

	// If we use hyperref, jurabib, japanese, varioref or vietnamese,
	// we have to call babel before
	if (use_babel
	    && (features.isRequired("jurabib")
		|| features.isRequired("hyperref")
		|| features.isRequired("varioref")
		|| features.isRequired("vietnamese")
		|| features.isRequired("japanese"))) {
			// FIXME UNICODE
			lyxpreamble += from_utf8(features.getBabelPresettings());
			lyxpreamble += from_utf8(babelCall(language_options.str(),
							   features.needBabelLangOptions())) + '\n';
			lyxpreamble += from_utf8(features.getBabelPostsettings());
	}

	// The optional packages;
	lyxpreamble += from_ascii(features.getPackages());

	// Additional Indices
	if (features.isRequired("splitidx")) {
		IndicesList::const_iterator iit = indiceslist().begin();
		IndicesList::const_iterator iend = indiceslist().end();
		for (; iit != iend; ++iit) {
			pair<docstring, docstring> indexname_latex =
				features.runparams().encoding->latexString(iit->index(),
									   features.runparams().dryrun);
			if (!indexname_latex.second.empty()) {
				// issue a warning about omitted characters
				// FIXME: should be passed to the error dialog
				frontend::Alert::warning(_("Uncodable characters"),
					bformat(_("The following characters that are used in an index name are not\n"
						  "representable in the current encoding and therefore have been omitted:\n%1$s."),
						indexname_latex.second));
			}
			lyxpreamble += "\\newindex[";
			lyxpreamble += indexname_latex.first;
			lyxpreamble += "]{";
			lyxpreamble += escape(iit->shortcut());
			lyxpreamble += "}\n";
		}
	}

	// Line spacing
	lyxpreamble += from_utf8(spacing().writePreamble(features.isProvided("SetSpace")));

	// PDF support.
	// * Hyperref manual: "Make sure it comes last of your loaded
	//   packages, to give it a fighting chance of not being over-written,
	//   since its job is to redefine many LaTeX commands."
	// * Email from Heiko Oberdiek: "It is usually better to load babel
	//   before hyperref. Then hyperref has a chance to detect babel.
	// * Has to be loaded before the "LyX specific LaTeX commands" to
	//   avoid errors with algorithm floats.
	// use hyperref explicitly if it is required
	if (features.isRequired("hyperref")) {
		// pass what we have to stream here, since we need
		// to access the stream itself in PDFOptions.
		os << lyxpreamble;

		OutputParams tmp_params = features.runparams();
		pdfoptions().writeLaTeX(tmp_params, os,
					features.isProvided("hyperref"));
		// set back for the rest
		lyxpreamble.clear();
		// correctly break URLs with hyperref and dvi output
		if (features.runparams().flavor == OutputParams::LATEX
		    && features.isAvailable("breakurl"))
			lyxpreamble += "\\usepackage{breakurl}\n";
	} else if (features.isRequired("nameref"))
		// hyperref loads this automatically
		lyxpreamble += "\\usepackage{nameref}\n";

	// bibtopic needs to be loaded after hyperref.
	// the dot provides the aux file naming which LyX can detect.
	if (features.mustProvide("bibtopic"))
		lyxpreamble += "\\usepackage[dot]{bibtopic}\n";

	// Will be surrounded by \makeatletter and \makeatother when not empty
	docstring atlyxpreamble;

	// Some macros LyX will need
	docstring tmppreamble(features.getMacros());

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

	// suppress date if selected
	// use \@ifundefined because we cannot be sure that every document class
	// has a \date command
	if (suppress_date)
		atlyxpreamble += "\\@ifundefined{date}{}{\\date{}}\n";

	/* the user-defined preamble */
	if (!containsOnly(preamble, " \n\t"))
		// FIXME UNICODE
		atlyxpreamble += "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% "
			"User specified LaTeX commands.\n"
			+ from_utf8(preamble) + '\n';

	// footmisc must be loaded after setspace
	// Load it here to avoid clashes with footmisc loaded in the user
	// preamble. For that reason we also pass the options via
	// \PassOptionsToPackage in getPreamble() and not here.
	if (features.mustProvide("footmisc"))
		atlyxpreamble += "\\usepackage{footmisc}\n";

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

	if (!atlyxpreamble.empty())
		lyxpreamble += "\n\\makeatletter\n"
			+ atlyxpreamble + "\\makeatother\n\n";

	// We try to load babel late, in case it interferes with other packages.
	// Jurabib, hyperref, varioref and listings (bug 8995) have to be
	// called after babel, though.
	if (use_babel && !features.isRequired("jurabib")
	    && !features.isRequired("hyperref")
		&& !features.isRequired("varioref")
	    && !features.isRequired("vietnamese")
	    && !features.isRequired("japanese")) {
		// FIXME UNICODE
		lyxpreamble += from_utf8(features.getBabelPresettings());
		lyxpreamble += from_utf8(babelCall(language_options.str(),
						   features.needBabelLangOptions())) + '\n';
		lyxpreamble += from_utf8(features.getBabelPostsettings());
	}
	if (!listings_params.empty() || features.isRequired("listings"))
		lyxpreamble += "\\usepackage{listings}\n";
	if (!listings_params.empty()) {
		lyxpreamble += "\\lstset{";
		// do not test validity because listings_params is
		// supposed to be valid
		string par =
			InsetListingsParams(listings_params).separatedParams(true);
		lyxpreamble += from_utf8(par);
		lyxpreamble += "}\n";
	}

	// xunicode needs to be loaded at least after amsmath, amssymb,
	// esint and the other packages that provide special glyphs
	if (features.runparams().flavor == OutputParams::XETEX
	    && useNonTeXFonts)
		lyxpreamble += "\\usepackage{xunicode}\n";

	// Polyglossia must be loaded last
	if (use_polyglossia) {
		// call the package
		lyxpreamble += "\\usepackage{polyglossia}\n";
		// set the main language
		lyxpreamble += "\\setdefaultlanguage";
		if (!language->polyglossiaOpts().empty())
			lyxpreamble += "[" + from_ascii(language->polyglossiaOpts()) + "]";
		lyxpreamble += "{" + from_ascii(language->polyglossia()) + "}\n";
		// now setup the other languages
		std::map<std::string, std::string> const polylangs =
			features.getPolyglossiaLanguages();
		for (std::map<std::string, std::string>::const_iterator mit = polylangs.begin();
		     mit != polylangs.end() ; ++mit) {
			lyxpreamble += "\\setotherlanguage";
			if (!mit->second.empty())
				lyxpreamble += "[" + from_ascii(mit->second) + "]";
			lyxpreamble += "{" + from_ascii(mit->first) + "}\n";
		}
	}

	// Load custom language package here
	if (features.langPackage() == LaTeXFeatures::LANG_PACK_CUSTOM) {
		if (lang_package == "default")
			lyxpreamble += from_utf8(lyxrc.language_custom_package);
		else
			lyxpreamble += from_utf8(lang_package);
		lyxpreamble += '\n';
	}

	docstring const i18npreamble =
		features.getTClassI18nPreamble(use_babel, use_polyglossia);
	if (!i18npreamble.empty())
		lyxpreamble += i18npreamble + '\n';

	os << lyxpreamble;

	return use_babel;
}


void BufferParams::useClassDefaults()
{
	DocumentClass const & tclass = documentClass();

	sides = tclass.sides();
	columns = tclass.columns();
	pagestyle = tclass.pagestyle();
	use_default_options = true;
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
		&& use_default_options
		&& secnumdepth == tclass.secnumdepth()
		&& tocdepth == tclass.tocdepth();
}


DocumentClass const & BufferParams::documentClass() const
{
	return *doc_class_.get();
}


DocumentClassConstPtr BufferParams::documentClassPtr() const
{
	return doc_class_;
}


void BufferParams::setDocumentClass(DocumentClassConstPtr tc)
{
	// evil, but this function is evil
	doc_class_ = const_pointer_cast<DocumentClass>(tc);
}


bool BufferParams::setBaseClass(string const & classname)
{
	LYXERR(Debug::TCLASS, "setBaseClass: " << classname);
	LayoutFileList & bcl = LayoutFileList::get();
	if (!bcl.haveClass(classname)) {
		docstring s =
			bformat(_("The layout file:\n"
				"%1$s\n"
				"could not be found. A default textclass with default\n"
				"layouts will be used. LyX will not be able to produce\n"
				"correct output."),
			from_utf8(classname));
		frontend::Alert::error(_("Document class not found"), s);
		bcl.addEmptyClass(classname);
	}

	bool const success = bcl[classname].load();
	if (!success) {
		docstring s =
			bformat(_("Due to some error in it, the layout file:\n"
				"%1$s\n"
				"could not be loaded. A default textclass with default\n"
				"layouts will be used. LyX will not be able to produce\n"
				"correct output."),
			from_utf8(classname));
		frontend::Alert::error(_("Could not load class"), s);
		bcl.addEmptyClass(classname);
	}

	pimpl_->baseClass_ = classname;
	layout_modules_.adaptToBaseClass(baseClass(), removed_modules_);
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


void BufferParams::makeDocumentClass(bool const clone)
{
	if (!baseClass())
		return;

	LayoutModuleList mods;
	LayoutModuleList::iterator it = layout_modules_.begin();
	LayoutModuleList::iterator en = layout_modules_.end();
	for (; it != en; ++it)
		mods.push_back(*it);

	it = cite_engine_.begin();
	en = cite_engine_.end();
	for (; it != en; ++it)
		mods.push_back(*it);

	doc_class_ = getDocumentClass(*baseClass(), mods, clone);

	TextClass::ReturnValues success = TextClass::OK;
	if (!forced_local_layout_.empty())
		success = doc_class_->read(forced_local_layout_, TextClass::MODULE);
	if (!local_layout_.empty() &&
	    (success == TextClass::OK || success == TextClass::OK_OLDFORMAT))
		success = doc_class_->read(local_layout_, TextClass::MODULE);
	if (success != TextClass::OK && success != TextClass::OK_OLDFORMAT) {
		docstring const msg = _("Error reading internal layout information");
		frontend::Alert::warning(_("Read Error"), msg);
	}
}


bool BufferParams::layoutModuleCanBeAdded(string const & modName) const
{
	return layout_modules_.moduleCanBeAdded(modName, baseClass());
}


bool BufferParams::citationModuleCanBeAdded(string const & modName) const
{
	return cite_engine_.moduleCanBeAdded(modName, baseClass());
}


std::string BufferParams::getLocalLayout(bool forced) const
{
	if (forced)
		return doc_class_->forcedLayouts();
	else
		return local_layout_;
}


void BufferParams::setLocalLayout(string const & layout, bool forced)
{
	if (forced)
		forced_local_layout_ = layout;
	else
		local_layout_ = layout;
}


bool BufferParams::addLayoutModule(string const & modName)
{
	LayoutModuleList::const_iterator it = layout_modules_.begin();
	LayoutModuleList::const_iterator end = layout_modules_.end();
	for (; it != end; ++it)
		if (*it == modName)
			return false;
	layout_modules_.push_back(modName);
	return true;
}


string BufferParams::bufferFormat() const
{
	string format = documentClass().outputFormat();
	if (format == "latex") {
		if (useNonTeXFonts)
			return "xetex";
		if (encoding().package() == Encoding::japanese)
			return "platex";
	}
	return format;
}


bool BufferParams::isExportable(string const & format) const
{
	vector<string> backs = backends();
	for (vector<string>::const_iterator it = backs.begin();
	     it != backs.end(); ++it)
		if (theConverters().isReachable(*it, format))
			return true;
	return false;
}


vector<Format const *> BufferParams::exportableFormats(bool only_viewable) const
{
	vector<string> const backs = backends();
	set<string> excludes;
	if (useNonTeXFonts) {
		excludes.insert("latex");
		excludes.insert("pdflatex");
	}
	vector<Format const *> result =
		theConverters().getReachable(backs[0], only_viewable, true, excludes);
	for (vector<string>::const_iterator it = backs.begin() + 1;
	     it != backs.end(); ++it) {
		vector<Format const *>  r =
			theConverters().getReachable(*it, only_viewable, false, excludes);
		result.insert(result.end(), r.begin(), r.end());
	}
	return result;
}


bool BufferParams::isExportableFormat(string const & format) const
{
	typedef vector<Format const *> Formats;
	Formats formats;
	formats = exportableFormats(true);
	Formats::const_iterator fit = formats.begin();
	Formats::const_iterator end = formats.end();
	for (; fit != end ; ++fit) {
		if ((*fit)->name() == format)
			return true;
	}
	return false;
}


vector<string> BufferParams::backends() const
{
	vector<string> v;
	string const buffmt = bufferFormat();

	// FIXME: Don't hardcode format names here, but use a flag
	if (buffmt == "latex") {
		if (!useNonTeXFonts) {
			v.push_back("pdflatex");
			v.push_back("latex");
		}
		v.push_back("luatex");
		v.push_back("dviluatex");
		v.push_back("xetex");
	} else if (buffmt == "xetex") {
		v.push_back("xetex");
		// FIXME: need to test all languages (bug 8205)
		if (!language || !language->isPolyglossiaExclusive()) {
			v.push_back("luatex");
			v.push_back("dviluatex");
		}
	} else
		v.push_back(buffmt);

	v.push_back("xhtml");
	v.push_back("text");
	v.push_back("lyx");
	return v;
}


OutputParams::FLAVOR BufferParams::getOutputFlavor(string const & format) const
{
	string const dformat = (format.empty() || format == "default") ?
		getDefaultOutputFormat() : format;
	DefaultFlavorCache::const_iterator it =
		default_flavors_.find(dformat);

	if (it != default_flavors_.end())
		return it->second;

	OutputParams::FLAVOR result = OutputParams::LATEX;

	// FIXME It'd be better not to hardcode this, but to do
	//       something with formats.
	if (dformat == "xhtml")
		result = OutputParams::HTML;
	else if (dformat == "text")
		result = OutputParams::TEXT;
	else if (dformat == "lyx")
		result = OutputParams::LYX;
	else if (dformat == "pdflatex")
 		result = OutputParams::PDFLATEX;
	else if (dformat == "xetex")
		result = OutputParams::XETEX;
	else if (dformat == "luatex")
		result = OutputParams::LUATEX;
	else if (dformat == "dviluatex")
		result = OutputParams::DVILUATEX;
	else {
		// Try to determine flavor of default output format
		vector<string> backs = backends();
		if (find(backs.begin(), backs.end(), dformat) == backs.end()) {
			// Get shortest path to format
			Graph::EdgePath path;
			for (vector<string>::const_iterator it = backs.begin();
			    it != backs.end(); ++it) {
				Graph::EdgePath p = theConverters().getPath(*it, dformat);
				if (!p.empty() && (path.empty() || p.size() < path.size())) {
					path = p;
				}
			}
			if (!path.empty())
				result = theConverters().getFlavor(path);
		}
	}
	// cache this flavor
	default_flavors_[dformat] = result;
	return result;
}


string BufferParams::getDefaultOutputFormat() const
{
	if (!default_output_format.empty()
	    && default_output_format != "default")
		return default_output_format;
	if (isDocBook()
	    || encoding().package() == Encoding::japanese) {
		vector<Format const *> const formats = exportableFormats(true);
		if (formats.empty())
			return string();
		// return the first we find
		return formats.front()->name();
	}
	if (useNonTeXFonts)
		return lyxrc.default_otf_view_format;
	return lyxrc.default_view_format;
}

Font const BufferParams::getFont() const
{
	FontInfo f = documentClass().defaultfont();
	if (fonts_default_family == "rmdefault")
		f.setFamily(ROMAN_FAMILY);
	else if (fonts_default_family == "sfdefault")
		f.setFamily(SANS_FAMILY);
	else if (fonts_default_family == "ttdefault")
		f.setFamily(TYPEWRITER_FAMILY);
	return Font(f, language);
}


InsetQuotes::QuoteLanguage BufferParams::getQuoteStyle(string const & qs) const
{
	return quoteslangtranslator().find(qs);
}


bool BufferParams::isLatex() const
{
	return documentClass().outputType() == LATEX;
}


bool BufferParams::isLiterate() const
{
	return documentClass().outputType() == LITERATE;
}


bool BufferParams::isDocBook() const
{
	return documentClass().outputType() == DOCBOOK;
}


void BufferParams::readPreamble(Lexer & lex)
{
	if (lex.getString() != "\\begin_preamble")
		lyxerr << "Error (BufferParams::readPreamble):"
			"consistency check failed." << endl;

	preamble = lex.getLongString("\\end_preamble");
}


void BufferParams::readLocalLayout(Lexer & lex, bool forced)
{
	string const expected = forced ? "\\begin_forced_local_layout" :
	                                 "\\begin_local_layout";
	if (lex.getString() != expected)
		lyxerr << "Error (BufferParams::readLocalLayout):"
			"consistency check failed." << endl;

	if (forced)
		forced_local_layout_ =
			lex.getLongString("\\end_forced_local_layout");
	else
		local_layout_ = lex.getLongString("\\end_local_layout");
}


bool BufferParams::setLanguage(string const & lang)
{
	Language const *new_language = languages.getLanguage(lang);
	if (!new_language) {
		// Language lang was not found
		return false;
	}
	language = new_language;
	return true;
}


void BufferParams::readLanguage(Lexer & lex)
{
	if (!lex.next()) return;

	string const tmptok = lex.getString();

	// check if tmptok is part of tex_babel in tex-defs.h
	if (!setLanguage(tmptok)) {
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
			graphics_driver = tmptok;
			break;
		}
		if (test.empty()) {
			lex.printError(
				"Warning: graphics driver `$$Token' not recognized!\n"
				"         Setting graphics driver to `default'.\n");
			graphics_driver = "default";
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
		removed_modules_.push_back(mod);
		lex.eatLine();
	}
	// now we want to remove any removed modules that were previously
	// added. normally, that will be because default modules were added in
	// setBaseClass(), which gets called when \textclass is read at the
	// start of the read.
	list<string>::const_iterator rit = removed_modules_.begin();
	list<string>::const_iterator const ren = removed_modules_.end();
	for (; rit != ren; ++rit) {
		LayoutModuleList::iterator const mit = layout_modules_.begin();
		LayoutModuleList::iterator const men = layout_modules_.end();
		LayoutModuleList::iterator found = find(mit, men, *rit);
		if (found == men)
			continue;
		layout_modules_.erase(found);
	}
}


void BufferParams::readIncludeonly(Lexer & lex)
{
	if (!lex.eatLine()) {
		lyxerr << "Error (BufferParams::readIncludeonly):"
				"Unexpected end of input." << endl;
		return;
	}
	while (true) {
		string child = lex.getString();
		if (child == "\\end_includeonly")
			break;
		included_children_.push_back(child);
		lex.eatLine();
	}
}


string BufferParams::paperSizeName(PapersizePurpose purpose) const
{
	switch (papersize) {
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
	case PAPER_A0:
		// dvips and dvipdfm do not know this
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "a0";
	case PAPER_A1:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "a1";
	case PAPER_A2:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "a2";
	case PAPER_A3:
		return "a3";
	case PAPER_A4:
		return "a4";
	case PAPER_A5:
		return "a5";
	case PAPER_A6:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "a6";
	case PAPER_B0:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "b0";
	case PAPER_B1:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "b1";
	case PAPER_B2:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "b2";
	case PAPER_B3:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "b3";
	case PAPER_B4:
		// dvipdfm does not know this
		if (purpose == DVIPDFM)
			return string();
		return "b4";
	case PAPER_B5:
		if (purpose == DVIPDFM)
			return string();
		return "b5";
	case PAPER_B6:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "b6";
	case PAPER_C0:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "c0";
	case PAPER_C1:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "c1";
	case PAPER_C2:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "c2";
	case PAPER_C3:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "c3";
	case PAPER_C4:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "c4";
	case PAPER_C5:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "c5";
	case PAPER_C6:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "c6";
	case PAPER_JISB0:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "jisb0";
	case PAPER_JISB1:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "jisb1";
	case PAPER_JISB2:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "jisb2";
	case PAPER_JISB3:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "jisb3";
	case PAPER_JISB4:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "jisb4";
	case PAPER_JISB5:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "jisb5";
	case PAPER_JISB6:
		if (purpose == DVIPS || purpose == DVIPDFM)
			return string();
		return "jisb6";
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

	// If the class loads the geometry package, we do not know which
	// paper size is used, since we do not set it (bug 7013).
	// Therefore we must not specify any argument here.
	// dvips gets the correct paper size via DVI specials in this case
	// (if the class uses the geometry package correctly).
	if (documentClass().provides("geometry"))
		return result;

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


string const BufferParams::font_encoding() const
{
	return (fontenc == "global") ? lyxrc.fontenc : fontenc;
}


string BufferParams::babelCall(string const & lang_opts, bool const langoptions) const
{
	// suppress the babel call if there is no BabelName defined
	// for the document language in the lib/languages file and if no
	// other languages are used (lang_opts is then empty)
	if (lang_opts.empty())
		return string();
	// either a specific language (AsBabelOptions setting in
	// lib/languages) or the prefs require the languages to
	// be submitted to babel itself (not the class).
	if (langoptions)
		return "\\usepackage[" + lang_opts + "]{babel}";
	return "\\usepackage{babel}";
}


docstring BufferParams::getGraphicsDriver(string const & package) const
{
	docstring result;

	if (package == "geometry") {
		if (graphics_driver == "dvips"
		    || graphics_driver == "dvipdfm"
		    || graphics_driver == "pdftex"
		    || graphics_driver == "vtex")
			result = from_ascii(graphics_driver);
		else if (graphics_driver == "dvipdfmx")
			result = from_ascii("dvipdfm");
	}

	return result;
}


void BufferParams::writeEncodingPreamble(otexstream & os,
					 LaTeXFeatures & features) const
{
	// XeTeX does not need this
	if (features.runparams().flavor == OutputParams::XETEX)
		return;
	// LuaTeX neither, but with tex fonts, we need to load
	// the luainputenc package.
	if (features.runparams().flavor == OutputParams::LUATEX
		|| features.runparams().flavor == OutputParams::DVILUATEX) {
		if (!useNonTeXFonts && inputenc != "default"
		    && ((inputenc == "auto" && language->encoding()->package() == Encoding::inputenc)
		        || (inputenc != "auto" && encoding().package() == Encoding::inputenc))) {
			os << "\\usepackage[utf8]{luainputenc}\n";
		}
		return;
	}
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
		}
		if (package == Encoding::CJK || features.mustProvide("CJK")) {
			if (language->encoding()->name() == "utf8-cjk"
			    && LaTeXFeatures::isAvailable("CJKutf8"))
				os << "\\usepackage{CJKutf8}\n";
			else
				os << "\\usepackage{CJK}\n";
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
			os << "\\usepackage[" << from_ascii(encoding().latexName())
			   << "]{inputenc}\n";
			break;
		case Encoding::CJK:
			if (encoding().name() == "utf8-cjk"
			    && LaTeXFeatures::isAvailable("CJKutf8"))
				os << "\\usepackage{CJKutf8}\n";
			else
				os << "\\usepackage{CJK}\n";
			break;
		}
		// Load the CJK package if needed by a secondary language.
		// If the main encoding is some variant of UTF8, use CJKutf8.
		if (encoding().package() != Encoding::CJK && features.mustProvide("CJK")) {
			if (encoding().iconvName() == "UTF-8"
			    && LaTeXFeatures::isAvailable("CJKutf8"))
				os << "\\usepackage{CJKutf8}\n";
			else
				os << "\\usepackage{CJK}\n";
		}
	}
}


string const BufferParams::parseFontName(string const & name) const
{
	string mangled = name;
	size_t const idx = mangled.find('[');
	if (idx == string::npos || idx == 0)
		return mangled;
	else
		return mangled.substr(0, idx - 1);
}


string const BufferParams::loadFonts(LaTeXFeatures & features) const
{
	if (fonts_roman == "default" && fonts_sans == "default"
	    && fonts_typewriter == "default"
	    && (fonts_math == "default" || fonts_math == "auto"))
		//nothing to do
		return string();

	ostringstream os;

	/* Fontspec (XeTeX, LuaTeX): we provide GUI support for oldstyle
	 * numbers (Numbers=OldStyle) and sf/tt scaling. The Ligatures=TeX/
	 * Mapping=tex-text option assures TeX ligatures (such as "--")
	 * are resolved. Note that tt does not use these ligatures.
	 * TODO:
	 *    -- add more GUI options?
	 *    -- add more fonts (fonts for other scripts)
	 *    -- if there's a way to find out if a font really supports
	 *       OldStyle, enable/disable the widget accordingly.
	*/
	if (useNonTeXFonts && features.isAvailable("fontspec")) {
		// "Mapping=tex-text" and "Ligatures=TeX" are equivalent.
		// However, until v.2 (2010/07/11) fontspec only knew
		// Mapping=tex-text (for XeTeX only); then "Ligatures=TeX"
		// was introduced for both XeTeX and LuaTeX (LuaTeX
		// didn't understand "Mapping=tex-text", while XeTeX
		// understood both. With most recent versions, both
		// variants are understood by both engines. However,
		// we want to provide support for at least TeXLive 2009
		// (for XeTeX; LuaTeX is only supported as of v.2)
		string const texmapping =
			(features.runparams().flavor == OutputParams::XETEX) ?
			"Mapping=tex-text" : "Ligatures=TeX";
		if (fonts_roman != "default") {
			os << "\\setmainfont[" << texmapping;
			if (fonts_old_figures)
				os << ",Numbers=OldStyle";
			os << "]{" << parseFontName(fonts_roman) << "}\n";
		}
		if (fonts_sans != "default") {
			string const sans = parseFontName(fonts_sans);
			if (fonts_sans_scale != 100)
				os << "\\setsansfont[Scale="
				   << float(fonts_sans_scale) / 100
				   << "," << texmapping << "]{"
				   << sans << "}\n";
			else
				os << "\\setsansfont[" << texmapping << "]{"
				   << sans << "}\n";
		}
		if (fonts_typewriter != "default") {
			string const mono = parseFontName(fonts_typewriter);
			if (fonts_typewriter_scale != 100)
				os << "\\setmonofont[Scale="
				   << float(fonts_typewriter_scale) / 100
				   << "]{"
				   << mono << "}\n";
			else
				os << "\\setmonofont{"
				   << mono << "}\n";
		}
		return os.str();
	}

	// Tex Fonts
	bool const ot1 = (font_encoding() == "default" || font_encoding() == "OT1");
	bool const dryrun = features.runparams().dryrun;
	bool const complete = (fonts_sans == "default" && fonts_typewriter == "default");
	bool const nomath = (fonts_math == "default");

	// ROMAN FONTS
	os << theLaTeXFonts().getLaTeXFont(from_ascii(fonts_roman)).getLaTeXCode(
		dryrun, ot1, complete, fonts_expert_sc, fonts_old_figures,
		nomath);

	// SANS SERIF
	os << theLaTeXFonts().getLaTeXFont(from_ascii(fonts_sans)).getLaTeXCode(
		dryrun, ot1, complete, fonts_expert_sc, fonts_old_figures,
		nomath, fonts_sans_scale);

	// MONOSPACED/TYPEWRITER
	os << theLaTeXFonts().getLaTeXFont(from_ascii(fonts_typewriter)).getLaTeXCode(
		dryrun, ot1, complete, fonts_expert_sc, fonts_old_figures,
		nomath, fonts_typewriter_scale);

	// MATH
	os << theLaTeXFonts().getLaTeXFont(from_ascii(fonts_math)).getLaTeXCode(
		dryrun, ot1, complete, fonts_expert_sc, fonts_old_figures,
		nomath);

	return os.str();
}


Encoding const & BufferParams::encoding() const
{
	// FIXME: actually, we should check for the flavor
	// or runparams.isFullyUnicode() here:
	// This check will not work with XeTeX/LuaTeX and tex fonts.
	// Thus we have to reset the encoding in Buffer::makeLaTeXFile.
	if (useNonTeXFonts)
		return *(encodings.fromLyXName("utf8-plain"));
	if (inputenc == "auto" || inputenc == "default")
		return *language->encoding();
	Encoding const * const enc = encodings.fromLyXName(inputenc);
	if (enc)
		return *enc;
	LYXERR0("Unknown inputenc value `" << inputenc
	       << "'. Using `auto' instead.");
	return *language->encoding();
}


bool BufferParams::addCiteEngine(string const & engine)
{
	LayoutModuleList::const_iterator it = cite_engine_.begin();
	LayoutModuleList::const_iterator en = cite_engine_.end();
	for (; it != en; ++it)
		if (*it == engine)
			return false;
	cite_engine_.push_back(engine);
	return true;
}


bool BufferParams::addCiteEngine(vector<string> const & engine)
{
	vector<string>::const_iterator it = engine.begin();
	vector<string>::const_iterator en = engine.end();
	bool ret = true;
	for (; it != en; ++it)
		if (!addCiteEngine(*it))
			ret = false;
	return ret;
}


string const & BufferParams::defaultBiblioStyle() const
{
	return documentClass().defaultBiblioStyle();
}


bool const & BufferParams::fullAuthorList() const
{
	return documentClass().fullAuthorList();
}


void BufferParams::setCiteEngine(string const & engine)
{
	clearCiteEngine();
	addCiteEngine(engine);
}


void BufferParams::setCiteEngine(vector<string> const & engine)
{
	clearCiteEngine();
	addCiteEngine(engine);
}


vector<string> BufferParams::citeCommands() const
{
	static CitationStyle const default_style;
	vector<string> commands =
		documentClass().citeCommands(citeEngineType());
	if (commands.empty())
		commands.push_back(default_style.cmd);
	return commands;
}


vector<CitationStyle> BufferParams::citeStyles() const
{
	static CitationStyle const default_style;
	vector<CitationStyle> styles =
		documentClass().citeStyles(citeEngineType());
	if (styles.empty())
		styles.push_back(default_style);
	return styles;
}

} // namespace lyx
