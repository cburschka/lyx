/** The .tex to .lyx converter
    \author André Pönitz (2003)
 */

// {[(

#include <config.h>

#include "tex2lyx.h"

#include "layout.h"
#include "lyxtextclass.h"
#include "lyxlex.h"
#include "support/filetools.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

using std::cerr;
using std::endl;
using std::getline;
using std::istream;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::string;
using std::vector;

using lyx::support::LibFileSearch;

// special columntypes
extern std::map<char, int> special_columns;

namespace {

const char * known_languages[] = { "austrian", "babel", "bahasa", "basque",
"breton", "british", "bulgarian", "catalan", "croatian", "czech", "danish",
"dutch", "english", "esperanto", "estonian", "finnish", "francais",
"frenchb", "galician", "german", "germanb", "greek", "hebcal", "hebfont",
"hebrew", "hebrew_newcode", "hebrew_oldcode", "hebrew_p", "hyphen",
"icelandic", "irish", "italian", "latin", "lgrcmr", "lgrcmro", "lgrcmss",
"lgrcmtt", "lgrenc", "lgrlcmss", "lgrlcmtt", "lheclas", "lhecmr",
"lhecmss", "lhecmtt", "lhecrml", "lheenc", "lhefr", "lheredis", "lheshold",
"lheshscr", "lheshstk", "lsorbian", "magyar", "naustrian", "ngermanb",
"ngerman", "norsk", "polish", "portuges", "rlbabel", "romanian",
"russianb", "samin", "scottish", "serbian", "slovak", "slovene", "spanish",
"swedish", "turkish", "ukraineb", "usorbian", "welsh", 0};

char const * known_fontsizes[] = { "10pt", "11pt", "12pt", 0 };

// some ugly stuff
ostringstream h_preamble;
string h_textclass               = "article";
string h_options                 = string();
string h_language                = "english";
string h_inputencoding           = "latin1";
string h_fontscheme              = "default";
string h_graphics                = "default";
string h_paperfontsize           = "default";
string h_spacing                 = "single";
string h_papersize               = "default";
string h_paperpackage            = "default";
string h_use_geometry            = "0";
string h_use_amsmath             = "0";
string h_use_natbib              = "0";
string h_use_numerical_citations = "0";
string h_paperorientation        = "portrait";
string h_secnumdepth             = "3";
string h_tocdepth                = "3";
string h_paragraph_separation    = "indent";
string h_defskip                 = "medskip";
string h_quotes_language         = "english";
string h_quotes_times            = "2";
string h_papercolumns            = "1";
string h_papersides              = "1";
string h_paperpagestyle          = "default";
string h_tracking_changes        = "0";


void handle_opt(vector<string> & opts, char const ** what, string & target)
{
	if (opts.empty())
		return;

	for ( ; *what; ++what) {
		vector<string>::iterator it = find(opts.begin(), opts.end(), *what);
		if (it != opts.end()) {
			//cerr << "### found option '" << *what << "'\n";
			target = *what;
			opts.erase(it);
			return;
		}
	}
}


void handle_package(string const & name, string const & options)
{
	//cerr << "handle_package: '" << name << "'\n";
	if (name == "a4wide") {
		h_papersize = "a4paper";
		h_paperpackage = "widemarginsa4";
	} else if (name == "ae")
		h_fontscheme = "ae";
	else if (name == "aecompl")
		h_fontscheme = "ae";
	else if (name == "amsmath")
		h_use_amsmath = "1";
	else if (name == "amssymb")
		h_use_amsmath = "1";
	else if (name == "babel")
		; // ignore this
	else if (name == "fontenc")
		; // ignore this
	else if (name == "inputenc")
		h_inputencoding = options;
	else if (name == "makeidx")
		; // ignore this
	else if (name == "verbatim")
		; // ignore this
	else if (is_known(name, known_languages)) {
		h_language = name;
		h_quotes_language = name;
	} else {
		if (options.size())
			h_preamble << "\\usepackage[" << options << "]{" << name << "}\n";
		else
			h_preamble << "\\usepackage{" << name << "}\n";
	}
}



void end_preamble(ostream & os, LyXTextClass const & /*textclass*/)
{
	os << "# tex2lyx 0.1.0 created this file\n"
	   << "\\lyxformat 225\n"
	   << "\\textclass " << h_textclass << "\n"
	   << "\\begin_preamble\n" << h_preamble.str() << "\n\\end_preamble\n";
	if (h_options.size())
	   os << "\\options " << h_options << "\n";
	os << "\\language " << h_language << "\n"
	   << "\\inputencoding " << h_inputencoding << "\n"
	   << "\\fontscheme " << h_fontscheme << "\n"
	   << "\\graphics " << h_graphics << "\n"
	   << "\\paperfontsize " << h_paperfontsize << "\n"
	   << "\\spacing " << h_spacing << "\n"
	   << "\\papersize " << h_papersize << "\n"
	   << "\\paperpackage " << h_paperpackage << "\n"
	   << "\\use_geometry " << h_use_geometry << "\n"
	   << "\\use_amsmath " << h_use_amsmath << "\n"
	   << "\\use_natbib " << h_use_natbib << "\n"
	   << "\\use_numerical_citations " << h_use_numerical_citations << "\n"
	   << "\\paperorientation " << h_paperorientation << "\n"
	   << "\\secnumdepth " << h_secnumdepth << "\n"
	   << "\\tocdepth " << h_tocdepth << "\n"
	   << "\\paragraph_separation " << h_paragraph_separation << "\n"
	   << "\\defskip " << h_defskip << "\n"
	   << "\\quotes_language " << h_quotes_language << "\n"
	   << "\\quotes_times " << h_quotes_times << "\n"
	   << "\\papercolumns " << h_papercolumns << "\n"
	   << "\\papersides " << h_papersides << "\n"
	   << "\\paperpagestyle " << h_paperpagestyle << "\n"
	   << "\\tracking_changes " << h_tracking_changes << "\n"
	   << "\\end_header";
}

} // anonymous namespace

LyXTextClass const parse_preamble(Parser & p, ostream & os)
{
	// initialize fixed types
	special_columns['D'] = 3;

	while (p.good()) {
		Token const & t = p.get_token();

#ifdef FILEDEBUG
		cerr << "t: " << t << " flags: " << flags << "\n";
		//cell->dump();
#endif

		//
		// cat codes
		//
		if (t.cat() == catLetter ||
			  t.cat() == catSpace ||
			  t.cat() == catSuper ||
			  t.cat() == catSub ||
			  t.cat() == catOther ||
			  t.cat() == catMath ||
			  t.cat() == catActive ||
			  t.cat() == catBegin ||
			  t.cat() == catEnd ||
			  t.cat() == catAlign ||
			  t.cat() == catNewline ||
			  t.cat() == catParameter)
		h_preamble << t.character();

		else if (t.cat() == catComment)
			handle_comment(p);

		else if (t.cs() == "pagestyle")
			h_paperpagestyle == p.verbatim_item();

		else if (t.cs() == "makeatletter") {
			p.setCatCode('@', catLetter);
			h_preamble << "\\makeatletter\n";
		}

		else if (t.cs() == "makeatother") {
			p.setCatCode('@', catOther);
			h_preamble << "\\makeatother\n";
		}

		else if (t.cs() == "newcommand" || t.cs() == "renewcommand"
			    || t.cs() == "providecommand") {
			bool star = false;
			if (p.next_token().character() == '*') {
				p.get_token();
				star = true;
			}
			string const name = p.verbatim_item();
			string const opts = p.getOpt();
			string const body = p.verbatim_item();
			// only non-lyxspecific stuff
			if (name != "\\noun "
				  && name != "\\tabularnewline "
			    && name != "\\LyX "
				  && name != "\\lyxline "
				  && name != "\\lyxaddress "
				  && name != "\\lyxrightaddress "
				  && name != "\\boldsymbol "
				  && name != "\\lyxarrow ") {
				ostringstream ss;
				ss << '\\' << t.cs();
				if (star)
					ss << '*';
				ss << '{' << name << '}' << opts << '{' << body << "}\n";
				h_preamble << ss.str();
/*
				ostream & out = in_preamble ? h_preamble : os;
				out << "\\" << t.cs() << "{" << name << "}"
				    << opts << "{" << body << "}\n";
*/
			}
		}

		else if (t.cs() == "documentclass") {
			vector<string> opts;
			split(p.getArg('[', ']'), opts, ',');
			handle_opt(opts, known_languages, h_language);
			handle_opt(opts, known_fontsizes, h_paperfontsize);
			h_quotes_language = h_language;
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
			ss << '\n';
			if (name != "lyxcode" && name != "lyxlist"
					&& name != "lyxrightadress" && name != "lyxaddress")
				h_preamble << ss.str();
		}

		else if (t.cs() == "def") {
			string name = p.get_token().cs();
			while (p.next_token().cat() != catBegin)
				name += p.get_token().asString();
			h_preamble << "\\def\\" << name << '{' << p.verbatim_item() << "}\n";
		}

		else if (t.cs() == "newcolumntype") {
			string const name = p.getArg('{', '}');
			trim(name);
			int nargs = 0;
			string opts = p.getOpt();
			if (opts.size()) {
				istringstream is(string(opts, 1));
				//cerr << "opt: " << is.str() << "\n";
				is >> nargs;
			}
			special_columns[name[0]] = nargs;
			h_preamble << "\\newcolumntype{" << name << "}";
			if (nargs)
				h_preamble << "[" << nargs << "]";
			h_preamble << "{" << p.verbatim_item() << "}\n";
		}

		else if (t.cs() == "setcounter") {
			string const name = p.getArg('{', '}');
			string const content = p.getArg('{', '}');
			if (name == "secnumdepth")
				h_secnumdepth = content;
			else if (name == "tocdepth")
				h_tocdepth = content;
			else
				h_preamble << "\\setcounter{" << name << "}{" << content << "}\n";
		}

		else if (t.cs() == "setlength") {
			string const name = p.verbatim_item();
			string const content = p.verbatim_item();
			if (name == "parskip")
				h_paragraph_separation = "skip";
			else if (name == "parindent")
				h_paragraph_separation = "skip";
			else
				h_preamble << "\\setlength{" << name << "}{" << content << "}\n";
		}

		else if (t.cs() == "par")
			h_preamble << '\n';

		else if (t.cs() == "begin") {
			string const name = p.getArg('{', '}');
			if (name == "document")
				break;
			h_preamble << "\\begin{" << name << "}";
		}

		else if (t.cs().size())
			h_preamble << '\\' << t.cs() << ' ';
	}

	LyXTextClass textclass;
	textclass.Read(LibFileSearch("layouts", h_textclass, "layout"));
	end_preamble(os, textclass);
	return textclass;
}

// }])
