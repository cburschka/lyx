/**
 * \file Preamble.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

// {[(

#include <config.h>

#include "Preamble.h"
#include "tex2lyx.h"

#include "LayoutFile.h"
#include "Layout.h"
#include "Lexer.h"
#include "TextClass.h"

#include "support/convert.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/lstrings.h"

#include "support/regex.h"

#include <algorithm>
#include <iostream>

using namespace std;
using namespace lyx::support;


namespace lyx {

// special columntypes
extern map<char, int> special_columns;

Preamble preamble;

namespace {

//add this to known_languages when updating to lyxformat 266:
// "armenian" (needs special handling since not supported by standard babel)
//add these to known_languages when updating to lyxformat 268:
//"chinese-simplified", "chinese-traditional", "japanese", "korean"
// Both changes require first that support for non-babel languages (CJK,
// armtex) is added.
/**
 * known babel language names (including synonyms)
 * not in standard babel: arabic, arabtex, armenian, belarusian, serbian-latin, thai
 * not yet supported by LyX: kurmanji
 * please keep this in sync with known_coded_languages line by line!
 */
const char * const known_languages[] = {"acadian", "afrikaans", "albanian",
"american", "arabic", "arabtex", "australian", "austrian", "bahasa", "bahasai",
"bahasam", "basque", "belarusian", "brazil", "brazilian", "breton", "british",
"bulgarian", "canadian", "canadien", "catalan", "croatian", "czech", "danish",
"dutch", "english", "esperanto", "estonian", "farsi", "finnish", "francais",
"french", "frenchb", "frenchle", "frenchpro", "galician", "german", "germanb",
"greek", "hebrew", "hungarian", "icelandic", "indon", "indonesian", "interlingua",
"irish", "italian", "kazakh", "latin", "latvian", "lithuanian", "lowersorbian",
"lsorbian", "magyar", "malay", "meyalu", "mongolian", "naustrian", "newzealand",
"ngerman", "ngermanb", "norsk", "nynorsk", "polutonikogreek", "polish",
"portuges", "portuguese", "romanian", "russian", "russianb", "samin",
"scottish", "serbian", "serbian-latin", "slovak", "slovene", "spanish",
"swedish", "thai", "turkish", "turkmen", "ukraineb", "ukrainian",
"uppersorbian", "UKenglish", "USenglish", "usorbian", "vietnam", "welsh",
0};

/**
 * the same as known_languages with .lyx names
 * please keep this in sync with known_languages line by line!
 */
const char * const known_coded_languages[] = {"french", "afrikaans", "albanian",
"american", "arabic_arabi", "arabic_arabtex", "australian", "austrian", "bahasa", "bahasa",
"bahasam", "basque", "belarusian", "brazilian", "brazilian", "breton", "british",
"bulgarian", "canadian", "canadien", "catalan", "croatian", "czech", "danish",
"dutch", "english", "esperanto", "estonian", "farsi", "finnish", "french",
"french", "french", "french", "french", "galician", "german", "german",
"greek", "hebrew", "magyar", "icelandic", "bahasa", "bahasa", "interlingua",
"irish", "italian", "kazakh", "latin", "latvian", "lithuanian", "lowersorbian",
"lowersorbian", "magyar", "bahasam", "bahasam", "mongolian", "naustrian", "newzealand",
"ngerman", "ngerman", "norsk", "nynorsk", "polutonikogreek", "polish",
"portuguese", "portuguese", "romanian", "russian", "russian", "samin",
"scottish", "serbian", "serbian-latin", "slovak", "slovene", "spanish",
"swedish", "thai", "turkish", "turkmen", "ukrainian", "ukrainian",
"uppersorbian", "uppersorbian", "english", "english", "vietnamese", "welsh",
0};

/// languages with english quotes (.lyx names)
const char * const known_english_quotes_languages[] = {"american", "australian",
"bahasa", "bahasam", "brazilian", "canadian", "chinese-simplified", "english",
"esperanto", "hebrew", "irish", "korean", "newzealand", "portuguese", "scottish",
"thai", 0};

/// languages with french quotes (.lyx names)
const char * const known_french_quotes_languages[] = {"albanian",
"arabic_arabi", "arabic_arabtex", "basque", "canadien", "catalan", "french",
"galician", "greek", "italian", "norsk", "nynorsk", "polutonikogreek",
"russian", "spanish", "spanish-mexico", "turkish", "turkmen", "ukrainian",
"vietnamese", 0};

/// languages with german quotes (.lyx names)
const char * const known_german_quotes_languages[] = {"austrian", "bulgarian",
"czech", "german", "icelandic", "lithuanian", "lowersorbian", "naustrian",
"ngerman", "serbian", "serbian-latin", "slovak", "slovene", "uppersorbian", 0};

/// languages with polish quotes (.lyx names)
const char * const known_polish_quotes_languages[] = {"afrikaans", "croatian",
"dutch", "estonian", "magyar", "polish", "romanian", 0};

/// languages with swedish quotes (.lyx names)
const char * const known_swedish_quotes_languages[] = {"finnish",
"swedish", 0};

/// known language packages from the times before babel
const char * const known_old_language_packages[] = {"french", "frenchle",
"frenchpro", "german", "ngerman", "pmfrench", 0};

char const * const known_fontsizes[] = { "10pt", "11pt", "12pt", 0 };

const char * const known_roman_fonts[] = { "ae", "beraserif", "bookman",
"ccfonts", "chancery", "charter", "cmr", "fourier", "lmodern", "mathpazo",
"mathptmx", "newcent", "utopia", 0};

const char * const known_sans_fonts[] = { "avant", "berasans", "cmbr", "cmss",
"helvet", "lmss", 0};

const char * const known_typewriter_fonts[] = { "beramono", "cmtl", "cmtt",
"courier", "lmtt", "luximono", "fourier", "lmodern", "mathpazo", "mathptmx",
"newcent", 0};

const char * const known_paper_sizes[] = { "a0paper", "b0paper", "c0paper",
"a1paper", "b1paper", "c1paper", "a2paper", "b2paper", "c2paper", "a3paper",
"b3paper", "c3paper", "a4paper", "b4paper", "c4paper", "a5paper", "b5paper",
"c5paper", "a6paper", "b6paper", "c6paper", "executivepaper", "legalpaper",
"letterpaper", "b0j", "b1j", "b2j", "b3j", "b4j", "b5j", "b6j", 0};

const char * const known_class_paper_sizes[] = { "a4paper", "a5paper",
"executivepaper", "legalpaper", "letterpaper", 0};

const char * const known_paper_margins[] = { "lmargin", "tmargin", "rmargin",
"bmargin", "headheight", "headsep", "footskip", "columnsep", 0};

const char * const known_coded_paper_margins[] = { "leftmargin", "topmargin",
"rightmargin", "bottommargin", "headheight", "headsep", "footskip",
"columnsep", 0};

/// commands that can start an \if...\else...\endif sequence
const char * const known_if_commands[] = {"if", "ifarydshln", "ifbraket",
"ifcancel", "ifcolortbl", "ifeurosym", "ifmarginnote", "ifmmode", "ifpdf",
"ifsidecap", "ifupgreek", 0};

const char * const known_basic_colors[] = {"blue", "black", "cyan", "green",
"magenta", "red", "white", "yellow", 0};

const char * const known_basic_color_codes[] = {"#0000ff", "#000000", "#00ffff", "#00ff00",
"#ff00ff", "#ff0000", "#ffffff", "#ffff00", 0};

/// conditional commands with three arguments like \@ifundefined{}{}{}
const char * const known_if_3arg_commands[] = {"@ifundefined", "IfFileExists",
0};

/// packages that work only in xetex
const char * const known_xetex_packages[] = {"arabxetex", "fixlatvian",
"fontbook", "fontwrap", "mathspec", "philokalia", "polyglossia", "unisugar",
"xeCJK", "xecolor", "xecyr", "xeindex", "xepersian", "xunicode", 0};

/// packages that are automatically skipped if loaded by LyX
const char * const known_lyx_packages[] = {"array", "booktabs", "calc",
"color", "float", "graphicx", "hhline", "ifthen", "longtable", "makeidx",
"multirow", "nomencl", "pdfpages", "rotfloat", "splitidx", "setspace",
"subscript", "textcomp", "ulem", "url", "varioref", "verbatim", "wrapfig", 0};

// codes used to remove packages that are loaded automatically by LyX.
// Syntax: package_beg_sep<name>package_mid_sep<package loading code>package_end_sep
const char package_beg_sep = '\001';
const char package_mid_sep = '\002';
const char package_end_sep = '\003';


// returns true if at least one of the options in what has been found
bool handle_opt(vector<string> & opts, char const * const * what, string & target)
{
	if (opts.empty())
		return false;

	bool found = false;
	// the last language option is the document language (for babel and LyX)
	// the last size option is the document font size
	vector<string>::iterator it;
	vector<string>::iterator position = opts.begin();
	for (; *what; ++what) {
		it = find(opts.begin(), opts.end(), *what);
		if (it != opts.end()) {
			if (it >= position) {
				found = true;
				target = *what;
				position = it;
			}
		}
	}
	return found;
}


void delete_opt(vector<string> & opts, char const * const * what)
{
	if (opts.empty())
		return;

	// remove found options from the list
	// do this after handle_opt to avoid potential memory leaks
	vector<string>::iterator it;
	for (; *what; ++what) {
		it = find(opts.begin(), opts.end(), *what);
		if (it != opts.end())
			opts.erase(it);
	}
}


/*!
 * Split a package options string (keyval format) into a vector.
 * Example input:
 *   authorformat=smallcaps,
 *   commabeforerest,
 *   titleformat=colonsep,
 *   bibformat={tabular,ibidem,numbered}
 */
vector<string> split_options(string const & input)
{
	vector<string> options;
	string option;
	Parser p(input);
	while (p.good()) {
		Token const & t = p.get_token();
		if (t.asInput() == ",") {
			options.push_back(trimSpaceAndEol(option));
			option.erase();
		} else if (t.asInput() == "=") {
			option += '=';
			p.skip_spaces(true);
			if (p.next_token().asInput() == "{")
				option += '{' + p.getArg('{', '}') + '}';
		} else if (t.cat() != catSpace)
			option += t.asInput();
	}

	if (!option.empty())
		options.push_back(trimSpaceAndEol(option));

	return options;
}


/*!
 * Retrieve a keyval option "name={value with=sign}" named \p name from
 * \p options and return the value.
 * The found option is also removed from \p options.
 */
string process_keyval_opt(vector<string> & options, string name)
{
	for (size_t i = 0; i < options.size(); ++i) {
		vector<string> option;
		split(options[i], option, '=');
		if (option.size() < 2)
			continue;
		if (option[0] == name) {
			options.erase(options.begin() + i);
			option.erase(option.begin());
			return join(option, "=");
		}
	}
	return "";
}

} // anonymous namespace


bool Preamble::indentParagraphs() const
{
	return h_paragraph_separation == "indent";
}


bool Preamble::isPackageUsed(string const & package) const
{
	return used_packages.find(package) != used_packages.end();
}


vector<string> Preamble::getPackageOptions(string const & package) const
{
	map<string, vector<string> >::const_iterator it = used_packages.find(package);
	if (it != used_packages.end())
		return it->second;
	return vector<string>();
}


void Preamble::registerAutomaticallyLoadedPackage(std::string const & package)
{
	auto_packages.insert(package);
}


void Preamble::addModule(string const & module)
{
	used_modules.push_back(module);
}


void Preamble::suppressDate(bool suppress)
{
	if (suppress)
		h_suppress_date = "true";
	else
		h_suppress_date = "false";
}


void Preamble::registerAuthor(std::string const & name)
{
	Author author(from_utf8(name), empty_docstring());
	author.setUsed(true);
	authors_.record(author);
	h_tracking_changes = "true";
	h_output_changes = "true";
}


Author const & Preamble::getAuthor(std::string const & name) const
{
	Author author(from_utf8(name), empty_docstring());
	for (AuthorList::Authors::const_iterator it = authors_.begin();
	     it != authors_.end(); it++)
		if (*it == author)
			return *it;
	static Author const dummy;
	return dummy;
}


void Preamble::add_package(string const & name, vector<string> & options)
{
	// every package inherits the global options
	if (used_packages.find(name) == used_packages.end())
		used_packages[name] = split_options(h_options);

	vector<string> & v = used_packages[name];
	v.insert(v.end(), options.begin(), options.end());
	if (name == "jurabib") {
		// Don't output the order argument (see the cite command
		// handling code in text.cpp).
		vector<string>::iterator end =
			remove(options.begin(), options.end(), "natbiborder");
		end = remove(options.begin(), end, "jurabiborder");
		options.erase(end, options.end());
	}
}


namespace {

// Given is a string like "scaled=0.9", return 0.9 * 100
string const scale_as_percentage(string const & scale)
{
	string::size_type pos = scale.find('=');
	if (pos != string::npos) {
		string value = scale.substr(pos + 1);
		if (isStrDbl(value))
			return convert<string>(100 * convert<double>(value));
	}
	// If the input string didn't match our expectations.
	// return the default value "100"
	return "100";
}


string remove_braces(string const & value)
{
	if (value.empty())
		return value;
	if (value[0] == '{' && value[value.length()-1] == '}')
		return value.substr(1, value.length()-2);
	return value;
}

} // anonymous namespace


Preamble::Preamble() : one_language(true), title_layout_found(false)
{
	//h_backgroundcolor;
	//h_boxbgcolor;
	h_biblio_style            = "plain";
	h_cite_engine             = "basic";
	h_cite_engine_type        = "numerical";
	h_defskip                 = "medskip";
	//h_float_placement;
	//h_fontcolor;
	h_fontencoding            = "default";
	h_font_roman              = "default";
	h_font_sans               = "default";
	h_font_typewriter         = "default";
	h_font_default_family     = "default";
	h_use_non_tex_fonts       = "false";
	h_font_sc                 = "false";
	h_font_osf                = "false";
	h_font_sf_scale           = "100";
	h_font_tt_scale           = "100";
	h_graphics                = "default";
	h_html_be_strict          = "false";
	h_html_css_as_file        = "0";
	h_html_math_output        = "0";
	h_inputencoding           = "auto";
	h_justification           = "true";
	h_language                = "english";
	h_language_package        = "none";
	//h_listings_params;
	//h_margins;
	//h_notefontcolor;
	//h_options;
	h_output_changes          = "false";
	h_papercolumns            = "1";
	h_paperfontsize           = "default";
	h_paperorientation        = "portrait";
	h_paperpagestyle          = "default";
	//h_papersides;
	h_papersize               = "default";
	h_paragraph_indentation   = "default";
	h_paragraph_separation    = "indent";
	//h_pdf_title;
	//h_pdf_author;
	//h_pdf_subject;
	//h_pdf_keywords;
	h_pdf_bookmarks           = "1";
	h_pdf_bookmarksnumbered   = "0";
	h_pdf_bookmarksopen       = "0";
	h_pdf_bookmarksopenlevel  = "1";
	h_pdf_breaklinks          = "0";
	h_pdf_pdfborder           = "0";
	h_pdf_colorlinks          = "0";
	h_pdf_backref             = "section";
	h_pdf_pdfusetitle         = "1";
	//h_pdf_pagemode;
	//h_pdf_quoted_options;
	h_quotes_language         = "english";
	h_secnumdepth             = "3";
	h_spacing                 = "single";
	h_suppress_date           = "false";
	h_textclass               = "article";
	h_tocdepth                = "3";
	h_tracking_changes        = "false";
	h_use_bibtopic            = "false";
	h_use_indices             = "false";
	h_use_geometry            = "false";
	h_use_default_options     = "false";
	h_use_hyperref            = "0";
	h_use_refstyle            = "0";
	h_use_packages["amsmath"]    = "1";
	h_use_packages["esint"]      = "1";
	h_use_packages["mhchem"]     = "0";
	h_use_packages["mathdots"]   = "0";
	h_use_packages["mathtools"]  = "0";
	h_use_packages["undertilde"] = "0";
}


void Preamble::handle_hyperref(vector<string> & options)
{
	// FIXME swallow inputencoding changes that might surround the
	//       hyperref setup if it was written by LyX
	h_use_hyperref = "1";
	// swallow "unicode=true", since LyX does always write that
	vector<string>::iterator it =
		find(options.begin(), options.end(), "unicode=true");
	if (it != options.end())
		options.erase(it);
	it = find(options.begin(), options.end(), "pdfusetitle");
	if (it != options.end()) {
		h_pdf_pdfusetitle = "1";
		options.erase(it);
	}
	string bookmarks = process_keyval_opt(options, "bookmarks");
	if (bookmarks == "true")
		h_pdf_bookmarks = "1";
	else if (bookmarks == "false")
		h_pdf_bookmarks = "0";
	if (h_pdf_bookmarks == "1") {
		string bookmarksnumbered =
			process_keyval_opt(options, "bookmarksnumbered");
		if (bookmarksnumbered == "true")
			h_pdf_bookmarksnumbered = "1";
		else if (bookmarksnumbered == "false")
			h_pdf_bookmarksnumbered = "0";
		string bookmarksopen =
			process_keyval_opt(options, "bookmarksopen");
		if (bookmarksopen == "true")
			h_pdf_bookmarksopen = "1";
		else if (bookmarksopen == "false")
			h_pdf_bookmarksopen = "0";
		if (h_pdf_bookmarksopen == "1") {
			string bookmarksopenlevel =
				process_keyval_opt(options, "bookmarksopenlevel");
			if (!bookmarksopenlevel.empty())
				h_pdf_bookmarksopenlevel = bookmarksopenlevel;
		}
	}
	string breaklinks = process_keyval_opt(options, "breaklinks");
	if (breaklinks == "true")
		h_pdf_breaklinks = "1";
	else if (breaklinks == "false")
		h_pdf_breaklinks = "0";
	string pdfborder = process_keyval_opt(options, "pdfborder");
	if (pdfborder == "{0 0 0}")
		h_pdf_pdfborder = "1";
	else if (pdfborder == "{0 0 1}")
		h_pdf_pdfborder = "0";
	string backref = process_keyval_opt(options, "backref");
	if (!backref.empty())
		h_pdf_backref = backref;
	string colorlinks = process_keyval_opt(options, "colorlinks");
	if (colorlinks == "true")
		h_pdf_colorlinks = "1";
	else if (colorlinks == "false")
		h_pdf_colorlinks = "0";
	string pdfpagemode = process_keyval_opt(options, "pdfpagemode");
	if (!pdfpagemode.empty())
		h_pdf_pagemode = pdfpagemode;
	string pdftitle = process_keyval_opt(options, "pdftitle");
	if (!pdftitle.empty()) {
		h_pdf_title = remove_braces(pdftitle);
	}
	string pdfauthor = process_keyval_opt(options, "pdfauthor");
	if (!pdfauthor.empty()) {
		h_pdf_author = remove_braces(pdfauthor);
	}
	string pdfsubject = process_keyval_opt(options, "pdfsubject");
	if (!pdfsubject.empty())
		h_pdf_subject = remove_braces(pdfsubject);
	string pdfkeywords = process_keyval_opt(options, "pdfkeywords");
	if (!pdfkeywords.empty())
		h_pdf_keywords = remove_braces(pdfkeywords);
	if (!options.empty()) {
		if (!h_pdf_quoted_options.empty())
			h_pdf_quoted_options += ',';
		h_pdf_quoted_options += join(options, ",");
		options.clear();
	}
}


void Preamble::handle_geometry(vector<string> & options)
{
	h_use_geometry = "true";
	vector<string>::iterator it;
	// paper orientation
	if ((it = find(options.begin(), options.end(), "landscape")) != options.end()) {
		h_paperorientation = "landscape";
		options.erase(it);
	}
	// paper size
	// keyval version: "paper=letter"
	string paper = process_keyval_opt(options, "paper");
	if (!paper.empty())
		h_papersize = paper + "paper";
	// alternative version: "letterpaper"
	handle_opt(options, known_paper_sizes, h_papersize);
	delete_opt(options, known_paper_sizes);
	// page margins
	char const * const * margin = known_paper_margins;
	for (; *margin; ++margin) {
		string value = process_keyval_opt(options, *margin);
		if (!value.empty()) {
			int k = margin - known_paper_margins;
			string name = known_coded_paper_margins[k];
			h_margins += '\\' + name + ' ' + value + '\n';
		}
	}
}


void Preamble::handle_package(Parser &p, string const & name,
                              string const & opts, bool in_lyx_preamble)
{
	vector<string> options = split_options(opts);
	add_package(name, options);
	string scale;

	if (is_known(name, known_xetex_packages)) {
		xetex = true;
		h_use_non_tex_fonts = "true";
		if (h_inputencoding == "auto")
			p.setEncoding("utf8");
	}

	// roman fonts
	if (is_known(name, known_roman_fonts)) {
		h_font_roman = name;
		p.skip_spaces();
	}

	if (name == "fourier") {
		h_font_roman = "utopia";
		// when font uses real small capitals
		if (opts == "expert")
			h_font_sc = "true";
	}

	else if (name == "mathpazo")
		h_font_roman = "palatino";

	else if (name == "mathptmx")
		h_font_roman = "times";

	// sansserif fonts
	if (is_known(name, known_sans_fonts)) {
		h_font_sans = name;
		if (!opts.empty()) {
			scale = opts;
			h_font_sf_scale = scale_as_percentage(scale);
		}
	}

	// typewriter fonts
	if (is_known(name, known_typewriter_fonts)) {
		// fourier can be set as roman font _only_
		// fourier as typewriter is handled in handling of \ttdefault
		if (name != "fourier") {
			h_font_typewriter = name;
			if (!opts.empty()) {
				scale = opts;
				h_font_tt_scale = scale_as_percentage(scale);
			}
		}
	}

	// font uses old-style figure
	if (name == "eco")
		h_font_osf = "true";

	// after the detection and handling of special cases, we can remove the
	// fonts, otherwise they would appear in the preamble, see bug #7856
	if (is_known(name, known_roman_fonts) || is_known(name, known_sans_fonts)
		||	is_known(name, known_typewriter_fonts))
		;

	else if (name == "amsmath" || name == "amssymb")
		h_use_packages["amsmath"] = "2";

	else if (name == "esint" || name == "mhchem" || name == "mathdots" ||
	         name == "mathtools" || name == "undertilde")
		h_use_packages[name] = "2";

	else if (name == "babel") {
		h_language_package = "default";
		// One might think we would have to do nothing if babel is loaded
		// without any options to prevent pollution of the preamble with this
		// babel call in every roundtrip.
		// But the user could have defined babel-specific things afterwards. So
		// we need to keep it in the preamble to prevent cases like bug #7861.
		if (!opts.empty()) {
			// check if more than one option was used - used later for inputenc
			if (options.begin() != options.end() - 1)
				one_language = false;
			// babel takes the last language of the option of its \usepackage
			// call as document language. If there is no such language option, the
			// last language in the documentclass options is used.
			handle_opt(options, known_languages, h_language);
			// If babel is called with options, LyX puts them by default into the
			// document class options. This works for most languages, except
			// for Latvian, Lithuanian, Mongolian, Turkmen and Vietnamese and
			// perhaps in future others.
			// Therefore keep the babel call as it is as the user might have
			// reasons for it.
			h_preamble << "\\usepackage[" << opts << "]{babel}\n";
			delete_opt(options, known_languages);
		}
		else
			h_preamble << "\\usepackage{babel}\n";
	}

	else if (name == "fontenc") {
		h_fontencoding = getStringFromVector(options, ",");
		/* We could do the following for better round trip support,
		 * but this makes the document less portable, so I skip it:
		if (h_fontencoding == lyxrc.fontenc)
			h_fontencoding = "global";
		 */
		options.clear();
	}

	else if (name == "inputenc" || name == "luainputenc") {
		// h_inputencoding is only set when there is not more than one
		// inputenc option because otherwise h_inputencoding must be
		// set to "auto" (the default encoding of the document language)
		// Therefore check for the "," character.
		// It is also only set when there is not more than one babel
		// language option.
		if (opts.find(",") == string::npos && one_language == true)
			h_inputencoding = opts;
		if (!options.empty())
			p.setEncoding(options.back());
		options.clear();
	}

	else if (is_known(name, known_old_language_packages)) {
		// known language packages from the times before babel
		// if they are found and not also babel, they will be used as
		// custom language package
		h_language_package = "\\usepackage{" + name + "}";
	}

	else if (name == "prettyref")
		; // ignore this FIXME: Use the package separator mechanism instead

	else if (name == "lyxskak") {
		// ignore this and its options
		const char * const o[] = {"ps", "mover", 0};
		delete_opt(options, o);
	}

	else if (is_known(name, known_lyx_packages) && options.empty()) {
		if (name == "splitidx")
			h_use_indices = "true";
		if (!in_lyx_preamble)
			h_preamble << package_beg_sep << name
			           << package_mid_sep << "\\usepackage{"
			           << name << "}\n" << package_end_sep;
	}

	else if (name == "geometry")
		handle_geometry(options);

	else if (name == "subfig")
		; // ignore this FIXME: Use the package separator mechanism instead

	else if (is_known(name, known_languages))
		h_language = name;

	else if (name == "natbib") {
		h_biblio_style = "plainnat";
		h_cite_engine = "natbib";
		h_cite_engine_type = "authoryear";
		vector<string>::iterator it =
			find(options.begin(), options.end(), "authoryear");
		if (it != options.end())
			options.erase(it);
		else {
			it = find(options.begin(), options.end(), "numbers");
			if (it != options.end()) {
				h_cite_engine_type = "numerical";
				options.erase(it);
			}
		}
	}

	else if (name == "jurabib") {
		h_biblio_style = "jurabib";
		h_cite_engine = "jurabib";
		h_cite_engine_type = "authoryear";
	}

	else if (name == "hyperref")
		handle_hyperref(options);

	else if (!in_lyx_preamble) {
		if (options.empty())
			h_preamble << "\\usepackage{" << name << "}";
		else {
			h_preamble << "\\usepackage[" << opts << "]{"
				   << name << "}";
			options.clear();
		}
	}

	// We need to do something with the options...
	if (!options.empty())
		cerr << "Ignoring options '" << join(options, ",")
		     << "' of package " << name << '.' << endl;

	// remove the whitespace
	p.skip_spaces();
}


void Preamble::handle_if(Parser & p, bool in_lyx_preamble)
{
	while (p.good()) {
		Token t = p.get_token();
		if (t.cat() == catEscape &&
		    is_known(t.cs(), known_if_commands))
			handle_if(p, in_lyx_preamble);
		else {
			if (!in_lyx_preamble)
				h_preamble << t.asInput();
			if (t.cat() == catEscape && t.cs() == "fi")
				return;
		}
	}
}


bool Preamble::writeLyXHeader(ostream & os, bool subdoc)
{
	// translate from babel to LyX names
	h_language = babel2lyx(h_language);

	// set the quote language
	// LyX only knows the following quotes languages:
	// english, swedish, german, polish, french and danish
	// (quotes for "japanese" and "chinese-traditional" are missing because
	//  they wouldn't be useful: http://www.lyx.org/trac/ticket/6383)
	// conversion list taken from
	// http://en.wikipedia.org/wiki/Quotation_mark,_non-English_usage
	// (quotes for kazakh and interlingua are unknown)
	// danish
	if (h_language == "danish")
		h_quotes_language = "danish";
	// french
	else if (is_known(h_language, known_french_quotes_languages))
		h_quotes_language = "french";
	// german
	else if (is_known(h_language, known_german_quotes_languages))
		h_quotes_language = "german";
	// polish
	else if (is_known(h_language, known_polish_quotes_languages))
		h_quotes_language = "polish";
	// swedish
	else if (is_known(h_language, known_swedish_quotes_languages))
		h_quotes_language = "swedish";
	//english
	else if (is_known(h_language, known_english_quotes_languages))
		h_quotes_language = "english";

	if (contains(h_float_placement, "H"))
		registerAutomaticallyLoadedPackage("float");
	if (h_spacing != "single" && h_spacing != "default")
		registerAutomaticallyLoadedPackage("setspace");

	// output the LyX file settings
	os << "#LyX file created by tex2lyx " << PACKAGE_VERSION << "\n"
	   << "\\lyxformat " << LYX_FORMAT << '\n'
	   << "\\begin_document\n"
	   << "\\begin_header\n"
	   << "\\textclass " << h_textclass << "\n";
	string const raw = subdoc ? empty_string() : h_preamble.str();
	if (!raw.empty()) {
		os << "\\begin_preamble\n";
		for (string::size_type i = 0; i < raw.size(); ++i) {
			if (raw[i] == package_beg_sep) {
				// Here follows some package loading code that
				// must be skipped if the package is loaded
				// automatically.
				string::size_type j = raw.find(package_mid_sep, i);
				if (j == string::npos)
					return false;
				string::size_type k = raw.find(package_end_sep, j);
				if (k == string::npos)
					return false;
				string const package = raw.substr(i + 1, j - i - 1);
				string const replacement = raw.substr(j + 1, k - j - 1);
				if (auto_packages.find(package) == auto_packages.end())
					os << replacement;
				i = k;
			} else
				os.put(raw[i]);
		}
		os << "\n\\end_preamble\n";
	}
	if (!h_options.empty())
		os << "\\options " << h_options << "\n";
	os << "\\use_default_options " << h_use_default_options << "\n";
	if (!used_modules.empty()) {
		os << "\\begin_modules\n";
		vector<string>::const_iterator const end = used_modules.end();
		vector<string>::const_iterator it = used_modules.begin();
		for (; it != end; it++)
			os << *it << '\n';
		os << "\\end_modules\n";
	}
	os << "\\language " << h_language << "\n"
	   << "\\language_package " << h_language_package << "\n"
	   << "\\inputencoding " << h_inputencoding << "\n"
	   << "\\fontencoding " << h_fontencoding << "\n"
	   << "\\font_roman " << h_font_roman << "\n"
	   << "\\font_sans " << h_font_sans << "\n"
	   << "\\font_typewriter " << h_font_typewriter << "\n"
	   << "\\font_default_family " << h_font_default_family << "\n"
	   << "\\use_non_tex_fonts " << h_use_non_tex_fonts << "\n"
	   << "\\font_sc " << h_font_sc << "\n"
	   << "\\font_osf " << h_font_osf << "\n"
	   << "\\font_sf_scale " << h_font_sf_scale << "\n"
	   << "\\font_tt_scale " << h_font_tt_scale << "\n"
	   << "\\graphics " << h_graphics << "\n";
	if (!h_float_placement.empty())
		os << "\\float_placement " << h_float_placement << "\n";
	os << "\\paperfontsize " << h_paperfontsize << "\n"
	   << "\\spacing " << h_spacing << "\n"
	   << "\\use_hyperref " << h_use_hyperref << '\n';
	if (h_use_hyperref == "1") {
		if (!h_pdf_title.empty())
			os << "\\pdf_title \"" << h_pdf_title << "\"\n";
		if (!h_pdf_author.empty())
			os << "\\pdf_author \"" << h_pdf_author << "\"\n";
		if (!h_pdf_subject.empty())
			os << "\\pdf_subject \"" << h_pdf_subject << "\"\n";
		if (!h_pdf_keywords.empty())
			os << "\\pdf_keywords \"" << h_pdf_keywords << "\"\n";
		os << "\\pdf_bookmarks " << h_pdf_bookmarks << "\n"
		      "\\pdf_bookmarksnumbered " << h_pdf_bookmarksnumbered << "\n"
		      "\\pdf_bookmarksopen " << h_pdf_bookmarksopen << "\n"
		      "\\pdf_bookmarksopenlevel " << h_pdf_bookmarksopenlevel << "\n"
		      "\\pdf_breaklinks " << h_pdf_breaklinks << "\n"
		      "\\pdf_pdfborder " << h_pdf_pdfborder << "\n"
		      "\\pdf_colorlinks " << h_pdf_colorlinks << "\n"
		      "\\pdf_backref " << h_pdf_backref << "\n"
		      "\\pdf_pdfusetitle " << h_pdf_pdfusetitle << '\n';
		if (!h_pdf_pagemode.empty())
			os << "\\pdf_pagemode " << h_pdf_pagemode << '\n';
		if (!h_pdf_quoted_options.empty())
			os << "\\pdf_quoted_options \"" << h_pdf_quoted_options << "\"\n";
	}
	os << "\\papersize " << h_papersize << "\n"
	   << "\\use_geometry " << h_use_geometry << '\n';
	for (map<string, string>::const_iterator it = h_use_packages.begin();
	     it != h_use_packages.end(); it++)
		os << "\\use_package " << it->first << ' ' << it->second << '\n';
	os << "\\cite_engine " << h_cite_engine << '\n'
	   << "\\cite_engine_type " << h_cite_engine_type << '\n'
	   << "\\biblio_style " << h_biblio_style << "\n"
	   << "\\use_bibtopic " << h_use_bibtopic << "\n"
	   << "\\use_indices " << h_use_indices << "\n"
	   << "\\paperorientation " << h_paperorientation << '\n'
	   << "\\suppress_date " << h_suppress_date << '\n'
	   << "\\justification " << h_justification << '\n'
	   << "\\use_refstyle " << h_use_refstyle << '\n';
	if (!h_fontcolor.empty())
		os << "\\fontcolor " << h_fontcolor << '\n';
	if (!h_notefontcolor.empty())
		os << "\\notefontcolor " << h_notefontcolor << '\n';
	if (!h_backgroundcolor.empty())
		os << "\\backgroundcolor " << h_backgroundcolor << '\n';
	if (!h_boxbgcolor.empty())
		os << "\\boxbgcolor " << h_boxbgcolor << '\n';
	os << h_margins
	   << "\\secnumdepth " << h_secnumdepth << "\n"
	   << "\\tocdepth " << h_tocdepth << "\n"
	   << "\\paragraph_separation " << h_paragraph_separation << "\n";
	if (h_paragraph_separation == "skip")
		os << "\\defskip " << h_defskip << "\n";
	else
		os << "\\paragraph_indentation " << h_paragraph_indentation << "\n";
	os << "\\quotes_language " << h_quotes_language << "\n"
	   << "\\papercolumns " << h_papercolumns << "\n"
	   << "\\papersides " << h_papersides << "\n"
	   << "\\paperpagestyle " << h_paperpagestyle << "\n";
	if (!h_listings_params.empty())
		os << "\\listings_params " << h_listings_params << "\n";
	os << "\\tracking_changes " << h_tracking_changes << "\n"
	   << "\\output_changes " << h_output_changes << "\n"
	   << "\\html_math_output " << h_html_math_output << "\n"
	   << "\\html_css_as_file " << h_html_css_as_file << "\n"
	   << "\\html_be_strict " << h_html_be_strict << "\n"
	   << authors_
	   << "\\end_header\n\n"
	   << "\\begin_body\n";
	return true;
}


void Preamble::parse(Parser & p, string const & forceclass,
                     TeX2LyXDocClass & tc)
{
	// initialize fixed types
	special_columns['D'] = 3;
	bool is_full_document = false;
	bool is_lyx_file = false;
	bool in_lyx_preamble = false;

	// determine whether this is a full document or a fragment for inclusion
	while (p.good()) {
		Token const & t = p.get_token();

		if (t.cat() == catEscape && t.cs() == "documentclass") {
			is_full_document = true;
			break;
		}
	}
	p.reset();

	while (is_full_document && p.good()) {
		Token const & t = p.get_token();

#ifdef FILEDEBUG
		cerr << "t: " << t << "\n";
#endif

		//
		// cat codes
		//
		if (!in_lyx_preamble &&
		    (t.cat() == catLetter ||
		     t.cat() == catSuper ||
		     t.cat() == catSub ||
		     t.cat() == catOther ||
		     t.cat() == catMath ||
		     t.cat() == catActive ||
		     t.cat() == catBegin ||
		     t.cat() == catEnd ||
		     t.cat() == catAlign ||
		     t.cat() == catParameter))
			h_preamble << t.cs();

		else if (!in_lyx_preamble &&
			 (t.cat() == catSpace || t.cat() == catNewline))
			h_preamble << t.asInput();

		else if (t.cat() == catComment) {
			static regex const islyxfile("%% LyX .* created this file");
			static regex const usercommands("User specified LaTeX commands");

			string const comment = t.asInput();

			// magically switch encoding default if it looks like XeLaTeX
			static string const magicXeLaTeX =
				"% This document must be compiled with XeLaTeX ";
			if (comment.size() > magicXeLaTeX.size()
				  && comment.substr(0, magicXeLaTeX.size()) == magicXeLaTeX
				  && h_inputencoding == "auto") {
				cerr << "XeLaTeX comment found, switching to UTF8\n";
				h_inputencoding = "utf8";
			}
			smatch sub;
			if (regex_search(comment, sub, islyxfile)) {
				is_lyx_file = true;
				in_lyx_preamble = true;
			} else if (is_lyx_file
				   && regex_search(comment, sub, usercommands))
				in_lyx_preamble = false;
			else if (!in_lyx_preamble)
				h_preamble << t.asInput();
		}

		else if (t.cs() == "pagestyle")
			h_paperpagestyle = p.verbatim_item();

		else if (t.cs() == "date") {
			string argument = p.getArg('{', '}');
			if (argument.empty())
				h_suppress_date = "true";
			else
				h_preamble << t.asInput() << '{' << argument << '}';
		}

		else if (t.cs() == "color") {
			string const space =
				(p.hasOpt() ? p.getOpt() : string());
			string argument = p.getArg('{', '}');
			// check the case that a standard color is used
			if (space.empty() && is_known(argument, known_basic_colors)) {
				h_fontcolor = rgbcolor2code(argument);
				preamble.registerAutomaticallyLoadedPackage("color");
			} else if (space.empty() && argument == "document_fontcolor")
				preamble.registerAutomaticallyLoadedPackage("color");
			// check the case that LyX's document_fontcolor is defined
			// but not used for \color
			else {
				h_preamble << t.asInput();
				if (!space.empty())
					h_preamble << space;
				h_preamble << '{' << argument << '}';
				// the color might already be set because \definecolor
				// is parsed before this
				h_fontcolor = "";
			}
		}

		else if (t.cs() == "pagecolor") {
			string argument = p.getArg('{', '}');
			// check the case that a standard color is used
			if (is_known(argument, known_basic_colors)) {
				h_backgroundcolor = rgbcolor2code(argument);
			} else if (argument == "page_backgroundcolor")
				preamble.registerAutomaticallyLoadedPackage("color");
			// check the case that LyX's page_backgroundcolor is defined
			// but not used for \pagecolor
			else {
				h_preamble << t.asInput() << '{' << argument << '}';
				// the color might already be set because \definecolor
				// is parsed before this
				h_backgroundcolor = "";
			}
		}

		else if (t.cs() == "makeatletter") {
			// LyX takes care of this
			p.setCatCode('@', catLetter);
		}

		else if (t.cs() == "makeatother") {
			// LyX takes care of this
			p.setCatCode('@', catOther);
		}

		else if (t.cs() == "newcommand" || t.cs() == "newcommandx"
		      || t.cs() == "renewcommand" || t.cs() == "renewcommandx"
		      || t.cs() == "providecommand" || t.cs() == "providecommandx"
				|| t.cs() == "DeclareRobustCommand"
		      || t.cs() == "DeclareRobustCommandx"
				|| t.cs() == "ProvideTextCommandDefault"
				|| t.cs() == "DeclareMathAccent") {
			bool star = false;
			if (p.next_token().character() == '*') {
				p.get_token();
				star = true;
			}
			string const name = p.verbatim_item();
			string const opt1 = p.getFullOpt();
			string const opt2 = p.getFullOpt();
			string const body = p.verbatim_item();
			// font settings
			if (name == "\\rmdefault")
				if (is_known(body, known_roman_fonts))
					h_font_roman = body;
			if (name == "\\sfdefault")
				if (is_known(body, known_sans_fonts))
					h_font_sans = body;
			if (name == "\\ttdefault")
				if (is_known(body, known_typewriter_fonts))
					h_font_typewriter = body;
			if (name == "\\familydefault") {
				string family = body;
				// remove leading "\"
				h_font_default_family = family.erase(0,1);
			}

			// remove the lyxdot definition that is re-added by LyX
			// if necessary
			if (name == "\\lyxdot")
				in_lyx_preamble = true;

			// Add the command to the known commands
			add_known_command(name, opt1, !opt2.empty(), from_utf8(body));

			// only non-lyxspecific stuff
			if (!in_lyx_preamble) {
				ostringstream ss;
				ss << '\\' << t.cs();
				if (star)
					ss << '*';
				ss << '{' << name << '}' << opt1 << opt2
				   << '{' << body << "}";
				h_preamble << ss.str();
/*
				ostream & out = in_preamble ? h_preamble : os;
				out << "\\" << t.cs() << "{" << name << "}"
				    << opts << "{" << body << "}";
*/
			}
		}

		else if (t.cs() == "documentclass") {
			vector<string>::iterator it;
			vector<string> opts = split_options(p.getArg('[', ']'));
			handle_opt(opts, known_fontsizes, h_paperfontsize);
			delete_opt(opts, known_fontsizes);
			// delete "pt" at the end
			string::size_type i = h_paperfontsize.find("pt");
			if (i != string::npos)
				h_paperfontsize.erase(i);
			// The documentclass options are always parsed before the options
			// of the babel call so that a language cannot overwrite the babel
			// options.
			handle_opt(opts, known_languages, h_language);
			delete_opt(opts, known_languages);

			// paper orientation
			if ((it = find(opts.begin(), opts.end(), "landscape")) != opts.end()) {
				h_paperorientation = "landscape";
				opts.erase(it);
			}
			// paper sides
			if ((it = find(opts.begin(), opts.end(), "oneside"))
				 != opts.end()) {
				h_papersides = "1";
				opts.erase(it);
			}
			if ((it = find(opts.begin(), opts.end(), "twoside"))
				 != opts.end()) {
				h_papersides = "2";
				opts.erase(it);
			}
			// paper columns
			if ((it = find(opts.begin(), opts.end(), "onecolumn"))
				 != opts.end()) {
				h_papercolumns = "1";
				opts.erase(it);
			}
			if ((it = find(opts.begin(), opts.end(), "twocolumn"))
				 != opts.end()) {
				h_papercolumns = "2";
				opts.erase(it);
			}
			// paper sizes
			// some size options are known to any document classes, other sizes
			// are handled by the \geometry command of the geometry package
			handle_opt(opts, known_class_paper_sizes, h_papersize);
			delete_opt(opts, known_class_paper_sizes);
			// the remaining options
			h_options = join(opts, ",");
			// FIXME This does not work for classes that have a
			//       different name in LyX than in LaTeX
			h_textclass = p.getArg('{', '}');
		}

		else if (t.cs() == "usepackage") {
			string const options = p.getArg('[', ']');
			string const name = p.getArg('{', '}');
			vector<string> vecnames;
			split(name, vecnames, ',');
			vector<string>::const_iterator it  = vecnames.begin();
			vector<string>::const_iterator end = vecnames.end();
			for (; it != end; ++it)
				handle_package(p, trimSpaceAndEol(*it), options,
					       in_lyx_preamble);
		}

		else if (t.cs() == "inputencoding") {
			string const encoding = p.getArg('{','}');
			h_inputencoding = encoding;
			p.setEncoding(encoding);
		}

		else if (t.cs() == "newenvironment") {
			string const name = p.getArg('{', '}');
			string const opt1 = p.getFullOpt();
			string const opt2 = p.getFullOpt();
			string const beg = p.verbatim_item();
			string const end = p.verbatim_item();
			if (!in_lyx_preamble) {
				h_preamble << "\\newenvironment{" << name
				           << '}' << opt1 << opt2 << '{'
				           << beg << "}{" << end << '}';
			}
			add_known_environment(name, opt1, !opt2.empty(),
			                      from_utf8(beg), from_utf8(end));

		}

		else if (t.cs() == "def") {
			string name = p.get_token().cs();
			while (p.next_token().cat() != catBegin)
				name += p.get_token().cs();
			if (!in_lyx_preamble)
				h_preamble << "\\def\\" << name << '{'
					   << p.verbatim_item() << "}";
		}

		else if (t.cs() == "newcolumntype") {
			string const name = p.getArg('{', '}');
			trimSpaceAndEol(name);
			int nargs = 0;
			string opts = p.getOpt();
			if (!opts.empty()) {
				istringstream is(string(opts, 1));
				is >> nargs;
			}
			special_columns[name[0]] = nargs;
			h_preamble << "\\newcolumntype{" << name << "}";
			if (nargs)
				h_preamble << "[" << nargs << "]";
			h_preamble << "{" << p.verbatim_item() << "}";
		}

		else if (t.cs() == "setcounter") {
			string const name = p.getArg('{', '}');
			string const content = p.getArg('{', '}');
			if (name == "secnumdepth")
				h_secnumdepth = content;
			else if (name == "tocdepth")
				h_tocdepth = content;
			else
				h_preamble << "\\setcounter{" << name << "}{" << content << "}";
		}

		else if (t.cs() == "setlength") {
			string const name = p.verbatim_item();
			string const content = p.verbatim_item();
			// the paragraphs are only not indented when \parindent is set to zero
			if (name == "\\parindent" && content != "") {
				if (content[0] == '0')
					h_paragraph_separation = "skip";
				else
					h_paragraph_indentation = translate_len(content);
			} else if (name == "\\parskip") {
				if (content == "\\smallskipamount")
					h_defskip = "smallskip";
				else if (content == "\\medskipamount")
					h_defskip = "medskip";
				else if (content == "\\bigskipamount")
					h_defskip = "bigskip";
				else
					h_defskip = content;
			} else
				h_preamble << "\\setlength{" << name << "}{" << content << "}";
		}

		else if (t.cs() == "onehalfspacing")
			h_spacing = "onehalf";

		else if (t.cs() == "doublespacing")
			h_spacing = "double";

		else if (t.cs() == "setstretch")
			h_spacing = "other " + p.verbatim_item();

		else if (t.cs() == "begin") {
			string const name = p.getArg('{', '}');
			if (name == "document")
				break;
			h_preamble << "\\begin{" << name << "}";
		}

		else if (t.cs() == "geometry") {
			vector<string> opts = split_options(p.getArg('{', '}'));
			handle_geometry(opts);
		}

		else if (t.cs() == "definecolor") {
			string const color = p.getArg('{', '}');
			string const space = p.getArg('{', '}');
			string const value = p.getArg('{', '}');
			if (color == "document_fontcolor" && space == "rgb") {
				RGBColor c(RGBColorFromLaTeX(value));
				h_fontcolor = X11hexname(c);
			} else if (color == "note_fontcolor" && space == "rgb") {
				RGBColor c(RGBColorFromLaTeX(value));
				h_notefontcolor = X11hexname(c);
			} else if (color == "page_backgroundcolor" && space == "rgb") {
				RGBColor c(RGBColorFromLaTeX(value));
				h_backgroundcolor = X11hexname(c);
			} else if (color == "shadecolor" && space == "rgb") {
				RGBColor c(RGBColorFromLaTeX(value));
				h_boxbgcolor = X11hexname(c);
			} else {
				h_preamble << "\\definecolor{" << color
				           << "}{" << space << "}{" << value
				           << '}';
			}
		}

		else if (t.cs() == "bibliographystyle")
			h_biblio_style = p.verbatim_item();

		else if (t.cs() == "jurabibsetup") {
			// FIXME p.getArg('{', '}') is most probably wrong (it
			//       does not handle nested braces).
			//       Use p.verbatim_item() instead.
			vector<string> jurabibsetup =
				split_options(p.getArg('{', '}'));
			// add jurabibsetup to the jurabib package options
			add_package("jurabib", jurabibsetup);
			if (!jurabibsetup.empty()) {
				h_preamble << "\\jurabibsetup{"
					   << join(jurabibsetup, ",") << '}';
			}
		}

		else if (t.cs() == "hypersetup") {
			vector<string> hypersetup =
				split_options(p.verbatim_item());
			// add hypersetup to the hyperref package options
			handle_hyperref(hypersetup);
			if (!hypersetup.empty()) {
				h_preamble << "\\hypersetup{"
				           << join(hypersetup, ",") << '}';
			}
		}

		else if (is_known(t.cs(), known_if_3arg_commands)) {
			// prevent misparsing of \usepackage if it is used
			// as an argument (see e.g. our own output of
			// \@ifundefined above)
			string const arg1 = p.verbatim_item();
			string const arg2 = p.verbatim_item();
			string const arg3 = p.verbatim_item();
			// test case \@ifundefined{date}{}{\date{}}
			if (t.cs() == "@ifundefined" && arg1 == "date" &&
			    arg2.empty() && arg3 == "\\date{}") {
				h_suppress_date = "true";
			// older tex2lyx versions did output
			// \@ifundefined{definecolor}{\usepackage{color}}{}
			} else if (t.cs() == "@ifundefined" &&
			           arg1 == "definecolor" &&
			           arg2 == "\\usepackage{color}" &&
			           arg3.empty()) {
				if (!in_lyx_preamble)
					h_preamble << package_beg_sep
					           << "color"
					           << package_mid_sep
					           << "\\@ifundefined{definecolor}{color}{}"
					           << package_end_sep;
			// test for case
			//\@ifundefined{showcaptionsetup}{}{%
			// \PassOptionsToPackage{caption=false}{subfig}}
			// that LyX uses for subfloats
			} else if (t.cs() == "@ifundefined" &&
			           arg1 == "showcaptionsetup" && arg2.empty()
				&& arg3 == "%\n \\PassOptionsToPackage{caption=false}{subfig}") {
				; // do nothing
			} else if (!in_lyx_preamble) {
				h_preamble << t.asInput()
				           << '{' << arg1 << '}'
				           << '{' << arg2 << '}'
				           << '{' << arg3 << '}';
			}
		}

		else if (is_known(t.cs(), known_if_commands)) {
			// must not parse anything in conditional code, since
			// LyX would output the parsed contents unconditionally
			if (!in_lyx_preamble)
				h_preamble << t.asInput();
			handle_if(p, in_lyx_preamble);
		}

		else if (!t.cs().empty() && !in_lyx_preamble)
			h_preamble << '\\' << t.cs();
	}

	// remove the whitespace
	p.skip_spaces();

	// Force textclass if the user wanted it
	if (!forceclass.empty())
		h_textclass = forceclass;
	if (noweb_mode && !prefixIs(h_textclass, "literate-"))
		h_textclass.insert(0, "literate-");
	tc.setName(h_textclass);
	if (!tc.load()) {
		cerr << "Error: Could not read layout file for textclass \"" << h_textclass << "\"." << endl;
		exit(EXIT_FAILURE);
	}
	if (h_papersides.empty()) {
		ostringstream ss;
		ss << tc.sides();
		h_papersides = ss.str();
	}
}


string babel2lyx(string const & language)
{
	char const * const * where = is_known(language, known_languages);
	if (where)
		return known_coded_languages[where - known_languages];
	return language;
}


string rgbcolor2code(string const & name)
{
	char const * const * where = is_known(name, known_basic_colors);
	if (where) {
		// "red", "green" etc
		return known_basic_color_codes[where - known_basic_colors];
	}
	// "255,0,0", "0,255,0" etc
	RGBColor c(RGBColorFromLaTeX(name));
	return X11hexname(c);
}

// }])


} // namespace lyx
