/** The .tex to .lyx converter
    \author André Pönitz (2003)
 */

// {[(

#include <config.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include "texparser.h"

using std::cout;
using std::cerr;
using std::endl;
using std::fill;
using std::getline;
using std::ios;
using std::ifstream;
using std::istream;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::stack;
using std::string;
using std::vector;


namespace {

char const OPEN = '<';
char const CLOSE = '>';

const char * known_languages[] = { "austrian", "babel", "bahasa",
"basque", "breton", "bulgarian", "catalan", "croatian", "czech", "danish",
"dutch", "english", "esperanto", "estonian", "finnish", "francais",
"frenchb", "galician", "germanb", "greek", "hebcal", "hebfont", "hebrew",
"hebrew_newcode", "hebrew_oldcode", "hebrew_p", "hyphen", "icelandic",
"irish", "italian", "latin", "lgrcmr", "lgrcmro", "lgrcmss", "lgrcmtt",
"lgrenc", "lgrlcmss", "lgrlcmtt", "lheclas", "lhecmr", "lhecmss",
"lhecmtt", "lhecrml", "lheenc", "lhefr", "lheredis", "lheshold",
"lheshscr", "lheshstk", "lsorbian", "magyar", "naustrian", "ngermanb",
"ngerman", "norsk", "polish", "portuges", "rlbabel", "romanian",
"russianb", "samin", "scottish", "serbian", "slovak", "slovene", "spanish",
"swedish", "turkish", "ukraineb", "usorbian", "welsh", 0};

const char * known_fontsizes[] = { "10pt", "11pt", "12pt", 0 };


// some ugly stuff
ostringstream h_preamble;
string h_textclass               = "FIXME";
string h_options                 = "FIXME";
string h_language                = "FIXME";
string h_inputencoding           = "FIXME";
string h_fontscheme              = "FIXME";
string h_graphics                = "default";
string h_paperfontsize           = "FIXME";
string h_spacing                 = "single";
string h_papersize               = "FIXME";
string h_paperpackage            = "FIXME";
string h_use_geometry            = "0";
string h_use_amsmath             = "0";
string h_use_natbib              = "0";
string h_use_numerical_citations = "0";
string h_paperorientation        = "portrait";
string h_secnumdepth             = "3";
string h_tocdepth                = "3";
string h_paragraph_separation    = "indent";
string h_defskip                 = "medskip";
string h_quotes_language         = "FIXME";
string h_quotes_times            = "1";
string h_papercolumns            = "1";
string h_papersides              = "1";
string h_paperpagestyle          = "default";
string h_tracking_changes        = "0";

// indicates whether we are in the preamble
bool in_preamble = true;

// current stack of nested environments
stack<string> active_environments;



string const trim(string const & a, char const * p = " ")
{
	// lyx::Assert(p);

	if (a.empty() || !*p)
		return a;

	string::size_type r = a.find_last_not_of(p);
	string::size_type l = a.find_first_not_of(p);

	// Is this the minimal test? (lgb)
	if (r == string::npos && l == string::npos)
		return string();

	return a.substr(l, r - l + 1);
}


void split(string const & s, vector<string> & result, char delim)
{
	istringstream is(s);	
	string t;
	while (getline(is, t, delim))
		result.push_back(t);
}


string join(vector<string> const & input, char delim)
{
	ostringstream os;
	for (size_t i = 0; i != input.size(); ++i) {
		if (i)
			os << delim;	
		os << input[i]; 
	}
	return os.str();
}


void handle_opt(vector<string> & opts, char const ** what, string & target)
{
	if (opts.empty())
		return;

	for ( ; what; ++what) {
		vector<string>::iterator it = find(opts.begin(), opts.end(), *what);
		if (it != opts.end()) {
			//cerr << "### found option '" << *what << "'\n";
			target = *what;
			opts.erase(it);
			return;
		}
	}
}


void begin_inset(ostream & os, string const & name)
{
	os << "\n\\begin_inset " << name;
}


void end_inset(ostream & os)
{
	os << "\n\\end_inset\n";
}


void handle_ert(ostream & os, string const & s)
{
	begin_inset(os, "ERT");
	os << "\nstatus Collapsed\n\n\\layout Standard\n\n";
	for (string::const_iterator it = s.begin(), et = s.end(); it != et; ++it) {
		if (*it == '\\')
			os << "\n\\backslash\n";
		else
			os << *it;
	}
	end_inset(os);
}


void handle_par(ostream & os)
{
	if (active_environments.empty())
		return;
	os << "\n\\layout ";
	string s = active_environments.top();
	if (s == "document") {
		os << "Standard\n\n";
		return;
	}
	if (s == "lyxcode") {
		os << "LyX-Code\n\n";
		return;
	}
	if (s.size()) 
		s[0] = toupper(s[0]);	
	os << s << "\n\n";
}


void handle_package(string const & name, string const & options)
{
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
	else {
		if (options.size())
			h_preamble << "\\usepackage[" << options << "]{" << name << "}\n";
		else
			h_preamble << "\\usepackage{" << name << "}\n";
	}
}


string wrap(string const & cmd, string const & str)
{
	return OPEN + cmd + ' ' + str + CLOSE;
}


void end_preamble(ostream & os)
{
	in_preamble = false;
	os << "# tex2lyx 0.0.2 created this file\n"
	   << "\\lyxformat 222\n"
	   << "\\textclass " << h_textclass << "\n"
	   << "\\begin_preamble\n" << h_preamble.str() << "\n\\end_preamble\n"
	   << "\\options " << h_options << "\n"
	   << "\\language " << h_language << "\n"
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
	   << h_preamble.str() << "\n";
}


void parse(Parser & p, ostream & os, unsigned flags, mode_type mode)
{
	while (p.good()) {
		Token const & t = p.getToken();

#ifdef FILEDEBUG
		cerr << "t: " << t << " flags: " << flags << "\n";
		//cell->dump();
#endif

		if (flags & FLAG_ITEM) {
			if (t.cat() == catSpace)
				continue;

			flags &= ~FLAG_ITEM;
			if (t.cat() == catBegin) {
				// skip the brace and collect everything to the next matching
				// closing brace
				flags |= FLAG_BRACE_LAST;
				continue;
			}

			// handle only this single token, leave the loop if done
			flags |= FLAG_LEAVE;
		}


		if (flags & FLAG_BRACED) {
			if (t.cat() == catSpace)
				continue;

			if (t.cat() != catBegin) {
				p.error("opening brace expected");
				return;
			}

			// skip the brace and collect everything to the next matching
			// closing brace
			flags = FLAG_BRACE_LAST;
		}


		if (flags & FLAG_OPTION) {
			if (t.cat() == catOther && t.character() == '[') {
				parse(p, os, FLAG_BRACK_LAST, mode);
			} else {
				// no option found, put back token and we are done
				p.putback();
			}
			return;
		}

		//
		// cat codes
		//
		if (t.cat() == catMath) {
			if (mode == TEXT_MODE || mode == MATHTEXT_MODE) {
				// we are inside some text mode thingy, so opening new math is allowed
				if (mode == TEXT_MODE)
					begin_inset(os, "Formula ");
				Token const & n = p.getToken();
				if (n.cat() == catMath) {
					// TeX's $$...$$ syntax for displayed math
					os << "\\[";
					parse(p, os, FLAG_SIMPLE, MATH_MODE);
					os << "\\]";
					p.getToken(); // skip the second '$' token
				} else {
					// simple $...$  stuff
					p.putback();
					os << '$';
					parse(p, os, FLAG_SIMPLE, MATH_MODE);
					os << '$';
				}
				if (mode == TEXT_MODE)
					end_inset(os);
			}

			else if (flags & FLAG_SIMPLE) {
				// this is the end of the formula
				return;
			}

			else {
				cerr << "mode: " << mode << endl;
				p.error("something strange in the parser\n");
				break;
			}
		}

		else if (t.cat() == catLetter)
			os << t.character();

		else if (t.cat() == catSpace) 
			os << t.character();

		else if (t.cat() == catNewline)
			os << t.character();

		else if (t.cat() == catSuper)
			os << t.character();

		else if (t.cat() == catSub)
			os << t.character();

		else if (t.cat() == catParameter) {
			Token const & n	= p.getToken();
			os << wrap("macroarg", string(1, n.character()));
		}

		else if (t.cat() == catActive)
			os << wrap("active", string(1, t.character()));

		else if (t.cat() == catBegin) {
			os << '{';
			parse(p, os, FLAG_BRACE_LAST, mode);	
			os << '}';
		}

		else if (t.cat() == catEnd) {
			if (flags & FLAG_BRACE_LAST)
				return;
			p.error("found '}' unexpectedly");
			//lyx::Assert(0);
			//add(cell, '}', LM_TC_TEX);
		}

		else if (t.cat() == catAlign) 
			os << t.character();

/*
			++cellcol;
			//cerr << " column now " << cellcol << " max: " << grid.ncols() << "\n";
			if (cellcol == grid.ncols()) {
				//cerr << "adding column " << cellcol << "\n";
				grid.addCol(cellcol - 1);
			}
			cell = &grid.cell(grid.index(cellrow, cellcol));
		}
*/

		else if (t.character() == ']' && (flags & FLAG_BRACK_LAST)) {
			//cerr << "finished reading option\n";
			return;
		}

		else if (t.cat() == catOther)
			os << string(1, t.character());

		else if (t.cat() == catComment) {
			string s;
			while (p.good()) {
				Token const & t = p.getToken();
				if (t.cat() == catNewline)
					break;
				s += t.asString();
			}
			//os << wrap("comment", s);
			p.skipSpaces();
		}

		//
		// control sequences
		//

		else if (t.cs() == "lyxlock") {
			// ignored;
		}

		else if (t.cs() == "newcommand" || t.cs() == "providecommand") {
			string const name = p.verbatimItem();
			string const opts = p.getArg('[', ']');
			string const body = p.verbatimItem();
			// only non-lyxspecific stuff
			if (name != "noun" && name != "tabularnewline") {
				h_preamble << "\\" << t.cs() << "{" << name << "}";
				if (opts.size()) 
					h_preamble << "[" << opts << "]";
				h_preamble << "{" << body << "}\n";
			}
		}

		else if (t.cs() == "(") {
			begin_inset(os, "Formula");
			os << " \\(";
			parse(p, os, FLAG_SIMPLE2, MATH_MODE);
			os << "\\)";
			end_inset(os);
		}

		else if (t.cs() == "[") {
			begin_inset(os, "Formula");
			os << " \\[";
			parse(p, os, FLAG_EQUATION, MATH_MODE);
			os << "\\]";
			end_inset(os);
		}

		else if (t.cs() == "protect")
			// ignore \\protect, will hopefully be re-added during output
			;

		else if (t.cs() == "begin") {
			string const name = p.getArg('{', '}');
			active_environments.push(name);
			if (name == "document") 
				end_preamble(os);
			else if (name == "abstract")
				handle_par(os);
			else
				os << "\\begin{" << name << "}";
			parse(p, os, FLAG_END, mode);
		}

		else if (t.cs() == "end") {
			if (flags & FLAG_END) {
				// eat environment name
				string const name = p.getArg('{', '}');
				if (name != active_environments.top())
					p.error("\\end{" + name + "} does not match \\begin{"
						+ active_environments.top() + "}");
				active_environments.pop();
				if (name == "document" || name == "abstract")
					;
				else
					os << "\\end{" << name << "}";
				return;
			}
			p.error("found 'end' unexpectedly");
		}

		else if (t.cs() == ")") {
			if (flags & FLAG_SIMPLE2)
				return;
			p.error("found '\\)' unexpectedly");
		}

		else if (t.cs() == "]") {
			if (flags & FLAG_EQUATION)
				return;
			p.error("found '\\]' unexpectedly");
		}

		else if (t.cs() == "documentclass") {
			vector<string> opts;
			split(p.getArg('[', ']'), opts, ',');
			handle_opt(opts, known_languages, h_language); 
			handle_opt(opts, known_fontsizes, h_paperfontsize); 
			h_quotes_language = h_language;
			h_options = join(opts, ',');
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
			p.skipSpaces();
			string const begin = p.verbatimItem();
			p.skipSpaces();
			string const end = p.verbatimItem();
			// ignore out mess
			if (name != "lyxcode") 
				os << wrap("newenvironment", begin + end); 
		}

		else if (t.cs() == "def") {
			string name = p.getToken().cs();
			while (p.nextToken().cat() != catBegin)
				name += p.getToken().asString();
			handle_ert(os, "\\def\\" + name + '{' + p.verbatimItem() + '}');
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
			string const name = p.getToken().cs();
			string const content = p.getArg('{', '}');
			if (name == "parskip")
				h_paragraph_separation = "skip";
			else if (name == "parindent")
				h_paragraph_separation = "skip";
			else
				h_preamble << "\\setlength{" << name << "}{" << content << "}\n";
		}
	
		else if (t.cs() == "par")
			handle_par(os);

		else if (t.cs() == "title")
			os << "\\layout Title\n\n" + p.verbatimItem();

		else if (t.cs() == "author")
			os << "\\layout Author\n\n" + p.verbatimItem();

		else if (t.cs() == "makeindex" || t.cs() == "maketitle")
			; // swallow this

		else if (t.cs() == "tableofcontents")
			p.verbatimItem(); // swallow this

		else if (t.cs() == "textrm") {
			os << '\\' << t.cs() << '{';
			parse(p, os, FLAG_ITEM, MATHTEXT_MODE);
			os << '}';
		}

		else if (t.cs() == "emph" && mode == TEXT_MODE) {
			os << "\n\\emph on\n";
			parse(p, os, FLAG_ITEM, mode);
			os << "\n\\emph default\n";
		}

		else if (t.cs() == "index") {
			begin_inset(os, "LatexCommand ");
			parse(p, os, FLAG_ITEM, TEXT_MODE);
			end_inset(os);
		}

		else 
			(in_preamble ? h_preamble : os) << '\\' << t.asString();

		if (flags & FLAG_LEAVE) {
			flags &= ~FLAG_LEAVE;
			break;
		}
	}
}



} // anonymous namespace


int main(int argc, char * argv[])
{
	if (argc <= 1) {
		cerr << "Usage: " << argv[0] << " <infile.tex>" << endl;
		return 2;
	}

	ifstream is(argv[1]);
	Parser p(is);
	parse(p, cout, 0, TEXT_MODE);
	cout << "\n\\the_end";

	return 0;	
}	

// }])
