/** The .tex to .lyx converter
    \author André Pönitz (2003)
 */

// {[(

#include <config.h>

#include "Lsstream.h"
#include "tex2lyx.h"

#include <iostream>
#include <map>
#include <vector>

using std::cerr;
using std::endl;
using std::map;
using std::ostream;
using std::ostringstream;
using std::vector;


namespace {

char const * known_headings[] = { "caption", "title", "author", "date",
"paragraph", "chapter", "section", "subsection", "subsubsection", 0 };

char const * known_latex_commands[] = { "ref", "cite", "label", "index",
"printindex", "pageref", "url", 0 };

// LaTeX names for quotes
char const * known_quotes[] = { "glqq", "grqq", "quotedblbase",
"textquotedblleft", "quotesinglbase", "guilsinglleft", "guilsinglright", 0};

// the same as known_quotes with .lyx names
char const * known_coded_quotes[] = { "gld", "grd", "gld",
"grd", "gls", "fls", "frd", 0};

char const * known_sizes[] = { "tiny", "scriptsize", "footnotesize",
"small", "normalsize", "large", "Large", "LARGE", "huge", "Huge", 0};

char const * known_coded_sizes[] = { "tiny", "scriptsize", "footnotesize",
"small", "normal", "large", "larger", "largest",  "huge", "giant", 0};


string cap(string s)
{
	if (s.size())
		s[0] = toupper(s[0]);
	return s;
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


void begin_inset(ostream & os, string const & name)
{
	os << "\n\\begin_inset " << name;
}


void end_inset(ostream & os)
{
	os << "\n\\end_inset\n\n";
}


void skip_braces(Parser & p)
{
	if (p.next_token().cat() != catBegin)
		return;
	p.get_token();
	if (p.next_token().cat() == catEnd) {
		p.get_token();
		return;
	}
	p.putback();
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
	string s = active_environment();
	if (s == "document" || s == "table")
		os << "Standard\n\n";
	else if (s == "center")
		os << "Standard\n\n\\align center\n";
	else if (s == "lyxcode")
		os << "LyX-Code\n\n";
	else if (s == "lyxlist")
		os << "List\n\n";
	else if (s == "thebibliography")
		os << "Bibliography\n\n";
	else
		os << cap(s) << "\n\n";
}


} // anonymous namespace


void parse_text(Parser & p, ostream & os, unsigned flags, bool outer)
{
	while (p.good()) {
		Token const & t = p.get_token();

#ifdef FILEDEBUG
		cerr << "t: " << t << " flags: " << flags << "\n";
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

		if (t.character() == ']' && (flags & FLAG_BRACK_LAST)) 
			return;

		//
		// cat codes
		//
		if (t.cat() == catMath) {
			// we are inside some text mode thingy, so opening new math is allowed
			begin_inset(os, "Formula ");
			Token const & n = p.get_token();
			if (n.cat() == catMath && outer) {
				// TeX's $$...$$ syntax for displayed math
				os << "\\[";
				parse_math(p, os, FLAG_SIMPLE, MATH_MODE);
				os << "\\]";
				p.get_token(); // skip the second '$' token
			} else {
				// simple $...$  stuff
				p.putback();
				os << '$';
				parse_math(p, os, FLAG_SIMPLE, MATH_MODE);
				os << '$';
			}
			end_inset(os);
		}

		else if (t.cat() == catSuper || t.cat() == catSub)
			cerr << "catcode " << t << " illegal in text mode\n";

		else if (t.cat() == catLetter ||
			       t.cat() == catSpace ||
			       t.cat() == catOther ||
			       t.cat() == catAlign ||
			       t.cat() == catParameter)
			os << t.character();

		else if (t.cat() == catNewline) {
			if (p.next_token().cat() == catNewline) {
				p.get_token();
				handle_par(os);
			} else {
				os << " "; // note the space
			}
		}

		else if (t.cat() == catActive) {
			if (t.character() == '~') {
				if (active_environment() == "lyxcode")
					os << ' ';
				else
					os << "\\SpecialChar ~\n";
			} else
				os << t.character();
		}

		else if (t.cat() == catBegin) {
			// special handling of size changes
			bool const is_size = is_known(p.next_token().cs(), known_sizes);
			string const s = parse_text(p, FLAG_BRACE_LAST, outer);
			if (s.empty() && p.next_token().character() == '`')
				; // ignore it in  {}``
			else if (is_size || s == "[" || s == "]" || s == "*")
				os << s;
			else {
				handle_ert(os, "{");
				os << s;
				handle_ert(os, "}");
			}
		}

		else if (t.cat() == catEnd) {
			if (flags & FLAG_BRACE_LAST)
				return;
			cerr << "stray '}' in text\n";
			handle_ert(os, "}");
		}

		else if (t.cat() == catOther)
			os << string(1, t.character());

		else if (t.cat() == catComment)
			handle_comment(p);

		//
		// control sequences
		//

		else if (t.cs() == "ldots") {
			skip_braces(p);
			os << "\n\\SpecialChar \\ldots{}\n";
		}

		else if (t.cs() == "(") {
			begin_inset(os, "Formula");
			os << " \\(";
			parse_math(p, os, FLAG_SIMPLE2, MATH_MODE);
			os << "\\)";
			end_inset(os);
		}

		else if (t.cs() == "[") {
			begin_inset(os, "Formula");
			os << " \\[";
			parse_math(p, os, FLAG_EQUATION, MATH_MODE);
			os << "\\]";
			end_inset(os);
		}

		else if (t.cs() == "begin") {
			string const name = p.getArg('{', '}');
			active_environments.push_back(name);
			if (is_math_env(name)) {
				begin_inset(os, "Formula ");
				os << "\\begin{" << name << "}";
				parse_math(p, os, FLAG_END, MATH_MODE);
				os << "\\end{" << name << "}";
				end_inset(os);
			} else if (name == "tabular") {
				begin_inset(os, "Tabular ");
				handle_tabular(p, os);
				end_inset(os);
			} else if (name == "table" || name == "figure") {
				string opts = p.getOpt();
				begin_inset(os, "Float " + name + "\n");
				if (opts.size())
					os << "placement " << opts << '\n';
				os << "wide false\ncollapsed false\n\n"
					 << "\\layout Standard\n";
				parse_text(p, os, FLAG_END, outer);
				end_inset(os);
			} else if (name == "center") {
				handle_par(os);
				parse_text(p, os, FLAG_END, outer);
			} else if (name == "enumerate" || name == "itemize"
					|| name == "lyxlist") {
				size_t const n = active_environments.size();
				string const s = active_environments[n - 2];
				bool const deeper = s == "enumerate" || s == "itemize"
					|| s == "lyxlist";
				if (deeper)
					os << "\n\\begin_deeper";
				os << "\n\\layout " << cap(name) << "\n\n";
				if (name == "lyxlist")
					p.verbatim_item(); // swallow next arg
				parse_text(p, os, FLAG_END, outer);
				if (deeper)
					os << "\n\\end_deeper\n";
				handle_par(os);
			} else if (name == "thebibliography") {
				p.verbatim_item(); // swallow next arg
				parse_text(p, os, FLAG_END, outer);
				os << "\n\\layout Bibliography\n\n";
			} else {
				handle_par(os);
				parse_text(p, os, FLAG_END, outer);
			}
		}

		else if (t.cs() == "end") {
			if (flags & FLAG_END) {
				// eat environment name
				string const name = p.getArg('{', '}');
				if (name != active_environment())
					cerr << "\\end{" + name + "} does not match \\begin{"
						+ active_environment() + "}\n";
				active_environments.pop_back();
				handle_par(os);
				return;
			}
			p.error("found 'end' unexpectedly");
		}

		else if (t.cs() == "item") {
			p.skip_spaces();
			string s; 
			if (p.next_token().character() == '[') {
				p.get_token(); // eat '['
				s = parse_text(p, FLAG_BRACK_LAST, outer);
			}
			handle_par(os);
			os << s << ' ';
		}

		else if (t.cs() == "def") {
			string name = p.get_token().cs();
			while (p.next_token().cat() != catBegin)
				name += p.get_token().asString();
			handle_ert(os, "\\def\\" + name + '{' + p.verbatim_item() + '}');
		}

		else if (t.cs() == "par") {
			p.skip_spaces();
			if (p.next_token().cs() != "\\begin")
				handle_par(os);
			//cerr << "next token: '" << p.next_token().cs() << "'\n";
		}

		else if (is_known(t.cs(), known_headings)) {
			string name = t.cs();
			if (p.next_token().asInput() == "*") {
				p.get_token();
				name += "*";
			}
			os << "\n\n\\layout " << cap(name) << "\n\n";
			string opt = p.getOpt();
			if (opt.size()) {
				begin_inset(os, "OptArg\n");
				os << "collapsed true\n\n\\layout Standard\n\n" << opt;
				end_inset(os);
			}
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\n\\layout Standard\n\n";
		}

		else if (t.cs() == "includegraphics") {
			map<string, string> opts = split_map(p.getArg('[', ']'));
			string name = p.verbatim_item();
			begin_inset(os, "Graphics ");
			os << "\n\tfilename " << name << '\n';
			if (opts.find("width") != opts.end())
				os << "\twidth " << opts["width"] << '\n';
			if (opts.find("height") != opts.end())
				os << "\theight " << opts["height"] << '\n';
			end_inset(os);
		}
		
		else if (t.cs() == "footnote") {
			begin_inset(os, "Foot\n");
			os << "collapsed true\n\n\\layout Standard\n\n";
			parse_text(p, os, FLAG_ITEM, false);
			end_inset(os);
		}

		else if (t.cs() == "ensuremath") {
			string s = parse_text(p, FLAG_ITEM, false);
			if (s == "±" || s == "³" || s == "²" || s == "µ")
				os << s;
			else
				handle_ert(os, "\\ensuremath{" + s + "}");
		}

		else if (t.cs() == "marginpar") {
			begin_inset(os, "Marginal\n");
			os << "collapsed true\n\n\\layout Standard\n\n";
			parse_text(p, os, FLAG_ITEM, false);
			end_inset(os);
		}

		else if (t.cs() == "hfill") {
			os << "\n\\hfill\n";
			skip_braces(p);
		}

		else if (t.cs() == "makeindex" || t.cs() == "maketitle")
			skip_braces(p); // swallow this

		else if (t.cs() == "tableofcontents")
			skip_braces(p); // swallow this

		else if (t.cs() == "textrm") {
			os << "\n\\family roman \n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\family default \n";
		}

		else if (t.cs() == "textsf") {
			os << "\n\\family sans \n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\family default \n";
		}

		else if (t.cs() == "texttt") {
			os << "\n\\family typewriter \n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\family default \n";
		}

		else if (t.cs() == "textit") {
			os << "\n\\shape italic \n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\shape default \n";
		}

		else if (t.cs() == "textsc") {
			os << "\n\\noun on \n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\noun default \n";
		}

		else if (t.cs() == "textbf") {
			os << "\n\\series bold \n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\series default \n";
		}

		else if (t.cs() == "underbar") {
			os << "\n\\bar under \n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\bar default \n";
		}

		else if (t.cs() == "emph" || t.cs() == "noun") {
			os << "\n\\" << t.cs() << " on \n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\" << t.cs() << " default \n";
		}

		else if (t.cs() == "bibitem") {
			os << "\n\\layout Bibliography\n\\bibitem ";
			os << p.getOpt();
			os << '{' << p.verbatim_item() << '}' << "\n\n";
		}

		else if (is_known(t.cs(), known_latex_commands)) {
			begin_inset(os, "LatexCommand ");
			os << '\\' << t.cs();
			os << p.getOpt();
			os << p.getOpt();
			os << '{' << p.verbatim_item() << '}';
			end_inset(os);
		}

		else if (is_known(t.cs(), known_quotes)) {
		  char const ** where = is_known(t.cs(), known_quotes);
			begin_inset(os, "Quotes ");
			os << known_coded_quotes[where - known_quotes];
			end_inset(os);
			skip_braces(p);
		}

		else if (is_known(t.cs(), known_sizes)) {
		  char const ** where = is_known(t.cs(), known_sizes);
			os << "\n\\size " << known_coded_sizes[where - known_sizes] << "\n";
		}

		else if (t.cs() == "LyX" || t.cs() == "TeX"
		      || t.cs() == "LaTeX" || t.cs() == "LaTeXe") {
			os << t.cs();
			skip_braces(p); // eat {}
		}

		else if (t.cs() == "lyxarrow") {
			os << "\\SpecialChar \\menuseparator\n";
			skip_braces(p);
		}

		else if (t.cs() == "ldots") {
			os << "\\SpecialChar \\ldots{}\n";
			skip_braces(p);
		}

		else if (t.cs() == "@") {
			os << "\\SpecialChar \\@";
			skip_braces(p);
		}

		else if (t.cs() == "textasciitilde") {
			os << '~';
			skip_braces(p);
		}

		else if (t.cs() == "textasciicircum") {
			os << '^';
			skip_braces(p);
		}

		else if (t.cs() == "textbackslash") {
			os << "\n\\backslash\n";
			skip_braces(p);
		}

		else if (t.cs() == "_" || t.cs() == "&" || t.cs() == "#" || t.cs() == "$"
			    || t.cs() == "{" || t.cs() == "}" || t.cs() == "%")
			os << t.cs();

		else if (t.cs() == "char") {
			if (p.next_token().character() == '`') {
				p.get_token();
				if (p.next_token().cs() == "\"") {
					p.get_token();
					os << '"';
					skip_braces(p);
				} else {
					handle_ert(os, "\\char`");
				}
			} else {
				handle_ert(os, "\\char");
			}
		}

		else if (t.cs() == "\"") {
			string const name = p.verbatim_item();
			     if (name == "a") os << 'ä';
			else if (name == "o") os << 'ö';
			else if (name == "u") os << 'ü';
			else if (name == "A") os << 'Ä';
			else if (name == "O") os << 'Ö';
			else if (name == "U") os << 'Ü';
			else handle_ert(os, "\"{" + name + "}");
		}

		else if (t.cs() == "=" || t.cs() == "H" || t.cs() == "c"
		      || t.cs() == "^" || t.cs() == "'" || t.cs() == "~") {
			// we need the trim as the LyX parser chokes on such spaces
			os << "\n\\i \\" << t.cs() << "{"
			   << trim(parse_text(p, FLAG_ITEM, outer), " ") << "}\n";
		}

		else if (t.cs() == "ss")
			os << "ß";

		else if (t.cs() == "i" || t.cs() == "j")
			os << "\\" << t.cs() << ' ';

		else if (t.cs() == "-")
			os << "\\SpecialChar \\-\n";

		else if (t.cs() == "\\")
			os << "\n\\newline\n";
	
		else if (t.cs() == "lyxrightaddress") {
			os << "\n\\layout Right Address\n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\layout Standard\n";
		}

		else if (t.cs() == "lyxaddress") {
			os << "\n\\layout Address\n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\layout Standard\n";
		}

		else if (t.cs() == "input")
			handle_ert(os, "\\input{" + p.verbatim_item() + "}\n");

		else if (t.cs() == "fancyhead") {
			ostringstream ss;
			ss << "\\fancyhead";
			ss << p.getOpt();
			ss << '{' << p.verbatim_item() << "}\n";
			handle_ert(os, STRCONV(ss.str()));
		}

		else {
			//cerr << "#: " << t << " mode: " << mode << endl;
			// heuristic: read up to next non-nested space
			/*
			string s = t.asInput();
			string z = p.verbatim_item();
			while (p.good() && z != " " && z.size()) {
				//cerr << "read: " << z << endl;
				s += z;
				z = p.verbatim_item();
			}
			cerr << "found ERT: " << s << endl;
			handle_ert(os, s + ' ');
			*/
			handle_ert(os, t.asInput() + ' ');
		}

		if (flags & FLAG_LEAVE) {
			flags &= ~FLAG_LEAVE;
			break;
		}
	}
}


string parse_text(Parser & p, unsigned flags, const bool outer)
{
	ostringstream os;
	parse_text(p, os, flags, outer);
	return STRCONV(os.str());
}


// }])
