/** The .tex to .lyx converter
    \author André Pönitz (2003)
 */

// {[(

#include <config.h>

#include "tex2lyx.h"
#include "FloatList.h"
#include "lyxtextclass.h"
#include "support/lstrings.h"
#include "support/tostr.h"

#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using std::cerr;
using std::endl;
using std::map;
using std::ostream;
using std::ostringstream;
using std::string;
using std::vector;

using lyx::support::rtrim;
using lyx::support::suffixIs;

// Do we need to output some \begin_layout command before the next characters?
bool need_layout = true;
// We may need to add something after this \begin_layout command
string extra_stuff;
// Do we need to output some \end_layout command 
bool need_end_layout = false;

void check_end_layout(ostream & os) 
{
	if (need_end_layout) {
		os << "\n\\end_layout\n";
		need_end_layout = false;
	}
}


namespace {

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


void check_layout(ostream & os, LyXLayout_ptr layout)
{
	if (need_layout) {
		check_end_layout(os);
		os << "\n\\begin_layout " << layout->name() << "\n\n";
		need_end_layout = true;
		need_layout=false;
		if (!extra_stuff.empty()) {
			os << extra_stuff;
			extra_stuff.erase();
		}
	}
}


void begin_inset(ostream & os, string const & name)
{
	os << "\n\\begin_inset " << name;
}


void end_inset(ostream & os)
{
	os << "\n\\end_inset \n\n";
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
	os << "\nstatus Collapsed\n\n\\begin_layout Standard\n\n";
	for (string::const_iterator it = s.begin(), et = s.end(); it != et; ++it) {
		if (*it == '\\')
			os << "\n\\backslash \n";
		else
			os << *it;
	}
	need_end_layout = true;
	check_end_layout(os);
	end_inset(os);
}


struct isLayout {
	isLayout(string const name) : name_(name) {}
	bool operator()(LyXLayout_ptr const & ptr) {
		return ptr.get() && ptr->latexname() == name_;
	}
private:
	string const name_;
};


LyXLayout_ptr findLayout(LyXTextClass const & textclass,
			 string const & name) 
{
	LyXTextClass::const_iterator it  = textclass.begin();
	LyXTextClass::const_iterator end = textclass.end();
	it = std::find_if(it, end, isLayout(name));
	return (it == end) ? LyXLayout_ptr() : *it;
}


void output_command_layout(ostream & os, LyXLayout_ptr const & layout,
		  Parser & p, bool outer, LyXTextClass const & textclass)
{
	need_layout = true;
	check_layout(os, layout);
	if (layout->optionalargs > 0) {
		string s; 
		if (p.next_token().character() == '[') {
			p.get_token(); // eat '['
			begin_inset(os, "OptArg\n");
			os << "collapsed true\n";
			parse_text_in_inset(p, os, FLAG_BRACK_LAST, outer, textclass);
			end_inset(os);
		}
	}
	parse_text(p, os, FLAG_ITEM, outer, textclass, layout);
	need_layout = true;
}


} // anonymous namespace


void parse_environment(Parser & p, ostream & os, bool outer,
		       LyXTextClass const & textclass, LyXLayout_ptr layout)
{
	LyXLayout_ptr newlayout;
	string const name = p.getArg('{', '}');
	const bool is_starred = suffixIs(name, '*');
	string const unstarred_name = rtrim(name, "*");
	active_environments.push_back(name);
	if (is_math_env(name)) {
		check_layout(os, layout);
		begin_inset(os, "Formula ");
		os << "\\begin{" << name << "}";
		parse_math(p, os, FLAG_END, MATH_MODE);
		os << "\\end{" << name << "}";
		end_inset(os);
	} else if (name == "tabular") {
		check_layout(os, layout);
		begin_inset(os, "Tabular ");
		handle_tabular(p, os, textclass);
		end_inset(os);
	} else if (textclass.floats().typeExist(unstarred_name)) {
		check_layout(os, layout);
		begin_inset(os, "Float " + unstarred_name + "\n");
		if (p.next_token().asInput() == "[") {
			os << "placement " << p.getArg('[', ']') << '\n';
		}
		os << "wide " << tostr(is_starred)
		   << "\ncollapsed false\n";
		parse_text_in_inset(p, os, FLAG_END, outer, textclass);
			end_inset(os);
	} else if (name == "center") {
		parse_text(p, os, FLAG_END, outer, textclass);
		// The single '=' is meant here.
	} else if ((newlayout = findLayout(textclass, name)).get() &&
		   newlayout->isEnvironment()) {
		size_t const n = active_environments.size();
		string const s = active_environments[n - 2];
		bool const deeper = s == "enumerate" || s == "itemize"
			|| s == "lyxlist";
		if (deeper)
			os << "\n\\begin_deeper";
		switch (newlayout->latextype) {
		case  LATEX_LIST_ENVIRONMENT:
			extra_stuff = "\\labelwidthstring "
				+ p.verbatim_item() + '\n';
			break;
		case  LATEX_BIB_ENVIRONMENT:
			p.verbatim_item(); // swallow next arg
			break;
		default:
			break;
		}
		need_layout = true;
		parse_text(p, os, FLAG_END, outer, textclass, newlayout);
		check_end_layout(os);
		if (deeper)
			os << "\n\\end_deeper\n";
		need_layout = true;
	} else {
		cerr << "why are we here?" << endl;
		parse_text(p, os, FLAG_END, outer, textclass);
	}
}


void parse_text(Parser & p, ostream & os, unsigned flags, bool outer,
		LyXTextClass const & textclass, LyXLayout_ptr layout)
{
	if (!layout.get())
		layout = textclass.defaultLayout();
	LyXLayout_ptr newlayout;
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
			check_layout(os, layout);
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

		// Basic support for english quotes. This should be
		// extended to other quotes, but is not so easy (a
		// left english quote is the same as a right german
		// quote...)
		else if (t.asInput() == "`" 
			 && p.next_token().asInput() == "`") {
			check_layout(os, layout);
			begin_inset(os, "Quotes ");
			os << "eld";
			end_inset(os);
			p.get_token();
			skip_braces(p);
		}	
		else if (t.asInput() == "'" 
			 && p.next_token().asInput() == "'") {
			check_layout(os, layout);
			begin_inset(os, "Quotes ");
			os << "erd";
			end_inset(os);
			p.get_token();
			skip_braces(p);
		}	


		else if (t.cat() == catLetter ||
			       t.cat() == catSpace ||
			       t.cat() == catOther ||
			       t.cat() == catAlign ||
			       t.cat() == catParameter) {
			check_layout(os, layout);
			os << t.character();
		}

		else if (t.cat() == catNewline) {
			if (p.next_token().cat() == catNewline) {
				p.get_token();
				need_layout = true;
			} else {
				os << " "; // note the space
			}
		}

		else if (t.cat() == catActive) {
			check_layout(os, layout);
			if (t.character() == '~') {
				if (layout->free_spacing)
					os << ' ';
				else 
					os << "\\InsetSpace ~\n";
			} else
				os << t.character();
		}

		else if (t.cat() == catBegin) {
// FIXME??? 
			// special handling of size changes
			check_layout(os, layout);
			bool const is_size = is_known(p.next_token().cs(), known_sizes);
			need_end_layout = false;
			string const s = parse_text(p, FLAG_BRACE_LAST, outer, textclass, layout);
			need_end_layout = true;
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
			if (flags & FLAG_BRACE_LAST) {
				check_end_layout(os);
				return;
			}
			cerr << "stray '}' in text\n";
			handle_ert(os, "}");
		}

		else if (t.cat() == catComment)
			handle_comment(p);

		//
		// control sequences
		//

		else if (t.cs() == "(") {
			check_layout(os, layout);
			begin_inset(os, "Formula");
			os << " \\(";
			parse_math(p, os, FLAG_SIMPLE2, MATH_MODE);
			os << "\\)";
			end_inset(os);
		}

		else if (t.cs() == "[") {
			check_layout(os, layout);
			begin_inset(os, "Formula");
			os << " \\[";
			parse_math(p, os, FLAG_EQUATION, MATH_MODE);
			os << "\\]";
			end_inset(os);
		}

		else if (t.cs() == "begin")
			parse_environment(p, os, outer, textclass, layout);

		else if (t.cs() == "end") {
			if (flags & FLAG_END) {
				// eat environment name
				string const name = p.getArg('{', '}');
				if (name != active_environment())
					cerr << "\\end{" + name + "} does not match \\begin{"
						+ active_environment() + "}\n";
				active_environments.pop_back();
				return;
			}
			p.error("found 'end' unexpectedly");
		}

		else if (t.cs() == "item") {
			// should be done automatically by Parser::tokenize
			//p.skip_spaces();
			string s; 
			if (p.next_token().character() == '[') {
				p.get_token(); // eat '['
				s = parse_text(p, FLAG_BRACK_LAST, outer, textclass, layout);
			}
			need_layout = true;
			check_layout(os, layout);
			if (s.size())
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
			need_layout = true;
//			if (p.next_token().cs() != "\\begin")
//				handle_par(os);
			//cerr << "next token: '" << p.next_token().cs() << "'\n";
		}

		// Must attempt to parse "Section*" before "Section".
		else if ((p.next_token().asInput() == "*") &&
			 // The single '=' is meant here.
			 (newlayout = findLayout(textclass,
						 t.cs() + '*')).get() &&
			 newlayout->isCommand()) {
			p.get_token();
			output_command_layout(os, newlayout, p, outer, textclass);
		}

		// The single '=' is meant here.
		else if ((newlayout = findLayout(textclass, t.cs())).get() &&
			 newlayout->isCommand()) {
			output_command_layout(os, newlayout, p, outer, textclass);
		}

		else if (t.cs() == "includegraphics") {
			map<string, string> opts = split_map(p.getArg('[', ']'));
			string name = p.verbatim_item();
			
			check_layout(os, layout);
			begin_inset(os, "Graphics ");
			os << "\n\tfilename " << name << '\n';
			if (opts.find("width") != opts.end())
				os << "\twidth " << opts["width"] << '\n';
			if (opts.find("height") != opts.end())
				os << "\theight " << opts["height"] << '\n';
			end_inset(os);
		}
		
		else if (t.cs() == "footnote") {
			check_layout(os, layout);
			begin_inset(os, "Foot\n");
			os << "collapsed true\n";
			parse_text_in_inset(p, os, FLAG_ITEM, false, textclass);
			end_inset(os);
		}

		else if (t.cs() == "ensuremath") {
			check_layout(os, layout);
			string s = parse_text(p, FLAG_ITEM, false, textclass);
			if (s == "±" || s == "³" || s == "²" || s == "µ")
				os << s;
			else
				handle_ert(os, "\\ensuremath{" + s + "}");
		}

		else if (t.cs() == "marginpar") {
			check_layout(os, layout);
			begin_inset(os, "Marginal\n");
			os << "collapsed true\n";
			need_layout = true;
			parse_text(p, os, FLAG_ITEM, false, textclass);
			end_inset(os);
			need_end_layout = true;
		}

		else if (t.cs() == "hfill") {
			check_layout(os, layout);
			os << "\n\\hfill\n";
			skip_braces(p);
		}

		else if (t.cs() == "makeindex" || t.cs() == "maketitle")
			skip_braces(p); // swallow this

		else if (t.cs() == "tableofcontents") {
			check_layout(os, layout);
			begin_inset(os, "LatexCommand ");
			os << '\\' << t.cs() << "{}\n";
			end_inset(os);
			skip_braces(p); // swallow this
		}


		else if (t.cs() == "textrm") {
			check_layout(os, layout);
			os << "\n\\family roman \n";
			parse_text(p, os, FLAG_ITEM, outer, textclass);
			os << "\n\\family default \n";
		}

		else if (t.cs() == "textsf") {
			check_layout(os, layout);
			os << "\n\\family sans \n";
			parse_text(p, os, FLAG_ITEM, outer, textclass);
			os << "\n\\family default \n";
		}

		else if (t.cs() == "texttt") {
			check_layout(os, layout);
			os << "\n\\family typewriter \n";
			parse_text(p, os, FLAG_ITEM, outer, textclass);
			os << "\n\\family default \n";
		}

		else if (t.cs() == "textit") {
			check_layout(os, layout);
			os << "\n\\shape italic \n";
			parse_text(p, os, FLAG_ITEM, outer, textclass);
			os << "\n\\shape default \n";
		}

		else if (t.cs() == "textsc") {
			check_layout(os, layout);
			os << "\n\\noun on \n";
			parse_text(p, os, FLAG_ITEM, outer, textclass);
			os << "\n\\noun default \n";
		}

		else if (t.cs() == "textbf") {
			check_layout(os, layout);
			os << "\n\\series bold \n";
			parse_text(p, os, FLAG_ITEM, outer, textclass);
			os << "\n\\series default \n";
		}

		else if (t.cs() == "underbar") {
			check_layout(os, layout);
			os << "\n\\bar under \n";
			parse_text(p, os, FLAG_ITEM, outer, textclass);
			os << "\n\\bar default \n";
		}

		else if (t.cs() == "emph" || t.cs() == "noun") {
			check_layout(os, layout);
			os << "\n\\" << t.cs() << " on \n";
			parse_text(p, os, FLAG_ITEM, outer, textclass);
			os << "\n\\" << t.cs() << " default \n";
		}

		else if (t.cs() == "bibitem") {
			check_layout(os, layout);
			os << "\\bibitem ";
			os << p.getOpt();
			os << '{' << p.verbatim_item() << '}' << "\n";
		}

		else if (is_known(t.cs(), known_latex_commands)) {
			check_layout(os, layout);
			begin_inset(os, "LatexCommand ");
			os << '\\' << t.cs();
			os << p.getOpt();
			os << p.getOpt();
			os << '{' << p.verbatim_item() << "}\n";
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
			check_layout(os, layout);
			os << "\n\\size " << known_coded_sizes[where - known_sizes] << "\n";
		}

		else if (t.cs() == "LyX" || t.cs() == "TeX" 
			 || t.cs() == "LaTeX") {
			check_layout(os, layout);
			os << t.cs();
			skip_braces(p); // eat {}
		}

		else if (t.cs() == "LaTeXe") {
			check_layout(os, layout);
			os << "LaTeX2e";
			skip_braces(p); // eat {}
		}

		else if (t.cs() == "ldots") {
			check_layout(os, layout);
			skip_braces(p);
			os << "\\SpecialChar \\ldots{}\n";
		}

		else if (t.cs() == "lyxarrow") {
			check_layout(os, layout);
			os << "\\SpecialChar \\menuseparator\n";
			skip_braces(p);
		}

		else if (t.cs() == "ldots") {
			check_layout(os, layout);
			os << "\\SpecialChar \\ldots{}\n";
			skip_braces(p);
		}

		else if (t.cs() == "@" && p.next_token().asInput() == ".") {
			check_layout(os, layout);
			os << "\\SpecialChar \\@.\n";
			p.get_token();
		}

		else if (t.cs() == "-") {
			check_layout(os, layout);
			os << "\\SpecialChar \\-\n";
		}

		else if (t.cs() == "textasciitilde") {
			check_layout(os, layout);
			os << '~';
			skip_braces(p);
		}

		else if (t.cs() == "textasciicircum") {
			check_layout(os, layout);
			os << '^';
			skip_braces(p);
		}

		else if (t.cs() == "textbackslash") {
			check_layout(os, layout);
			os << "\n\\backslash \n";
			skip_braces(p);
		}

		else if (t.cs() == "_" || t.cs() == "&" || t.cs() == "#" 
			    || t.cs() == "$" || t.cs() == "{" || t.cs() == "}" 
			    || t.cs() == "%") {
			check_layout(os, layout);
			os << t.cs();
		}

		else if (t.cs() == "char") {
			check_layout(os, layout);
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
			check_layout(os, layout);
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
			check_layout(os, layout);
			os << "\n\\i \\" << t.cs() << "{"
			   << trim(parse_text(p, FLAG_ITEM, outer, textclass), " ") << "}\n";
		}

		else if (t.cs() == "ss") {
			check_layout(os, layout);
			os << "ß";
		}

		else if (t.cs() == "i" || t.cs() == "j") {
			check_layout(os, layout);
			os << "\\" << t.cs() << ' ';
		}

		else if (t.cs() == "\\") {
			check_layout(os, layout);
			os << "\n\\newline \n";
		}
	
		else if (t.cs() == "input") {
			check_layout(os, layout);
			handle_ert(os, "\\input{" + p.verbatim_item() + "}\n");
		}
		else if (t.cs() == "fancyhead") {
			check_layout(os, layout);
			ostringstream ss;
			ss << "\\fancyhead";
			ss << p.getOpt();
			ss << '{' << p.verbatim_item() << "}\n";
			handle_ert(os, ss.str());
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
			check_layout(os, layout);
			handle_ert(os, t.asInput() + ' ');
		}

		if (flags & FLAG_LEAVE) {
			flags &= ~FLAG_LEAVE;
			break;
		}
	}
}


string parse_text(Parser & p, unsigned flags, const bool outer,
		  LyXTextClass const & textclass,
		  LyXLayout_ptr layout)
{
	ostringstream os;
	parse_text(p, os, flags, outer, textclass, layout);
	return os.str();
}

void parse_text_in_inset(Parser & p, ostream & os, unsigned flags, bool outer,
		LyXTextClass const & textclass, LyXLayout_ptr layout)
{
		need_layout = true;
		need_end_layout = false;
		parse_text(p, os, flags, outer, textclass, layout);
		check_end_layout(os);
		need_end_layout = true;
}
// }])
