/** The .tex to .lyx converter
    \author André Pönitz (2003)
 */

// {[(

#include <config.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <vector>

#include "Lsstream.h"

#include "texparser.h"

using std::count_if;
using std::cout;
using std::cerr;
using std::endl;
using std::fill;
using std::getline;
using std::ios;
using std::ifstream;
using std::istream;
using std::istringstream;
using std::map;
using std::ostream;
using std::ostringstream;
using std::stack;
using std::string;
using std::vector;


namespace {

void parse_preamble(Parser & p, ostream & os);

void parse(Parser & p, ostream & os, unsigned flags, const mode_type mode);

char const OPEN = '<';
char const CLOSE = '>';

// rather brutish way to code table structure in a string:
//
//  \begin{tabular}{ccc}
//    1 & 2 & 3\\ \hline
//    \multicolumn{2}{c}{4} & 5\\ 
//    6 & 7 \\ 
//  \end{tabular}
//
// gets "translated" to:
//  
//  1 TAB 2 TAB 3 LINE
//  HLINE 2 MULT c MULT 4 TAB 5 LINE
//  5 TAB 7 LINE

char const TAB   = '\001';
char const LINE  = '\002';
char const MULT  = '\003';
char const HLINE = '\004';

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

char const * known_headings[] = { "caption", "title", "author", "date",
"paragraph", "chapter", "section", "subsection", "subsubsection", 0 };

char const * known_math_envs[] = { "equation", "equation*",
"eqnarray", "eqnarray*", "align", "align*", 0};

char const * known_latex_commands[] = { "ref", "cite", "label", "index",
"printindex", 0 };

// LaTeX names for quotes
char const * known_quotes[] = { "glqq", "grqq", "quotedblbase",
"textquotedblleft", 0};

// the same as known_quotes with .lyx names
char const * known_coded_quotes[] = { "gld", "grd", "gld", "grd", 0};



// some ugly stuff
ostringstream h_preamble;
string h_textclass               = "article";
string h_options                 = "";
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

// current stack of nested environments
stack<string> active_environments;


string cap(string s)
{
	if (s.size())
		s[0] = toupper(s[0]);
	return s;
}


string const trim(string const & a, char const * p = " \t\n\r")
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


void split(string const & s, vector<string> & result, char delim = ',')
{
	istringstream is(s);	
	string t;
	while (getline(is, t, delim))
		result.push_back(t);
}


// splits "x=z, y=b" into a map
map<string, string> split_map(string const & s)
{
	map<string, string> res;
	vector<string> v;
	split(s, v);
	for (size_t i = 0; i < v.size(); ++i) {
		size_t const pos   = v[i].find('=');
		string const index = v[i].substr(0, pos);
		string const value = v[i].substr(pos + 1, string::npos);	
		res[trim(index)] = trim(value);
	}
	return res;
}


string join(vector<string> const & input, char const * delim)
{
	ostringstream os;
	for (size_t i = 0; i != input.size(); ++i) {
		if (i)
			os << delim;	
		os << input[i]; 
	}
	return os.str();
}


char const ** is_known(string const & str, char const ** what)
{
	for ( ; *what; ++what)
		if (str == *what)
			return what;
	return 0;
}


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


bool is_math_env(string const & name)
{
	for (char const ** what = known_math_envs; *what; ++what)
		if (*what == name)
			return true;
	return false;
}


void begin_inset(ostream & os, string const & name)
{
	os << "\n\\begin_inset " << name;
}


void end_inset(ostream & os)
{
	os << "\n\\end_inset\n\n";
}


string curr_env()
{
	return active_environments.empty() ? string() : active_environments.top();
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
	string s = curr_env();
	if (s == "document" || s == "table") 
		os << "Standard\n\n";
	else if (s == "lyxcode")
		os << "LyX-Code\n\n";
	else if (s == "lyxlist")
		os << "List\n\n";
	else if (s == "thebibliography")
		os << "Bibliography\n\n";
	else
		os << cap(s) << "\n\n";
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


vector<string> extract_col_align(string const & s)
{
	vector<string> res;
	string t;
	for (size_t i = 0; i < s.size(); ++i) {
		switch (s[i]) {
			case 'c': res.push_back("center"); break;
			case 'l': res.push_back("left");   break;
			case 'r': res.push_back("right");  break;
			//case '|': cerr << "ignoring vertical separator\n";  break;
			//default : cerr << "ignoring special separator '" << s[i] << "'\n"; break;
			default: break;
		}
	}
	return res;
}


void handle_tabular(Parser & p, ostream & os, mode_type mode)
{
	begin_inset(os, "Tabular \n");
	string posopts = p.getOpt();
	if (posopts.size())
		cerr << "vertical tabular positioning '" << posopts << "' ignored\n";
	string colopts = p.verbatimItem();
	vector<string> colalign = extract_col_align(colopts);
	ostringstream ss;
	parse(p, ss, FLAG_END, mode);
	vector<string> lines;
	split(ss.str(), lines, LINE);
	const size_t cols = colalign.size();
	const size_t rows = lines.size();
	os << "<lyxtabular version=\"3\" rows=\"" << rows
		 << "\" columns=\"" << cols << "\">\n"
		 << "<features>\n";
	for (size_t c = 0; c < cols; ++c)
		os << "<column alignment=\"" << colalign[c] << "\""
			 << " valignment=\"top\""
			 << " width=\"0pt\""
			 << ">\n";
	for (size_t r = 0; r < rows; ++r) {
		vector<string> dummy;
		// handle hlines
		split(lines[r], dummy, HLINE);
		int hlines = dummy.size() - 1;
		lines[r] = join(dummy, "");
		// handle almost empty line
		if (lines[r].empty() && hlines && r > 0) {
			cerr << "last hline lost\n";
			continue;
		}
		// handle cells
		vector<string> cells;
		split(lines[r], cells, TAB);
		while (cells.size() < cols)
			cells.push_back(string());
		os << "<row";
		if (hlines)
			os << " topline=\"true\"";	
		os << ">\n";
		for (size_t c = 0; c < cols; ++c) {
			os << "<cell";
			string alignment = "center";
			vector<string> parts;
			split(cells[c], parts, MULT);
			if (parts.size() > 2) {
				os << " multicolumn=\"" << parts[0] << "\"";
				alignment = parts[1];
			}
			os << " alignment=\"" << alignment << "\""
				 << " valignment=\"top\""
				 << " topline=\"true\""
				 << " leftline=\"true\""
				 << " usebox=\"none\""
				 << ">";
			begin_inset(os, "Text");
			os << "\n\n\\layout Standard\n\n";
			if (parts.size())
				os << parts.back();
			end_inset(os);
			os << "</cell>\n";
		}
		os << "</row>\n";
	}
	os << "</lyxtabular>\n";
	end_inset(os);	
}


string wrap(string const & cmd, string const & str)
{
	return OPEN + cmd + ' ' + str + CLOSE;
}


void end_preamble(ostream & os)
{
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
	   << "\\tracking_changes " << h_tracking_changes << "\n";
}


void parse_preamble(Parser & p, ostream & os)
{
	while (p.good()) {
		Token const & t = p.getToken();

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

		else if (t.cs() == "pagestyle")
			h_paperpagestyle == p.verbatimItem();

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
			string const name = p.verbatimItem();
			string const opts = p.getOpt();
			string const body = p.verbatimItem();
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
				ss << '\\' << t.cs() << '{' << name << '}'
					<< opts << '{' << body << "}\n";
				h_preamble << ss.str();
/*
				ostream & out = in_preamble ? h_preamble : os;
				out << "\\" << t.cs() << "{" << name << "}"
				    << opts << "{" << body << "}\n";
				if (!in_preamble)
					end_inset(os);
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
			ss << '{' << p.verbatimItem() << '}';
			ss << '{' << p.verbatimItem() << '}';
			ss << '\n';
			if (name != "lyxcode" && name != "lyxlist"
					&& name != "lyxrightadress" && name != "lyxaddress")
				h_preamble << ss.str();
		}

		else if (t.cs() == "def") {
			string name = p.getToken().cs();
			while (p.nextToken().cat() != catBegin)
				name += p.getToken().asString();
			h_preamble << "\\def\\" << name << '{' << p.verbatimItem() << "}\n";
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
			string const name = p.verbatimItem();
			string const content = p.verbatimItem();
			if (name == "parskip")
				h_paragraph_separation = "skip";
			else if (name == "parindent")
				h_paragraph_separation = "skip";
			else 
				h_preamble << "\\setlength{" + name + "}{" + content + "}\n";
		}
	
		else if (t.cs() == "par")
			h_preamble << '\n';

		else if (t.cs() == "begin") {
			string const name = p.getArg('{', '}');
			if (name == "document") {
				end_preamble(os);
				os << "\n\n\\layout Standard\n\n";
				return;
			}
			h_preamble << "\\begin{" << name << "}";
		}

		else if (t.cs().size())
			h_preamble << '\\' << t.cs() << ' ';
	}
}


void parse(Parser & p, ostream & os, unsigned flags, const mode_type mode)
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

		else if (t.cat() == catLetter ||
			       t.cat() == catSpace || 
		         t.cat() == catSuper ||
			       t.cat() == catSub ||
			       t.cat() == catOther ||
			       t.cat() == catParameter)
			os << t.character();

		else if (t.cat() == catNewline)
			os << ' ';

		else if (t.cat() == catActive) {
			if (t.character() == '~') {
				if (curr_env() == "lyxcode")
					os << ' ';
				else if (mode == TEXT_MODE)
					os << "\\SpecialChar ~\n";
				else 
					os << '~';
			} else
				os << t.character();
		}

		else if (t.cat() == catBegin) {
			if (mode == TEXT_MODE)
				handle_ert(os, "{");
			else
				os << '{';
		}

		else if (t.cat() == catEnd) {
			if (flags & FLAG_BRACE_LAST)
				return;
			if (mode == TEXT_MODE)
				handle_ert(os, "}");
			else
				os << '}';
		}

		else if (t.cat() == catAlign) {
			if (mode == TEXT_MODE)
				os << TAB;
			else
				os << t.character();
		}

		else if (t.cs() == "tabularnewline") {
			if (mode == TEXT_MODE)
				os << LINE;
			else
				os << t.asInput();
		}

		else if (t.cs() == "\\" && mode == MATH_MODE)
			os << t.asInput();

		else if (t.cs() == "\\" && mode == TEXT_MODE && curr_env() == "tabular")
			os << LINE;

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

		else if (t.cs() == "ldots" && mode == MATH_MODE)
			os << "\n\\SpecialChar \\ldots{}\n";

		else if (t.cs() == "lyxlock")
			; // ignored

		else if (t.cs() == "makeatletter") {
			p.setCatCode('@', catLetter);
			handle_ert(os, "\\makeatletter\n");
		}
			
		else if (t.cs() == "makeatother") {
			p.setCatCode('@', catOther);
			handle_ert(os, "\\makeatother\n");
		}

		else if (t.cs() == "newcommand" || t.cs() == "renewcommand"
			    || t.cs() == "providecommand") {
			string const name = p.verbatimItem();
			string const opts = p.getOpt();
			string const body = p.verbatimItem();
			// only non-lyxspecific stuff
			if (name != "\\noun " && name != "\\tabularnewline ") {
				ostringstream ss;
				ss << '\\' << t.cs() << '{' << name << '}'
					<< opts << '{' << body << "}\n";
				handle_ert(os, ss.str());
/*
				ostream & out = in_preamble ? h_preamble : os;
				if (!in_preamble)
					begin_inset(os, "FormulaMacro\n");
				out << "\\" << t.cs() << "{" << name << "}"
				    << opts << "{" << body << "}\n";
				if (!in_preamble)
					end_inset(os);
*/
			}
		}

		else if (t.cs() == "newtheorem") {
			ostringstream ss;
			ss << "\\newtheorem";
			ss << '{' << p.verbatimItem() << '}';
			ss << p.getOpt();
			ss << '{' << p.verbatimItem() << '}';
			ss << p.getOpt();
			ss << '\n';
			handle_ert(os, ss.str());
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
			if (name == "abstract") {
				handle_par(os);
				parse(p, os, FLAG_END, mode);
			} else if (is_math_env(name)) {
				begin_inset(os, "Formula ");	
				os << "\\begin{" << name << "}";
				parse(p, os, FLAG_END, MATH_MODE);
				os << "\\end{" << name << "}";
				end_inset(os);	
			} else if (name == "tabular") {
				if (mode == TEXT_MODE) 
					handle_tabular(p, os, mode);
				else {
					os << "\\begin{" << name << "}";
					parse(p, os, FLAG_END, MATHTEXT_MODE);
					os << "\\end{" << name << "}";
				}
			} else if (name == "table" || name == "figure") {
				string opts = p.getOpt();
				begin_inset(os, "Float " + name + "\n");
				if (opts.size())
					os << "placement " << opts << '\n';
				os << "wide false\n"
					 << "collapsed false\n"
					 << "\n"
					 << "\\layout Standard\n";
				parse(p, os, FLAG_END, mode);
				end_inset(os);	
			} else if (name == "thebibliography") {
				p.verbatimItem(); // swallow next arg
				parse(p, os, FLAG_END, mode);
				os << "\n\\layout Bibliography\n\n";
			} else if (mode == MATH_MODE || mode == MATHTEXT_MODE) {
				os << "\\begin{" << name << "}";
				parse(p, os, FLAG_END, mode);
				os << "\\end{" << name << "}";
			} else {
				parse(p, os, FLAG_END, mode);
			}
		}

		else if (t.cs() == "end") {
			if (flags & FLAG_END) {
				// eat environment name
				string const name = p.getArg('{', '}');
				if (name != curr_env())
					p.error("\\end{" + name + "} does not match \\begin{"
						+ curr_env() + "}");
				active_environments.pop();
				return;
			}
			p.error("found 'end' unexpectedly");
		}

		else if (t.cs() == "item")
			handle_par(os);

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

		else if (t.cs() == "def") {
			string name = p.getToken().cs();
			while (p.nextToken().cat() != catBegin)
				name += p.getToken().asString();
			handle_ert(os, "\\def\\" + name + '{' + p.verbatimItem() + '}');
		}

		else if (t.cs() == "par")
			handle_par(os);

		else if (is_known(t.cs(), known_headings)) {
			string name = t.cs();
			if (p.nextToken().asInput() == "*") {
				p.getToken();
				name += "*";
			}
			os << "\n\n\\layout " << cap(name) << "\n\n";
			string opt = p.getOpt();
			if (opt.size()) {
				begin_inset(os, "OptArg\n");
				os << "collapsed true\n\n\\layout Standard\n\n" << opt;
				end_inset(os);
			}
			parse(p, os, FLAG_ITEM, mode);
			os << "\n\n\\layout Standard\n\n";
		}

		else if (t.cs() == "includegraphics") {
			if (mode == TEXT_MODE) {
				map<string, string> opts = split_map(p.getArg('[', ']'));
				string name = p.verbatimItem();
				begin_inset(os, "Graphics ");
				os << "\n\tfilename " << name << '\n';
				if (opts.find("width") != opts.end())
					os << "\twidth " << opts["width"] << '\n';
				if (opts.find("height") != opts.end())
					os << "\theight " << opts["height"] << '\n';
				end_inset(os);
			} else {
				os << "\\includegraphics ";
			}
		}

		else if (t.cs() == "makeindex" || t.cs() == "maketitle")
			; // swallow this

		else if (t.cs() == "tableofcontents")
			p.verbatimItem(); // swallow this

		else if (t.cs() == "multicolumn" && mode == TEXT_MODE) {
			// brutish...
			parse(p, os, FLAG_ITEM, mode); 
			os << MULT;
			parse(p, os, FLAG_ITEM, mode); 
			os << MULT;
			parse(p, os, FLAG_ITEM, mode); 
		}

		else if (t.cs() == "hline" && mode == TEXT_MODE) 
			os << HLINE;

		else if (t.cs() == "textrm") {
			if (mode == TEXT_MODE) {
				os << "\n\\family roman\n";
				parse(p, os, FLAG_ITEM, TEXT_MODE);
				os << "\n\\family default\n";
			} else {
				os << '\\' << t.cs() << '{';
				parse(p, os, FLAG_ITEM, MATHTEXT_MODE);
				os << '}';
			}
		}

		else if (t.cs() == "textsf") {
			if (mode == TEXT_MODE) {
				os << "\n\\family sans\n";
				parse(p, os, FLAG_ITEM, TEXT_MODE);
				os << "\n\\family default\n";
			} else {
				os << '\\' << t.cs() << '{';
				parse(p, os, FLAG_ITEM, MATHTEXT_MODE);
				os << '}';
			}
		}

		else if (t.cs() == "texttt") {
			if (mode == TEXT_MODE) {
				os << "\n\\family typewriter\n";
				parse(p, os, FLAG_ITEM, TEXT_MODE);
				os << "\n\\family default\n";
			} else {
				os << '\\' << t.cs() << '{';
				parse(p, os, FLAG_ITEM, MATHTEXT_MODE);
				os << '}';
			}
		}

		else if (t.cs() == "textsc") {
			if (mode == TEXT_MODE) {
				os << "\n\\noun on\n";
				parse(p, os, FLAG_ITEM, TEXT_MODE);
				os << "\n\\noun default\n";
			} else {
				os << '\\' << t.cs() << '{';
				parse(p, os, FLAG_ITEM, MATHTEXT_MODE);
				os << '}';
			}
		}

		else if (t.cs() == "textbf") {
			if (mode == TEXT_MODE) {
				os << "\n\\series bold\n";
				parse(p, os, FLAG_ITEM, TEXT_MODE);
				os << "\n\\series default\n";
			} else {
				os << '\\' << t.cs() << '{';
				parse(p, os, FLAG_ITEM, MATHTEXT_MODE);
				os << '}';
			}
		}

		else if (t.cs() == "underbar") {
			if (mode == TEXT_MODE) {
				os << "\n\\bar under\n";
				parse(p, os, FLAG_ITEM, TEXT_MODE);
				os << "\n\\bar default\n";
			} else {
				os << '\\' << t.cs() << '{';
				parse(p, os, FLAG_ITEM, MATHTEXT_MODE);
				os << '}';
			}
		}

		else if ((t.cs() == "emph" || t.cs() == "noun") && mode == TEXT_MODE) {
			os << "\n\\" << t.cs() << " on\n";
			parse(p, os, FLAG_ITEM, mode);
			os << "\n\\" << t.cs() << " default\n";
		}

		else if (is_known(t.cs(), known_latex_commands) && mode == TEXT_MODE) {
			begin_inset(os, "LatexCommand ");
			os << '\\' << t.cs();
			os << p.getOpt();
			os << p.getOpt();
			os << '{' << p.verbatimItem() << '}';
			end_inset(os);
		}

		else if (t.cs() == "bibitem") {
			os << "\n\\layout Bibliography\n\\bibitem ";
			os << p.getOpt();
			os << '{' << p.verbatimItem() << '}' << "\n\n";
		}

		else if (mode == TEXT_MODE && is_known(t.cs(), known_quotes)) {
		  char const ** where = is_known(t.cs(), known_quotes);
			begin_inset(os, "Quotes ");
			os << known_coded_quotes[where - known_quotes];
			end_inset(os);
		}

		else if (t.cs() == "LyX") {
			p.verbatimItem(); // eat {}
			os << "LyX";
		}

		else if (t.cs() == "TeX") {
			p.verbatimItem(); // eat {}
			os << "TeX";
		}

		else if (t.cs() == "LaTeX") {
			p.verbatimItem(); // eat {}
			os << "LaTeX";
		}

		else if (t.cs() == "LaTeXe") {
			p.verbatimItem(); // eat {}
			os << "LaTeXe";
		}

		else if (t.cs() == "textasciitilde")
			os << '~';

		else if (t.cs() == "_" && mode == TEXT_MODE)
			os << '_';

		else if (t.cs() == "&" && mode == TEXT_MODE)
			os << '&';

		else if (t.cs() == "\"") {
			string const name = p.verbatimItem();
			     if (name == "a") os << 'ä';
			else if (name == "o") os << 'ö';
			else if (name == "u") os << 'ü';
			else if (name == "A") os << 'Ä';
			else if (name == "O") os << 'Ö';
			else if (name == "U") os << 'Ü';
			else handle_ert(os, "\"{" + name + "}");
		}

		else if (t.cs() == "ss") 
			os << "ß";

		else if (t.cs() == "input")
			handle_ert(os, "\\input{" + p.verbatimItem() + "}\n");

		else if (t.cs() == "fancyhead") {
			ostringstream ss;
			ss << "\\fancyhead";
			ss << p.getOpt();
			ss << '{' << p.verbatimItem() << "}\n";
			handle_ert(os, ss.str());
		}

		else {
			//cerr << "#: " << t << " mode: " << mode << endl;
			if (mode == MATH_MODE || mode == MATHTEXT_MODE) {
				os << t.asInput();
				//cerr << "#: writing: '" << t.asInput() << "'\n";
			} else {
				// heuristic: read up to next non-nested space
				/*
				string s = t.asInput();
				string z = p.verbatimItem();
				while (p.good() && z != " " && z.size()) {
					//cerr << "read: " << z << endl;
					s += z;
					z = p.verbatimItem();
				}
				cerr << "found ERT: " << s << endl;
				handle_ert(os, s + ' ');
				*/
				handle_ert(os, t.asInput() + ' ');
			}
		}

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
	parse_preamble(p, cout);
	active_environments.push("document");
	parse(p, cout, FLAG_END, TEXT_MODE);
	cout << "\n\\the_end";

	return 0;	
}	

// }])
