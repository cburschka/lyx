/**
 * \file preamble.cpp
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

#include "tex2lyx.h"

#include "LayoutFile.h"
#include "Layout.h"
#include "Lexer.h"
#include "TextClass.h"

#include "support/convert.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/lstrings.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace lyx::support;

namespace lyx {

// special columntypes
extern map<char, int> special_columns;

map<string, vector<string> > used_packages;

// needed to handle encodings with babel
bool one_language = true;

// to avoid that the babel options overwrite the documentclass options
bool documentclass_language;

namespace {

const char * const known_languages[] = { "afrikaans", "american", "arabic",
"austrian", "bahasa", "basque", "belarusian", "brazil", "breton", "british",
"bulgarian", "canadian", "canadien", "catalan", "croatian", "czech", "danish",
"dutch", "english", "esperanto", "estonian", "finnish", "francais", "french",
"frenchb", "frenchle", "frenchpro", "galician", "german", "germanb", "greek",
"hebrew", "icelandic", "irish", "italian", "lsorbian", "magyar", "naustrian",
"ngerman", "ngermanb", "norsk", "nynorsk", "polish", "portuges", "romanian",
"russian", "russianb", "scottish", "serbian", "slovak", "slovene", "spanish",
"swedish", "thai", "turkish", "ukraineb", "ukrainian", "usorbian", "welsh", 0};

//note this when updating to lyxformat 305:
//bahasai, indonesian, and indon = equal to bahasa
//malay, and meyalu = equal to bahasam

const char * const known_french_languages[] = {"french", "frenchb", "francais",
						"frenchle", "frenchpro", 0};
const char * const known_german_languages[] = {"german", "germanb", 0};
const char * const known_ngerman_languages[] = {"ngerman", "ngermanb", 0};
const char * const known_russian_languages[] = {"russian", "russianb", 0};
const char * const known_ukrainian_languages[] = {"ukrainian", "ukraineb", 0};

char const * const known_fontsizes[] = { "10pt", "11pt", "12pt", 0 };

const char * const known_roman_fonts[] = { "ae", "bookman", "charter",
"cmr", "fourier", "lmodern", "mathpazo", "mathptmx", "newcent", 0};

const char * const known_sans_fonts[] = { "avant", "berasans", "cmbr", "cmss",
"helvet", "lmss", 0};

const char * const known_typewriter_fonts[] = { "beramono", "cmtl", "cmtt",
"courier", "lmtt", "luximono", "fourier", "lmodern", "mathpazo", "mathptmx",
"newcent", 0};

// some ugly stuff
ostringstream h_preamble;
string h_textclass               = "article";
string h_options                 = string();
string h_language                = "english";
string h_inputencoding           = "auto";
string h_font_roman              = "default";
string h_font_sans               = "default";
string h_font_typewriter         = "default";
string h_font_default_family     = "default";
string h_font_sc                 = "false";
string h_font_osf                = "false";
string h_font_sf_scale           = "100";
string h_font_tt_scale           = "100";
string h_graphics                = "default";
string h_paperfontsize           = "default";
string h_spacing                 = "single";
string h_papersize               = "default";
string h_use_geometry            = "false";
string h_use_amsmath             = "0";
string h_cite_engine             = "basic";
string h_use_bibtopic            = "false";
string h_paperorientation        = "portrait";
string h_secnumdepth             = "3";
string h_tocdepth                = "3";
string h_paragraph_separation    = "indent";
string h_defskip                 = "medskip";
string h_quotes_language         = "english";
string h_papercolumns            = "1";
string h_papersides              = string();
string h_paperpagestyle          = "default";
string h_tracking_changes        = "false";
string h_output_changes          = "false";


void handle_opt(vector<string> & opts, char const * const * what, string & target)
{
	if (opts.empty())
		return;

	// the last language option is the document language (for babel and LyX)
	// the last size option is the document font size
	vector<string>::iterator it;
	vector<string>::iterator position = opts.begin();
	for (; *what; ++what) {
		it = find(opts.begin(), opts.end(), *what);
		if (it != opts.end()) {
			documentclass_language = true;
			if (it >= position) {
				target = *what;
				position = it;
			}
		}
	}
}


void delete_opt(vector<string> & opts, char const * const * what)
{
	if (opts.empty())
		return;

	// remove found options from the list
	// do this after handle_opt to avoid potential memory leaks and to be able
	// to find in every case the last language option
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
			options.push_back(trim(option));
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
		options.push_back(trim(option));

	return options;
}


/*!
 * Add package \p name with options \p options to used_packages.
 * Remove options from \p options that we don't want to output.
 */
void add_package(string const & name, vector<string> & options)
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


void handle_package(string const & name, string const & opts)
{
	vector<string> options = split_options(opts);
	add_package(name, options);
	string scale;

	// roman fonts
	if (is_known(name, known_roman_fonts))
		h_font_roman = name;

	if (name == "fourier") {
		h_font_roman = "utopia";
		// when font uses real small capitals
		if (opts == "expert")
			h_font_sc = "true";
	}
	if (name == "mathpazo")
		h_font_roman = "palatino";

	if (name == "mathptmx")
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
		h_font_typewriter = name;
		if (!opts.empty()) {
			scale = opts;
			h_font_tt_scale = scale_as_percentage(scale);
		}
	}
	// font uses old-style figure
	if (name == "eco")
		h_font_osf = "true";

	else if (name == "amsmath" || name == "amssymb")
		h_use_amsmath = "1";

	else if (name == "babel" && !opts.empty()) {
		// check if more than one option was used - used later for inputenc
		// in case inputenc is parsed before babel, set the encoding to auto
		if (options.begin() != options.end() - 1) {
			one_language = false;
			h_inputencoding = "auto";
		}
		// only set the document language when there was not already one set
		// via the documentclass options
		// babel takes the the last language given in the documentclass options
		// as document language. If there is no such language option, the last
		// option of its \usepackage call is used.
		if (documentclass_language == false) {
			handle_opt(options, known_languages, h_language);
			delete_opt(options, known_languages);
			if (is_known(h_language, known_french_languages))
				h_language = "french";
			else if (is_known(h_language, known_german_languages))
				h_language = "german";
			else if (is_known(h_language, known_ngerman_languages))
				h_language = "ngerman";
			else if (is_known(h_language, known_russian_languages))
				h_language = "russian";
			else if (is_known(h_language, known_ukrainian_languages))
				h_language = "ukrainian";
			h_quotes_language = h_language;
		}
	}
	else if (name == "fontenc")
		; // ignore this

	else if (name == "inputenc") {
		// only set when there is not more than one inputenc option
		// therefore check for the "," character
		// also only set when there is not more then one babel language option
		if (opts.find(",") == string::npos && one_language == true)
			if (opts == "ascii")
				//change ascii to auto to be in the unicode range, see
				//http://bugzilla.lyx.org/show_bug.cgi?id=4719
				h_inputencoding = "auto";
			else
				h_inputencoding = opts;
		options.clear();
	}
	else if (name == "makeidx")
		; // ignore this

	else if (name == "verbatim")
		; // ignore this

	else if (name == "color")
		// with the following command this package is only loaded when needed for
		// undefined colors, since we only support the predefined colors
		h_preamble << "\\@ifundefined{definecolor}\n {\\usepackage{color}}{}\n";

	else if (name == "graphicx")
		; // ignore this

	else if (name == "setspace")
		; // ignore this

	else if (is_known(name, known_languages)) {
		if (is_known(name, known_french_languages))
			h_language = "french";
		else if (is_known(name, known_german_languages))
			h_language = "german";
		else if (is_known(name, known_ngerman_languages))
			h_language = "ngerman";
		else if (is_known(name, known_russian_languages))
			h_language = "russian";
		else if (is_known(name, known_ukrainian_languages))
			h_language = "ukrainian";
		else
			h_language = name;
		h_quotes_language = h_language;
	}
	else if (name == "natbib") {
		h_cite_engine = "natbib_authoryear";
		vector<string>::iterator it =
			find(options.begin(), options.end(), "authoryear");
		if (it != options.end())
			options.erase(it);
		else {
			it = find(options.begin(), options.end(), "numbers");
			if (it != options.end()) {
				h_cite_engine = "natbib_numerical";
				options.erase(it);
			}
		}
	}
	else if (name == "jurabib")
		h_cite_engine = "jurabib";

	else if (options.empty())
		h_preamble << "\\usepackage{" << name << "}\n";
	else {
		h_preamble << "\\usepackage[" << opts << "]{" << name << "}\n";
		options.clear();
	}
	// We need to do something with the options...
	if (!options.empty())
		cerr << "Ignoring options '" << join(options, ",")
		     << "' of package " << name << '.' << endl;
}



void end_preamble(ostream & os, TextClass const & /*textclass*/)
{
	os << "#LyX file created by tex2lyx " << PACKAGE_VERSION << "\n"
	   << "\\lyxformat 247\n"
	   << "\\begin_document\n"
	   << "\\begin_header\n"
	   << "\\textclass " << h_textclass << "\n";
	if (!h_preamble.str().empty())
		os << "\\begin_preamble\n" << h_preamble.str() << "\n\\end_preamble\n";
	if (!h_options.empty())
		os << "\\options " << h_options << "\n";
	os << "\\language " << h_language << "\n"
	   << "\\inputencoding " << h_inputencoding << "\n"
	   << "\\font_roman " << h_font_roman << "\n"
	   << "\\font_sans " << h_font_sans << "\n"
	   << "\\font_typewriter " << h_font_typewriter << "\n"
	   << "\\font_default_family " << h_font_default_family << "\n"
	   << "\\font_sc " << h_font_sc << "\n"
	   << "\\font_osf " << h_font_osf << "\n"
	   << "\\font_sf_scale " << h_font_sf_scale << "\n"
	   << "\\font_tt_scale " << h_font_tt_scale << "\n"
	   << "\\graphics " << h_graphics << "\n"
	   << "\\paperfontsize " << h_paperfontsize << "\n"
	   << "\\spacing " << h_spacing << "\n"
	   << "\\papersize " << h_papersize << "\n"
	   << "\\use_geometry " << h_use_geometry << "\n"
	   << "\\use_amsmath " << h_use_amsmath << "\n"
	   << "\\cite_engine " << h_cite_engine << "\n"
	   << "\\use_bibtopic " << h_use_bibtopic << "\n"
	   << "\\paperorientation " << h_paperorientation << "\n"
	   << "\\secnumdepth " << h_secnumdepth << "\n"
	   << "\\tocdepth " << h_tocdepth << "\n"
	   << "\\paragraph_separation " << h_paragraph_separation << "\n"
	   << "\\defskip " << h_defskip << "\n"
	   << "\\quotes_language " << h_quotes_language << "\n"
	   << "\\papercolumns " << h_papercolumns << "\n"
	   << "\\papersides " << h_papersides << "\n"
	   << "\\paperpagestyle " << h_paperpagestyle << "\n"
	   << "\\tracking_changes " << h_tracking_changes << "\n"
	   << "\\output_changes " << h_output_changes << "\n"
	   << "\\end_header\n\n"
	   << "\\begin_body\n";
	// clear preamble for subdocuments
	h_preamble.str("");
}

} // anonymous namespace

void parse_preamble(Parser & p, ostream & os, 
	string const & forceclass, TeX2LyXDocClass & tc)
{
	// initialize fixed types
	special_columns['D'] = 3;
	bool is_full_document = false;

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
		if (t.cat() == catLetter ||
			  t.cat() == catSuper ||
			  t.cat() == catSub ||
			  t.cat() == catOther ||
			  t.cat() == catMath ||
			  t.cat() == catActive ||
			  t.cat() == catBegin ||
			  t.cat() == catEnd ||
			  t.cat() == catAlign ||
			  t.cat() == catParameter)
		h_preamble << t.character();

		else if (t.cat() == catSpace || t.cat() == catNewline)
			h_preamble << t.asInput();

		else if (t.cat() == catComment)
			h_preamble << t.asInput();

		else if (t.cs() == "pagestyle")
			h_paperpagestyle = p.verbatim_item();

		else if (t.cs() == "makeatletter") {
			p.setCatCode('@', catLetter);
		}

		else if (t.cs() == "makeatother") {
			p.setCatCode('@', catOther);
		}

		else if (t.cs() == "newcommand" || t.cs() == "renewcommand"
			    || t.cs() == "providecommand") {
			bool star = false;
			if (p.next_token().character() == '*') {
				p.get_token();
				star = true;
			}
			string const name = p.verbatim_item();
			string const opt1 = p.getOpt();
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
			// only non-lyxspecific stuff
			if (   name != "\\noun"
			    && name != "\\tabularnewline"
			    && name != "\\LyX"
			    && name != "\\lyxline"
			    && name != "\\lyxaddress"
			    && name != "\\lyxrightaddress"
			    && name != "\\lyxdot"
			    && name != "\\boldsymbol"
			    && name != "\\lyxarrow"
			    && name != "\\rmdefault"
			    && name != "\\sfdefault"
			    && name != "\\ttdefault"
			    && name != "\\familydefault") {
				ostringstream ss;
				ss << '\\' << t.cs();
				if (star)
					ss << '*';
				ss << '{' << name << '}' << opt1 << opt2
				   << '{' << body << "}";
				h_preamble << ss.str();

				// Add the command to the known commands
				add_known_command(name, opt1, !opt2.empty());
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
			// to avoid that the babel options overwrite the documentclass options
			documentclass_language = false;
			handle_opt(opts, known_languages, h_language);
			delete_opt(opts, known_languages);
			if (is_known(h_language, known_french_languages))
				h_language = "french";
			else if (is_known(h_language, known_german_languages))
				h_language = "german";
			else if (is_known(h_language, known_ngerman_languages))
				h_language = "ngerman";
			else if (is_known(h_language, known_russian_languages))
				h_language = "russian";
			else if (is_known(h_language, known_ukrainian_languages))
				h_language = "ukrainian";
			h_quotes_language = h_language;
			// paper sides
			if ((it = find(opts.begin(), opts.end(), "twoside")) != opts.end()) {
				h_papersides = "2";
				opts.erase(it);
			}
			// paper columns
			if ((it = find(opts.begin(), opts.end(), "twocolumn")) != opts.end()) {
				h_papercolumns = "2";
				opts.erase(it);
			}
			h_options = join(opts, ",");
			h_textclass = p.getArg('{', '}');
		}

		else if (t.cs() == "usepackage") {
			string const options = p.getArg('[', ']');
			string const name = p.getArg('{', '}');
			if (options.empty() && name.find(',')) {
				vector<string> vecnames;
				split(name, vecnames, ',');
				vector<string>::const_iterator it  = vecnames.begin();
				vector<string>::const_iterator end = vecnames.end();
				for (; it != end; ++it)
					handle_package(trim(*it), string());
			} else {
				handle_package(name, options);
			}
		}

		else if (t.cs() == "newenvironment") {
			string const name = p.getArg('{', '}');
			ostringstream ss;
			ss << "\\newenvironment{" << name << "}";
			ss << p.getOpt();
			ss << p.getOpt();
			ss << '{' << p.verbatim_item() << '}';
			ss << '{' << p.verbatim_item() << '}';
			if (name != "lyxcode" && name != "lyxlist" &&
			    name != "lyxrightadress" &&
			    name != "lyxaddress" && name != "lyxgreyedout")
				h_preamble << ss.str();
		}

		else if (t.cs() == "def") {
			string name = p.get_token().cs();
			while (p.next_token().cat() != catBegin)
				name += p.get_token().asString();
			h_preamble << "\\def\\" << name << '{'
				   << p.verbatim_item() << "}";
		}

		else if (t.cs() == "newcolumntype") {
			string const name = p.getArg('{', '}');
			trim(name);
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

		else if (t.cs() =="onehalfspacing")
			h_spacing = "onehalf";

		else if (t.cs() =="doublespacing")
			h_spacing = "double";

		else if (t.cs() =="setstretch")
			h_spacing = "other " + p.verbatim_item();

		else if (t.cs() == "begin") {
			string const name = p.getArg('{', '}');
			if (name == "document")
				break;
			h_preamble << "\\begin{" << name << "}";
		}

		else if (t.cs() == "jurabibsetup") {
			vector<string> jurabibsetup =
				split_options(p.getArg('{', '}'));
			// add jurabibsetup to the jurabib package options
			add_package("jurabib", jurabibsetup);
			if (!jurabibsetup.empty()) {
				h_preamble << "\\jurabibsetup{"
					   << join(jurabibsetup, ",") << '}';
			}
		}

		else if (!t.cs().empty())
			h_preamble << '\\' << t.cs();
	}
	p.skip_spaces();

	// Force textclass if the user wanted it
	if (!forceclass.empty())
		h_textclass = forceclass;
	if (noweb_mode && !prefixIs(h_textclass, "literate-"))
		h_textclass.insert(0, "literate-");
	FileName layoutfilename = libFileSearch("layouts", h_textclass, "layout");
	if (layoutfilename.empty()) {
		cerr << "Error: Could not find layout file for textclass \"" << h_textclass << "\"." << endl;
		exit(1);
	}
	tc.read(layoutfilename);
	if (h_papersides.empty()) {
		ostringstream ss;
		ss << tc.sides();
		h_papersides = ss.str();
	}
	end_preamble(os, tc);
}

// }])


} // namespace lyx
