/** The .tex to .lyx converter
    \author André Pönitz (2003)
 */

// {[(

#include <config.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
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
using std::ostream;
using std::ostringstream;
using std::stack;
using std::string;
using std::vector;


namespace {

void parse(Parser & p, ostream & os, unsigned flags, mode_type mode);

char const OPEN = '<';
char const CLOSE = '>';

// rather brutish way to code table structure in a string:
//
//  \begin{tabular}{ccc}
//    1 & 2 & 3\\ 
//    \multicolumn{2}{c}{4} & 5\\ 
//    6 & 7 \\ 
//  \end{tabular}
//
// gets "translated" to:
//  
//  1 TAB 2 TAB 3 LINE
//  2 MULT c MULT 4 TAB 5 LINE
//  5 TAB 7 LINE

char const TAB  = '\001';
char const LINE = '\002';
char const MULT = '\003';

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

char const * known_headings[] = { "caption", "title", "author",
"paragraph", "chapter", "section", "subsection", "subsubsection", 0 };

char const * known_math_envs[] = { "equation", "eqnarray", "eqnarray*",
"align", "align*", 0};

char const * known_latex_commands[] = { "ref", "cite", "label", "index",
"printindex", 0 };

// LaTeX names for quotes
char const * known_quotes[] = { "glqq", "grqq", 0};

// the same as known_quotes with .lyx names
char const * known_coded_quotes[] = { "gld", "grd", 0};



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
string h_quotes_times            = "1";
string h_papercolumns            = "1";
string h_papersides              = "1";
string h_paperpagestyle          = "default";
string h_tracking_changes        = "0";

// indicates whether we are in the preamble
bool in_preamble = true;

// current stack of nested environments
stack<string> active_environments;


string cap(string s)
{
	if (s.size())
		s[0] = toupper(s[0]);
	return s;
}


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
	os << "\n\\end_inset\n";
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
	if (s == "document" || s == "table") {
		os << "Standard\n\n";
		return;
	}
	if (s == "lyxcode") {
		os << "LyX-Code\n\n";
		return;
	}
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
	for (size_t i = 0; i < s.size(); ++i) {
		switch (s[i]) {
			case 'c': res.push_back("center"); break;
			case 'l': res.push_back("left");   break;
			case 'r': res.push_back("right");  break;
			default : res.push_back("right");  break;
		}
	}
	return res;
}


void handle_tabular(Parser & p, ostream & os, mode_type mode)
{
	begin_inset(os, "Tabular \n");
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
		vector<string> cells;
		split(lines[r], cells, TAB);
		while (cells.size() < cols)
			cells.push_back(string());
		//os << "<row bottomline=\"true\">\n";
		os << "<row>\n";
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
	   << "\\tracking_changes " << h_tracking_changes << "\n";
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
			os << ' ';

		else if (t.cat() == catSuper)
			os << t.character();

		else if (t.cat() == catSub)
			os << t.character();

		else if (t.cat() == catParameter) {
			Token const & n	= p.getToken();
			os << wrap("macroarg", string(1, n.character()));
		}

		else if (t.cat() == catActive) {
			if (t.character() == '~')
				os << (curr_env() == "lyxcode" ? ' ' : '~');
			else
				os << t.asInput();
		}

		else if (t.cat() == catBegin) {
			if (mode == MATH_MODE)
				os << '{';
			else
				handle_ert(os, "{");
		}

		else if (t.cat() == catEnd) {
			if (flags & FLAG_BRACE_LAST)
				return;
			if (mode == MATH_MODE)
				os << '}';
			else
				handle_ert(os, "}");
		}

		else if (t.cat() == catAlign) {
			if (mode == MATH_MODE)
				os << t.character();
			else
				os << TAB;
		}

		else if (t.cs() == "tabularnewline") {
			if (mode == MATH_MODE)
				os << t.asInput();
			else
				os << LINE;
		}

		else if (t.cs() == "\\" && mode == MATH_MODE)
			os << t.asInput();

		else if (t.cs() == "\\" && curr_env() == "tabular")
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

		else if (t.cs() == "lyxlock")
			; // ignored

		else if (t.cs() == "newcommand" || t.cs() == "providecommand") {
			string const name = p.verbatimItem();
			string const opts = p.getArg('[', ']');
			string const body = p.verbatimItem();
			// only non-lyxspecific stuff
			if (name != "\\noun " && name != "\\tabularnewline ") {
				ostream & out = in_preamble ? h_preamble : os;
				if (!in_preamble)
					begin_inset(os, "FormulaMacro\n");
				out << "\\" << t.cs() << "{" << name << "}";
				if (opts.size()) 
					out << "[" << opts << "]";
				out << "{" << body << "}";
				if (!in_preamble)
					end_inset(os);
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
			if (name == "document") {
				end_preamble(os);
				parse(p, os, FLAG_END, mode);
			} else if (name == "abstract") {
				handle_par(os);
				parse(p, os, FLAG_END, mode);
			} else if (is_math_env(name)) {
				begin_inset(os, "Formula ");	
				os << "\\begin{" << name << "}";
				parse(p, os, FLAG_END, MATH_MODE);
				os << "\\end{" << name << "}";
				end_inset(os);	
			} else if (name == "tabular") {
				handle_tabular(p, os, mode);
			} else if (name == "table") {
				begin_inset(os, "Float table\n");	
				os << "wide false\n"
				   << "collapsed false\n"
				   << "\n"
				   << "\\layout Standard\n";
				parse(p, os, FLAG_END, mode);
				end_inset(os);	
			} else if (name == "thebibliography") {
				p.verbatimItem(); // swallow next arg
				parse(p, os, FLAG_END, mode);
				os << "\n\\layout Standard\n\n";
			} else if (mode == MATH_MODE) {
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

		else if (is_known(t.cs(), known_headings)) {
			string name = t.cs();
			if (p.nextToken().asInput() == "*") {
				p.getToken();
				name += "*";
			}
			os << "\\layout " << cap(name) << "\n\n";
			parse(p, os, FLAG_ITEM, mode);
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

		else if (t.cs() == "textrm") {
			os << '\\' << t.cs() << '{';
			parse(p, os, FLAG_ITEM, MATHTEXT_MODE);
			os << '}';
		}

		else if ((t.cs() == "emph" || t.cs() == "noun") && mode == TEXT_MODE) {
			os << "\n\\" << t.cs() << " on\n";
			parse(p, os, FLAG_ITEM, mode);
			os << "\n\\" << t.cs() << " default\n";
		}

		else if (is_known(t.cs(), known_latex_commands) && mode == TEXT_MODE) {
			begin_inset(os, "LatexCommand ");
			os << '\\' << t.cs() << '{';
			parse(p, os, FLAG_ITEM, TEXT_MODE);
			os << '}';
			end_inset(os);
		}

		else if (t.cs() == "bibitem") {
			os << "\n\\layout Bibliography\n\\bibitem ";
			string opt = p.getArg('[',']');
			if (opt.size())
				os << '[' << opt << ']';
			os << '{' << p.getArg('{','}') << '}' << "\n\n";
		}

		else if (char const ** where = is_known(t.cs(), known_quotes)) {
			begin_inset(os, "Quotes ");
			os << known_coded_quotes[where - known_quotes];
			end_inset(os);
		}

		else if (t.cs() == "textasciitilde")
			os << '~';

		else if (t.cs() == "_" && mode == TEXT_MODE)
			os << '_';

		else if (t.cs() == "&" && mode == TEXT_MODE)
			os << '&';

		else if (t.cs() == "pagestyle" && in_preamble)
			h_paperpagestyle == p.getArg('{','}');

		else {
			if (mode == MATH_MODE)
				os << t.asInput();
			else if (in_preamble)
				h_preamble << t.asInput();
			else {
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
	parse(p, cout, 0, TEXT_MODE);
	cout << "\n\\the_end";

	return 0;	
}	

// }])
