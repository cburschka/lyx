/**
 * \file tex2lyx/text.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

// {[(

#include <config.h>

#include "tex2lyx.h"
#include "context.h"
#include "FloatList.h"
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
using std::istringstream;
using std::string;
using std::vector;

using lyx::support::rtrim;
using lyx::support::suffixIs;


// thin wrapper around parse_text using a string
string parse_text(Parser & p, unsigned flags, const bool outer,
		  Context & context)
{
	ostringstream os;
	parse_text(p, os, flags, outer, context);
	return os.str();
}

// parses a subdocument, usually useful in insets (whence the name)
void parse_text_in_inset(Parser & p, ostream & os, unsigned flags, bool outer,
		Context & context)
{
	Context newcontext(true, context.textclass);
	parse_text(p, os, flags, outer, newcontext);
	newcontext.check_end_layout(os);
}


// parses a paragraph snippet, useful for example for \emph{...}
void parse_text_snippet(Parser & p, ostream & os, unsigned flags, bool outer,
		Context & context)
{
	Context newcontext(false, context.textclass);
	parse_text(p, os, flags, outer, newcontext);
	// should not be needed
	newcontext.check_end_layout(os);
}


namespace {

char const * known_latex_commands[] = { "ref", "cite", "label", "index",
"printindex", "pageref", "url", "vref", "vpageref", "prettyref", "eqref", 0 };

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

// A simple function to translate a latex length to something lyx can
// understand. Not perfect, but rather best-effort.
string translate_len(string const & len)
{
	const string::size_type i = len.find_first_not_of(" -01234567890.");
	// a normal length
	if (i == string::npos || len[i]  != '\\')
		return len;
	istringstream iss(string(len, 0, i));
	double val;
	iss >> val;
	val = val*100;
	ostringstream oss;
	oss << val;
	string const valstring = oss.str();
	const string::size_type i2 = len.find(" ", i);
	string const unit = string(len, i, i2 - i);
	string const endlen = (i2 == string::npos) ? string() : string(len, i2);
	if (unit == "\\textwidth")
		return valstring + "text%" + endlen;
	else if (unit == "\\columnwidth")
		return valstring + "col%" + endlen;
	else if (unit == "\\paperwidth")
		return valstring + "page%" + endlen;
	else if (unit == "\\linewidth")
		return valstring + "line%" + endlen;
	else if (unit == "\\paperheight")
		return valstring + "pheight%" + endlen;
	else if (unit == "\\textheight")
		return valstring + "theight%" + endlen;
	else
		return len;
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


void handle_ert(ostream & os, string const & s, Context const & context)
{
	Context newcontext(true, context.textclass);
	begin_inset(os, "ERT");
	os << "\nstatus Collapsed\n";
	newcontext.check_layout(os);
	for (string::const_iterator it = s.begin(), et = s.end(); it != et; ++it) {
		if (*it == '\\')
			os << "\n\\backslash \n";
		else
			os << *it;
	}
	newcontext.check_end_layout(os);
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


void output_command_layout(ostream & os, Parser & p, bool outer,
			   Context & parent_context,
			   LyXLayout_ptr newlayout)
{
	parent_context.check_end_layout(os);
	Context context(true, parent_context.textclass, newlayout,
			parent_context.layout);
	context.check_deeper(os);
	context.check_layout(os);
	if (context.layout->optionalargs > 0) {
		string s;
		if (p.next_token().character() == '[') {
			p.get_token(); // eat '['
			begin_inset(os, "OptArg\n");
			os << "collapsed true\n";
			parse_text_in_inset(p, os, FLAG_BRACK_LAST, outer, context);
			end_inset(os);
		}
	}
	parse_text_snippet(p, os, FLAG_ITEM, outer, context);
	context.check_end_layout(os);
	context.check_end_deeper(os);
}


void parse_environment(Parser & p, ostream & os, bool outer,
		       Context & parent_context)
{
	LyXLayout_ptr newlayout;
	string const name = p.getArg('{', '}');
	const bool is_starred = suffixIs(name, '*');
	string const unstarred_name = rtrim(name, "*");
	active_environments.push_back(name);
	if (is_math_env(name)) {
		parent_context.check_layout(os);
		begin_inset(os, "Formula ");
		os << "\\begin{" << name << "}";
		parse_math(p, os, FLAG_END, MATH_MODE);
		os << "\\end{" << name << "}";
		end_inset(os);
	}

	else if (name == "tabular") {
		parent_context.check_layout(os);
		begin_inset(os, "Tabular ");
		handle_tabular(p, os, parent_context);
		end_inset(os);
	}

	else if (parent_context.textclass.floats().typeExist(unstarred_name)) {
		parent_context.check_layout(os);
		begin_inset(os, "Float " + unstarred_name + "\n");
		if (p.next_token().asInput() == "[") {
			os << "placement " << p.getArg('[', ']') << '\n';
		}
		os << "wide " << tostr(is_starred)
		   << "\ncollapsed false\n";
		parse_text_in_inset(p, os, FLAG_END, outer, parent_context);
		end_inset(os);
	}

	else if (name == "minipage") {
		parent_context.check_layout(os);
		begin_inset(os, "Minipage\n");
		string position = "1";
		string inner_pos = "0";
		string height;
		if (p.next_token().asInput() == "[") {
			switch(p.getArg('[', ']')[0]) {
			case 't': position = "0"; break;
			case 'c': position = "1"; break;
			case 'b': position = "2"; break;
			default:
				cerr << "invalid position for minipage"
				     << endl;
				break;
			}
			if (p.next_token().asInput() == "[") {
				height = translate_len(p.getArg('[', ']'));

				if (p.next_token().asInput() == "[") {
					switch(p.getArg('[', ']')[0]) {
					case 't': inner_pos = "0"; break;
					case 'c': inner_pos = "1"; break;
					case 'b': inner_pos = "2"; break;
					case 's': inner_pos = "3"; break;
					default:
						cerr << "invalid inner_pos for minipage"
						     << endl;
						break;
					}
				}
			}
		}
		os << "position " << position << '\n';
		os << "inner_position " << inner_pos << '\n';
		os << "height \"" << height << "\"\n";
		os << "width \"" << translate_len(p.verbatim_item()) << "\"\n";
		os << "collapsed false\n";
		parse_text_in_inset(p, os, FLAG_END, outer, parent_context);
		end_inset(os);

	}

	else if (name == "center") {
		parse_text(p, os, FLAG_END, outer, parent_context);
		// The single '=' is meant here.
	}

	else if ((newlayout = findLayout(parent_context.textclass, name)).get() &&
		   newlayout->isEnvironment()) {
		Context context(true, parent_context.textclass, newlayout,
				parent_context.layout);
		parent_context.check_end_layout(os);
		switch (context.layout->latextype) {
		case  LATEX_LIST_ENVIRONMENT:
			context.extra_stuff = "\\labelwidthstring "
				+ p.verbatim_item() + '\n';
			break;
		case  LATEX_BIB_ENVIRONMENT:
			p.verbatim_item(); // swallow next arg
			break;
		default:
			break;
		}
		context.check_deeper(os);
		parse_text(p, os, FLAG_END, outer, context);
		context.check_end_layout(os);
		context.check_end_deeper(os);
	}

	else {
		parent_context.check_layout(os);
		handle_ert(os, "\\begin{" + name + "}", parent_context);
		parse_text_snippet(p, os, FLAG_END, outer, parent_context);
		handle_ert(os, "\\end{" + name + "}", parent_context);
	}
}

} // anonymous namespace




void parse_text(Parser & p, ostream & os, unsigned flags, bool outer,
		Context & context)
{
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
			context.check_layout(os);
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
			context.check_layout(os);
			begin_inset(os, "Quotes ");
			os << "eld";
			end_inset(os);
			p.get_token();
			skip_braces(p);
		}
		else if (t.asInput() == "'"
			 && p.next_token().asInput() == "'") {
			context.check_layout(os);
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
			context.check_layout(os);
			os << t.character();
		}

		else if (t.cat() == catNewline) {
			if (p.next_token().cat() == catNewline) {
				// this should have been be done by
				// the parser already
				cerr << "what are we doing here?" << endl;
				p.get_token();
				context.need_layout = true;
			} else {
				os << " "; // note the space
			}
		}

		else if (t.cat() == catActive) {
			context.check_layout(os);
			if (t.character() == '~') {
				if (context.layout->free_spacing)
					os << ' ';
				else
					os << "\\InsetSpace ~\n";
			} else
				os << t.character();
		}

		else if (t.cat() == catBegin) {
// FIXME???
			// special handling of size changes
			context.check_layout(os);
			bool const is_size = is_known(p.next_token().cs(), known_sizes);
			Context newcontext(false, context.textclass);
//			need_end_layout = false;
			string const s = parse_text(p, FLAG_BRACE_LAST, outer, newcontext);
//			need_end_layout = true;
			if (s.empty() && p.next_token().character() == '`')
				; // ignore it in  {}``
			else if (is_size || s == "[" || s == "]" || s == "*")
				os << s;
			else {
				handle_ert(os, "{", context);
				os << s;
				handle_ert(os, "}", context);
			}
		}

		else if (t.cat() == catEnd) {
			if (flags & FLAG_BRACE_LAST) {
				context.check_end_layout(os);
				return;
			}
			cerr << "stray '}' in text\n";
			handle_ert(os, "}", context);
		}

		else if (t.cat() == catComment)
			handle_comment(p);

		//
		// control sequences
		//

		else if (t.cs() == "(") {
			context.check_layout(os);
			begin_inset(os, "Formula");
			os << " \\(";
			parse_math(p, os, FLAG_SIMPLE2, MATH_MODE);
			os << "\\)";
			end_inset(os);
		}

		else if (t.cs() == "[") {
			context.check_layout(os);
			begin_inset(os, "Formula");
			os << " \\[";
			parse_math(p, os, FLAG_EQUATION, MATH_MODE);
			os << "\\]";
			end_inset(os);
		}

		else if (t.cs() == "begin")
			parse_environment(p, os, outer, context);

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
				Context newcontext(false, context.textclass);
				s = parse_text(p, FLAG_BRACK_LAST, outer, newcontext);
			}
			context.need_layout = true;
			context.has_item = true;
			context.check_layout(os);
			if (s.size())
				os << s << ' ';
		}

		else if (t.cs() == "bibitem") {
			context.need_layout = true;
			context.has_item = true;
			context.check_layout(os);
			os << "\\bibitem ";
			os << p.getOpt();
			os << '{' << p.verbatim_item() << '}' << "\n";
		}

		else if (t.cs() == "def") {
			string name = p.get_token().cs();
			while (p.next_token().cat() != catBegin)
				name += p.get_token().asString();
			handle_ert(os, "\\def\\" + name + '{' + p.verbatim_item() + '}', context);
		}

		else if (t.cs() == "par") {
			p.skip_spaces();
			context.check_end_layout(os);
			context.need_layout = true;
		}

		// Must attempt to parse "Section*" before "Section".
		else if ((p.next_token().asInput() == "*") &&
			 // The single '=' is meant here.
			 (newlayout = findLayout(context.textclass,
						 t.cs() + '*')).get() &&
			 newlayout->isCommand()) {
			p.get_token();
			output_command_layout(os, p, outer, context, newlayout);
		}

		// The single '=' is meant here.
		else if ((newlayout = findLayout(context.textclass, t.cs())).get() &&
			 newlayout->isCommand()) {
			output_command_layout(os, p, outer, context, newlayout);
		}

		else if (t.cs() == "includegraphics") {
			map<string, string> opts = split_map(p.getArg('[', ']'));
			string name = p.verbatim_item();

			context.check_layout(os);
			begin_inset(os, "Graphics ");
			os << "\n\tfilename " << name << '\n';
			if (opts.find("width") != opts.end())
				os << "\twidth "
				   << translate_len(opts["width"]) << '\n';
			if (opts.find("height") != opts.end())
				os << "\theight "
				   << translate_len(opts["height"]) << '\n';
			end_inset(os);
		}

		else if (t.cs() == "footnote") {
			context.check_layout(os);
			begin_inset(os, "Foot\n");
			os << "collapsed true\n";
			parse_text_in_inset(p, os, FLAG_ITEM, false, context);
			end_inset(os);
		}

		else if (t.cs() == "marginpar") {
			context.check_layout(os);
			begin_inset(os, "Marginal\n");
			os << "collapsed true\n";
			parse_text_in_inset(p, os, FLAG_ITEM, false, context);
			end_inset(os);
		}

		else if (t.cs() == "ensuremath") {
			context.check_layout(os);
			Context newcontext(false, context.textclass);
			string s = parse_text(p, FLAG_ITEM, false, newcontext);
			if (s == "±" || s == "³" || s == "²" || s == "µ")
				os << s;
			else
				handle_ert(os, "\\ensuremath{" + s + "}",
					   context);
		}

		else if (t.cs() == "hfill") {
			context.check_layout(os);
			os << "\n\\hfill\n";
			skip_braces(p);
		}

		else if (t.cs() == "makeindex" || t.cs() == "maketitle")
			skip_braces(p); // swallow this

		else if (t.cs() == "tableofcontents") {
			context.check_layout(os);
			begin_inset(os, "LatexCommand \\tableofcontents\n");
			end_inset(os);
			skip_braces(p); // swallow this
		}

		else if (t.cs() == "listoffigures") {
			context.check_layout(os);
			begin_inset(os, "FloatList figure\n");
			end_inset(os);
			skip_braces(p); // swallow this
		}

		else if (t.cs() == "listoftables") {
			context.check_layout(os);
			begin_inset(os, "FloatList table\n");
			end_inset(os);
			skip_braces(p); // swallow this
		}

		else if (t.cs() == "listof") {
			string const name = p.get_token().asString();
			if (context.textclass.floats().typeExist(name)) {
				context.check_layout(os);
				begin_inset(os, "FloatList ");
				os << name << "\n";
				end_inset(os);
				p.get_token(); // swallow second arg
			} else
				handle_ert(os, "\\listof{" + name + "}", context);
		}

		else if (t.cs() == "textrm") {
			context.check_layout(os);
			os << "\n\\family roman \n";
			parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			os << "\n\\family default \n";
		}

		else if (t.cs() == "textsf") {
			context.check_layout(os);
			os << "\n\\family sans \n";
			parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			os << "\n\\family default \n";
		}

		else if (t.cs() == "texttt") {
			context.check_layout(os);
			os << "\n\\family typewriter \n";
			parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			os << "\n\\family default \n";
		}

		else if (t.cs() == "textit") {
			context.check_layout(os);
			os << "\n\\shape italic \n";
			parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			os << "\n\\shape default \n";
		}

		else if (t.cs() == "textsc") {
			context.check_layout(os);
			os << "\n\\noun on \n";
			parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			os << "\n\\noun default \n";
		}

		else if (t.cs() == "textbf") {
			context.check_layout(os);
			os << "\n\\series bold \n";
			parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			os << "\n\\series default \n";
		}

		else if (t.cs() == "underbar") {
			context.check_layout(os);
			os << "\n\\bar under \n";
			parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			os << "\n\\bar default \n";
		}

		else if (t.cs() == "emph" || t.cs() == "noun") {
			context.check_layout(os);
			os << "\n\\" << t.cs() << " on \n";
			parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			os << "\n\\" << t.cs() << " default \n";
		}

		else if (is_known(t.cs(), known_latex_commands)) {
			context.check_layout(os);
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
			context.check_layout(os);
			os << "\n\\size " << known_coded_sizes[where - known_sizes] << "\n";
		}

		else if (t.cs() == "LyX" || t.cs() == "TeX"
			 || t.cs() == "LaTeX") {
			context.check_layout(os);
			os << t.cs();
			skip_braces(p); // eat {}
		}

		else if (t.cs() == "LaTeXe") {
			context.check_layout(os);
			os << "LaTeX2e";
			skip_braces(p); // eat {}
		}

		else if (t.cs() == "ldots") {
			context.check_layout(os);
			skip_braces(p);
			os << "\\SpecialChar \\ldots{}\n";
		}

		else if (t.cs() == "lyxarrow") {
			context.check_layout(os);
			os << "\\SpecialChar \\menuseparator\n";
			skip_braces(p);
		}

		else if (t.cs() == "ldots") {
			context.check_layout(os);
			os << "\\SpecialChar \\ldots{}\n";
			skip_braces(p);
		}

		else if (t.cs() == "textcompwordmark") {
			context.check_layout(os);
			os << "\\SpecialChar \\textcompwordmark{}\n";
			skip_braces(p);
		}

		else if (t.cs() == "@" && p.next_token().asInput() == ".") {
			context.check_layout(os);
			os << "\\SpecialChar \\@.\n";
			p.get_token();
		}

		else if (t.cs() == "-") {
			context.check_layout(os);
			os << "\\SpecialChar \\-\n";
		}

		else if (t.cs() == "textasciitilde") {
			context.check_layout(os);
			os << '~';
			skip_braces(p);
		}

		else if (t.cs() == "textasciicircum") {
			context.check_layout(os);
			os << '^';
			skip_braces(p);
		}

		else if (t.cs() == "textbackslash") {
			context.check_layout(os);
			os << "\n\\backslash \n";
			skip_braces(p);
		}

		else if (t.cs() == "_" || t.cs() == "&" || t.cs() == "#"
			    || t.cs() == "$" || t.cs() == "{" || t.cs() == "}"
			    || t.cs() == "%") {
			context.check_layout(os);
			os << t.cs();
		}

		else if (t.cs() == "char") {
			context.check_layout(os);
			if (p.next_token().character() == '`') {
				p.get_token();
				if (p.next_token().cs() == "\"") {
					p.get_token();
					os << '"';
					skip_braces(p);
				} else {
					handle_ert(os, "\\char`", context);
				}
			} else {
				handle_ert(os, "\\char", context);
			}
		}

		else if (t.cs() == "\"") {
			context.check_layout(os);
			string const name = p.verbatim_item();
			     if (name == "a") os << 'ä';
			else if (name == "o") os << 'ö';
			else if (name == "u") os << 'ü';
			else if (name == "A") os << 'Ä';
			else if (name == "O") os << 'Ö';
			else if (name == "U") os << 'Ü';
			else handle_ert(os, "\"{" + name + "}", context);
		}

		else if (t.cs() == "=" || t.cs() == "H" || t.cs() == "c"
		      || t.cs() == "^" || t.cs() == "'" || t.cs() == "~") {
			// we need the trim as the LyX parser chokes on such spaces
			context.check_layout(os);
			os << "\n\\i \\" << t.cs() << "{"
			   << trim(parse_text(p, FLAG_ITEM, outer, context), " ") << "}\n";
		}

		else if (t.cs() == "ss") {
			context.check_layout(os);
			os << "ß";
		}

		else if (t.cs() == "i" || t.cs() == "j") {
			context.check_layout(os);
			os << "\\" << t.cs() << ' ';
		}

		else if (t.cs() == "\\") {
			string const next = p.next_token().asInput();
			if (next == "[")
				handle_ert(os, "\\\\" + p.getOpt(), context);
			else if (next == "*") {
				p.get_token();
				handle_ert(os, "\\\\*" + p.getOpt(), context);
			}
			else {
				context.check_layout(os);
				os << "\n\\newline \n";
			}
		}

		else if (t.cs() == "input" || t.cs() == "include"
			 || t.cs() == "verbatiminput") {
			string name = '\\' + t.cs();
			if (t.cs() == "verbatiminput"
			    && p.next_token().asInput() == "*")
				name += p.get_token().asInput();
			context.check_layout(os);
			begin_inset(os, "Include ");
			os << name << '{' << p.getArg('{', '}') << "}\n";
			os << "preview false\n";
			end_inset(os);
		}
		else if (t.cs() == "fancyhead") {
			context.check_layout(os);
			ostringstream ss;
			ss << "\\fancyhead";
			ss << p.getOpt();
			ss << '{' << p.verbatim_item() << "}\n";
			handle_ert(os, ss.str(), context);
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
			handle_ert(os, s + ' ', context);
			*/
			context.check_layout(os);
			handle_ert(os, t.asInput() + ' ', context);
		}

		if (flags & FLAG_LEAVE) {
			flags &= ~FLAG_LEAVE;
			break;
		}
	}
}


// }])
