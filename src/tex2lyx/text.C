/** The .tex to .lyx converter
    \author André Pönitz (2003)
 */

// {[(

#include <config.h>

#include "Lsstream.h"
#include "tex2lyx.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>

using std::cerr;
using std::endl;
using std::map;
using std::ostream;
using std::ostringstream;
using std::string;
using std::vector;


namespace {

char const * known_headings[] = { "caption", "title", "author", "date",
"paragraph", "chapter", "section", "subsection", "subsubsection", 0 };

char const * known_latex_commands[] = { "ref", "cite", "label", "index",
"printindex", "pageref", "url", 0 };

// LaTeX names for quotes
char const * known_quotes[] = { "glqq", "grqq", "quotedblbase",
"textquotedblleft", 0};

// the same as known_quotes with .lyx names
char const * known_coded_quotes[] = { "gld", "grd", "gld", "grd", 0};


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
	if (active_environments_empty())
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


} // anonymous namespace


void parse_text(Parser & p, ostream & os, unsigned flags, bool outer)
{
	while (p.good()) {
		Token const & t = p.getToken();

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


		//
		// cat codes
		//
		if (t.cat() == catMath) {
			// we are inside some text mode thingy, so opening new math is allowed
			begin_inset(os, "Formula ");
			Token const & n = p.getToken();
			if (n.cat() == catMath && outer) {
				// TeX's $$...$$ syntax for displayed math
				os << "\\[";
				parse_math(p, os, FLAG_SIMPLE, MATH_MODE);
				os << "\\]";
				p.getToken(); // skip the second '$' token
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
			if (p.nextToken().cat() == catNewline) {
				p.getToken();
				handle_par(os);
			} else {
				os << "\n "; // note the space
			}
		}

		else if (t.cat() == catActive) {
			if (t.character() == '~') {
				if (curr_env() == "lyxcode")
					os << ' ';
				else
					os << "\\SpecialChar ~\n";
			} else
				os << t.character();
		}

		else if (t.cat() == catBegin) {
			handle_ert(os, "{");
			parse_text(p, os, FLAG_BRACE_LAST, outer);
			handle_ert(os, "}");
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

		else if (t.cs() == "ldots")
			os << "\n\\SpecialChar \\ldots{}\n";

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
			active_environments_push(name);
			if (name == "abstract") {
				handle_par(os);
				parse_text(p, os, FLAG_END, outer);
			} else if (is_math_env(name)) {
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
				os << "wide false\n"
					 << "collapsed false\n"
					 << "\n"
					 << "\\layout Standard\n";
				parse_text(p, os, FLAG_END, outer);
				end_inset(os);
			} else if (name == "lyxlist") {
				p.verbatimItem(); // swallow next arg
				parse_text(p, os, FLAG_END, outer);
				os << "\n\\layout Bibliography\n\n";
			} else if (name == "thebibliography") {
				p.verbatimItem(); // swallow next arg
				parse_text(p, os, FLAG_END, outer);
				os << "\n\\layout Bibliography\n\n";
			} else {
				parse_text(p, os, FLAG_END, outer);
			}
		}

		else if (t.cs() == "end") {
			if (flags & FLAG_END) {
				// eat environment name
				string const name = p.getArg('{', '}');
				if (name != curr_env())
					p.error("\\end{" + name + "} does not match \\begin{"
						+ curr_env() + "}");
				active_environments_pop();
				return;
			}
			p.error("found 'end' unexpectedly");
		}

		else if (t.cs() == "item")
			handle_par(os);

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
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\n\\layout Standard\n\n";
		}

		else if (t.cs() == "includegraphics") {
			map<string, string> opts = split_map(p.getArg('[', ']'));
			string name = p.verbatimItem();
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

		else if (t.cs() == "makeindex" || t.cs() == "maketitle")
			; // swallow this

		else if (t.cs() == "tableofcontents")
			p.verbatimItem(); // swallow this

		else if (t.cs() == "tiny" || t.cs() == "scriptsize")
			os << "\n\\size " << t.cs() << "\n";

		else if (t.cs() == "Large")
			os << "\n\\size larger\n";

		else if (t.cs() == "textrm") {
			os << "\n\\family roman\n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\family default\n";
		}

		else if (t.cs() == "textsf") {
			os << "\n\\family sans\n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\family default\n";
		}

		else if (t.cs() == "texttt") {
			os << "\n\\family typewriter\n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\family default\n";
		}

		else if (t.cs() == "textsc") {
			os << "\n\\noun on\n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\noun default\n";
		}

		else if (t.cs() == "textbf") {
			os << "\n\\series bold\n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\series default\n";
		}

		else if (t.cs() == "underbar") {
			os << "\n\\bar under\n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\bar default\n";
		}

		else if (t.cs() == "emph" || t.cs() == "noun") {
			os << "\n\\" << t.cs() << " on\n";
			parse_text(p, os, FLAG_ITEM, outer);
			os << "\n\\" << t.cs() << " default\n";
		}

		else if (t.cs() == "bibitem") {
			os << "\n\\layout Bibliography\n\\bibitem ";
			os << p.getOpt();
			os << '{' << p.verbatimItem() << '}' << "\n\n";
		}

		else if (is_known(t.cs(), known_latex_commands)) {
			begin_inset(os, "LatexCommand ");
			os << '\\' << t.cs();
			os << p.getOpt();
			os << p.getOpt();
			os << '{' << p.verbatimItem() << '}';
			end_inset(os);
		}

		else if (is_known(t.cs(), known_quotes)) {
		  char const ** where = is_known(t.cs(), known_quotes);
			begin_inset(os, "Quotes ");
			os << known_coded_quotes[where - known_quotes];
			end_inset(os);
		}

		else if (t.cs() == "LyX" || t.cs() == "TeX"
		      || t.cs() == "LaTeX" || t.cs() == "LaTeXe") {
			p.verbatimItem(); // eat {}
			os << "LyX";
		}

		else if (t.cs() == "lyxarrow") {
			p.verbatimItem();
			os << "\\SpecialChar \\menuseparator\n";
		}

		else if (t.cs() == "ldots") {
			p.verbatimItem();
			os << "\\SpecialChar \\ldots{}\n";
		}

		else if (t.cs() == "@")
			os << "\\SpecialChar \\@";

		else if (t.cs() == "textasciitilde")
			os << '~';

		else if (t.cs() == "_" || t.cs() == "&" || t.cs() == "#")
			os << t.cs();

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
	return os.str();
}


// }])
