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
#include "lengthcommon.h"
#include "support/lstrings.h"
#include "support/tostr.h"
#include "support/filetools.h"

#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using lyx::support::rtrim;
using lyx::support::suffixIs;
using lyx::support::contains;

using std::cerr;
using std::endl;

using std::map;
using std::ostream;
using std::ostringstream;
using std::istringstream;
using std::string;
using std::vector;


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


/*!
 * Split a LaTeX length into value and unit.
 * The latter can be a real unit like "pt", or a latex length variable
 * like "\textwidth". The unit may contain additional stuff like glue
 * lengths, but we don't care, because such lengths are ERT anyway.
 * \return true if \param value and \param unit are valid.
 */
bool splitLatexLength(string const & len, string & value, string & unit)
{
	if (len.empty())
		return false;
	const string::size_type i = len.find_first_not_of(" -+0123456789.,");
	//'4,5' is a valid LaTeX length number. Change it to '4.5'
	string const length = lyx::support::subst(len, ',', '.');
	if (i == string::npos)
		return false;
	if (i == 0) {
		if (len[0] == '\\') {
			// We had something like \textwidth without a factor
			value = "1.0";
		} else {
			return false;
		}
	} else {
		value = trim(string(length, 0, i));
	}
	if (value == "-")
		value = "-1.0";
	// 'cM' is a valid LaTeX length unit. Change it to 'cm'
	if (contains(len, '\\'))
		unit = trim(string(len, i));
	else
		unit = lyx::support::lowercase(trim(string(len, i)));
	return true;
}


// A simple function to translate a latex length to something lyx can
// understand. Not perfect, but rather best-effort.
bool translate_len(string const & length, string & valstring, string & unit)
{
	if (!splitLatexLength(length, valstring, unit))
		return false;
	// LyX uses percent values
	double value;
	istringstream iss(valstring);
	iss >> value;
	value *= 100;
	ostringstream oss;
	oss << value;
	string const percentval = oss.str();
	// a normal length
	if (unit.empty() || unit[0] != '\\')
		return true;
	string::size_type const i = unit.find(' ');
	string const endlen = (i == string::npos) ? string() : string(unit, i);
	if (unit == "\\textwidth") {
		valstring = percentval;
		unit = "text%" + endlen;
	} else if (unit == "\\columnwidth") {
		valstring = percentval;
		unit = "col%" + endlen;
	} else if (unit == "\\paperwidth") {
		valstring = percentval;
		unit = "page%" + endlen;
	} else if (unit == "\\linewidth") {
		valstring = percentval;
		unit = "line%" + endlen;
	} else if (unit == "\\paperheight") {
		valstring = percentval;
		unit = "pheight%" + endlen;
	} else if (unit == "\\textheight") {
		valstring = percentval;
		unit = "theight%" + endlen;
	}
	return true;
}


string translate_len(string const & length)
{
	string unit;
	string value;
	if (translate_len(length, value, unit))
		return value + unit;
	// If the input is invalid, return what we have.
	return length;
}


/*!
 * Translates a LaTeX length into \param value, \param unit and
 * \param special parts suitable for a box inset.
 * The difference from translate_len() is that a box inset knows about
 * some special "units" that are stored in \param special.
 */
void translate_box_len(string const & length, string & value, string & unit, string & special)
{
	if (translate_len(length, value, unit)) {
		if (unit == "\\height" || unit == "\\depth" ||
		    unit == "\\totalheight" || unit == "\\width") {
			special = unit.substr(1);
			// The unit is not used, but LyX requires a dummy setting
			unit = "in";
		} else
			special = "none";
	} else {
		value.clear();
		unit = length;
		special = "none";
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


void handle_ert(ostream & os, string const & s, Context & context, bool check_layout = true)
{
	if (check_layout) {
		// We must have a valid layout before outputting the ERT inset.
		context.check_layout(os);
	}
	Context newcontext(true, context.textclass);
	begin_inset(os, "ERT");
	os << "\nstatus collapsed\n";
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


void handle_comment(ostream & os, string const & s, Context & context)
{
	// TODO: Handle this better
	Context newcontext(true, context.textclass);
	begin_inset(os, "ERT");
	os << "\nstatus collapsed\n";
	newcontext.check_layout(os);
	for (string::const_iterator it = s.begin(), et = s.end(); it != et; ++it) {
		if (*it == '\\')
			os << "\n\\backslash \n";
		else
			os << *it;
	}
	// make sure that our comment is the last thing on the line
	os << "\n\\newline";
	newcontext.check_end_layout(os);
	end_inset(os);
}


class isLayout : public std::unary_function<LyXLayout_ptr, bool> {
public:
	isLayout(string const name) : name_(name) {}
	bool operator()(LyXLayout_ptr const & ptr) const {
		return ptr->latexname() == name_;
	}
private:
	string const name_;
};


LyXLayout_ptr findLayout(LyXTextClass const & textclass,
			 string const & name)
{
	LyXTextClass::const_iterator beg  = textclass.begin();
	LyXTextClass::const_iterator end = textclass.end();

	LyXTextClass::const_iterator
		it = std::find_if(beg, end, isLayout(name));

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
		p.skip_spaces();
		if (p.next_token().character() == '[') {
			p.get_token(); // eat '['
			begin_inset(os, "OptArg\n");
			os << "status collapsed\n\n";
			parse_text_in_inset(p, os, FLAG_BRACK_LAST, outer, context);
			end_inset(os);
		}
	}
	parse_text_snippet(p, os, FLAG_ITEM, outer, context);
	context.check_end_layout(os);
	context.check_end_deeper(os);
	// We don't need really a new paragraph, but
	// we must make sure that the next item gets a \begin_layout.
	parent_context.new_paragraph(os);
}


/*!
 * Output a space if necessary.
 * This function gets called for every whitespace token.
 *
 * We have three cases here:
 * 1. A space must be suppressed. Example: The lyxcode case below
 * 2. A space may be suppressed. Example: Spaces before "\par"
 * 3. A space must not be suppressed. Example: A space between two words
 *
 * We currently handle only 1. and 3 and from 2. only the case of
 * spaces before newlines as a side effect.
 *
 * 2. could be used to suppress as many spaces as possible. This has two effects:
 * - Reimporting LyX generated LaTeX files changes almost no whitespace
 * - Superflous whitespace from non LyX generated LaTeX files is removed.
 * The drawback is that the logic inside the function becomes
 * complicated, and that is the reason why it is not implemented.
 */
void check_space(Parser const & p, ostream & os, Context & context)
{
	Token const next = p.next_token();
	Token const curr = p.curr_token();
	// A space before a single newline and vice versa must be ignored
	// LyX emits a newline before \end{lyxcode}.
	// This newline must be ignored,
	// otherwise LyX will add an additional protected space.
	if (next.cat() == catSpace ||
	    next.cat() == catNewline ||
	    (next.cs() == "end" && context.layout->free_spacing && curr.cat() == catNewline)) {
		return;
	}
	context.check_layout(os);
	os << ' ';
}


/*!
 * Check wether \param command is a known command. If yes,
 * handle the command with all arguments.
 * \return true if the command was parsed, false otherwise.
 */
bool parse_command(string const & command, Parser & p, ostream & os,
                   bool outer, Context & context)
{
	if (known_commands.find(command) != known_commands.end()) {
		vector<ArgumentType> const & template_arguments = known_commands[command];
		string ert = command;
		size_t no_arguments = template_arguments.size();
		for (size_t i = 0; i < no_arguments; ++i) {
			switch (template_arguments[i]) {
			case required:
				// This argument contains regular LaTeX
				handle_ert(os, ert + '{', context);
				parse_text(p, os, FLAG_ITEM, outer, context);
				ert = "}";
				break;
			case verbatim:
				// This argument may contain special characters
				ert += '{' + p.verbatim_item() + '}';
				break;
			case optional:
				ert += p.getOpt();
				break;
			}
		}
		handle_ert(os, ert, context);
		return true;
	}
	return false;
}


/// Parses a minipage or parbox
void parse_box(Parser & p, ostream & os, unsigned flags, bool outer,
               Context & parent_context, bool use_parbox)
{
	string position;
	string inner_pos;
	string height_value = "0";
	string height_unit = "pt";
	string height_special = "none";
	string latex_height;
	if (p.next_token().asInput() == "[") {
		position = p.getArg('[', ']');
		if (position != "t" && position != "c" && position != "b") {
			position = "c";
			cerr << "invalid position for minipage/parbox" << endl;
		}
		if (p.next_token().asInput() == "[") {
			latex_height = p.getArg('[', ']');
			translate_box_len(latex_height, height_value, height_unit, height_special);

			if (p.next_token().asInput() == "[") {
				inner_pos = p.getArg('[', ']');
				if (inner_pos != "c" && inner_pos != "t" &&
				    inner_pos != "b" && inner_pos != "s") {
					inner_pos = position;
					cerr << "invalid inner_pos for minipage/parbox"
					     << endl;
				}
			}
		}
	}
	string width_value;
	string width_unit;
	string const latex_width = p.verbatim_item();
	translate_len(latex_width, width_value, width_unit);
	if (contains(width_unit, "\\") || contains(height_unit, "\\")) {
		// LyX can't handle length variables
		ostringstream ss;
		if (use_parbox)
			ss << "\\parbox";
		else
			ss << "\\begin{minipage}";
		if (!position.empty())
			ss << '[' << position << ']';
		if (!latex_height.empty())
			ss << '[' << latex_height << ']';
		if (!inner_pos.empty())
			ss << '[' << inner_pos << ']';
		ss << "{" << latex_width << "}";
		if (use_parbox)
			ss << '{';
		handle_ert(os, ss.str(), parent_context);
		parent_context.new_paragraph(os);
		parse_text_in_inset(p, os, flags, outer, parent_context);
		if (use_parbox)
			handle_ert(os, "}", parent_context);
		else
			handle_ert(os, "\\end{minipage}", parent_context);
	} else {
		// LyX does not like empty positions, so we have
		// to set them to the LaTeX default values here.
		if (position.empty())
			position = "c";
		if (inner_pos.empty())
			inner_pos = position;
		parent_context.check_layout(os);
		begin_inset(os, "Box Frameless\n");
		os << "position \"" << position << "\"\n";
		os << "hor_pos \"c\"\n";
		os << "has_inner_box 1\n";
		os << "inner_pos \"" << inner_pos << "\"\n";
		os << "use_parbox " << use_parbox << "\n";
		os << "width \"" << width_value << width_unit << "\"\n";
		os << "special \"none\"\n";
		os << "height \"" << height_value << height_unit << "\"\n";
		os << "height_special \"" << height_special << "\"\n";
		os << "status open\n\n";
		parse_text_in_inset(p, os, flags, outer, parent_context);
		end_inset(os);
#ifdef PRESERVE_LAYOUT
		// lyx puts a % after the end of the minipage
		if (p.next_token().cat() == catNewline && p.next_token().cs().size() > 1) {
			// new paragraph
			//handle_comment(os, "%dummy", parent_context);
			p.get_token();
			p.skip_spaces();
			parent_context.new_paragraph(os);
		}
		else if (p.next_token().cat() == catSpace || p.next_token().cat() == catNewline) {
			//handle_comment(os, "%dummy", parent_context);
			p.get_token();
			p.skip_spaces();
			// We add a protected space if something real follows
			if (p.good() && p.next_token().cat() != catComment) {
				os << "\\InsetSpace ~\n";
			}
		}
#endif
	}
}


void parse_environment(Parser & p, ostream & os, bool outer,
		       Context & parent_context)
{
	LyXLayout_ptr newlayout;
	string const name = p.getArg('{', '}');
	const bool is_starred = suffixIs(name, '*');
	string const unstarred_name = rtrim(name, "*");
	active_environments.push_back(name);
	p.skip_spaces();

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
		   << "\nstatus open\n\n";
		parse_text_in_inset(p, os, FLAG_END, outer, parent_context);
		end_inset(os);
		// We don't need really a new paragraph, but
		// we must make sure that the next item gets a \begin_layout.
		parent_context.new_paragraph(os);
	}

	else if (name == "minipage")
		parse_box(p, os, FLAG_END, outer, parent_context, false);

	// Alignment settings
	else if (name == "center" || name == "flushleft" || name == "flushright" ||
	         name == "centering" || name == "raggedright" || name == "raggedleft") {
		// We must begin a new paragraph if not already done
		if (! parent_context.atParagraphStart()) {
			parent_context.check_end_layout(os);
			parent_context.new_paragraph(os);
		}
		if (name == "flushleft" || name == "raggedright")
			parent_context.add_extra_stuff("\\align left ");
		else if (name == "flushright" || name == "raggedleft")
			parent_context.add_extra_stuff("\\align right ");
		else
			parent_context.add_extra_stuff("\\align center ");
		parse_text(p, os, FLAG_END, outer, parent_context);
		// Just in case the environment is empty ..
		parent_context.extra_stuff.erase();
		// We must begin a new paragraph to reset the alignment
		parent_context.new_paragraph(os);
	}

	// The single '=' is meant here.
	else if ((newlayout = findLayout(parent_context.textclass, name)).get() &&
		   newlayout->isEnvironment()) {
		Context context(true, parent_context.textclass, newlayout,
				parent_context.layout);
		parent_context.check_end_layout(os);
		switch (context.layout->latextype) {
		case  LATEX_LIST_ENVIRONMENT:
			context.extra_stuff = "\\labelwidthstring "
				+ p.verbatim_item() + '\n';
			p.skip_spaces();
			break;
		case  LATEX_BIB_ENVIRONMENT:
			p.verbatim_item(); // swallow next arg
			p.skip_spaces();
			break;
		default:
			break;
		}
		context.check_deeper(os);
		parse_text(p, os, FLAG_END, outer, context);
		context.check_end_layout(os);
		context.check_end_deeper(os);
		parent_context.new_paragraph(os);
	}

	else if (name == "appendix") {
		// This is no good latex style, but it works and is used in some documents...
		parent_context.check_end_layout(os);
		Context context(true, parent_context.textclass, parent_context.layout,
				parent_context.layout);
		context.check_layout(os);
		os << "\\start_of_appendix\n";
		parse_text(p, os, FLAG_END, outer, context);
		context.check_end_layout(os);
	}

	else if (name == "comment") {
		parent_context.check_layout(os);
		begin_inset(os, "Note Comment\n");
		os << "status open\n";
		parse_text_in_inset(p, os, FLAG_END, outer, parent_context);
		end_inset(os);
	}

	else if (name == "lyxgreyedout") {
		parent_context.check_layout(os);
		begin_inset(os, "Note Greyedout\n");
		os << "status open\n";
		parse_text_in_inset(p, os, FLAG_END, outer, parent_context);
		end_inset(os);
	}

	else if (name == "tabbing") {
		// We need to remember that we have to handle '\=' specially
		handle_ert(os, "\\begin{" + name + "}", parent_context);
		parse_text_snippet(p, os, FLAG_END | FLAG_TABBING, outer, parent_context);
		handle_ert(os, "\\end{" + name + "}", parent_context);
	}

	else {
		handle_ert(os, "\\begin{" + name + "}", parent_context);
		parse_text_snippet(p, os, FLAG_END, outer, parent_context);
		handle_ert(os, "\\end{" + name + "}", parent_context);
	}

	active_environments.pop_back();
	if (name != "math")
		p.skip_spaces();
}

} // anonymous namespace




void parse_text(Parser & p, ostream & os, unsigned flags, bool outer,
		Context & context)
{
	LyXLayout_ptr newlayout;
	// Store the latest bibliographystyle (needed for bibtex inset)
	string bibliographystyle;
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

		else if (t.cat() == catSpace || (t.cat() == catNewline && t.cs().size() == 1))
			check_space(p, os, context);

		else if (t.cat() == catLetter ||
			       t.cat() == catOther ||
			       t.cat() == catAlign ||
			       t.cat() == catParameter) {
			context.check_layout(os);
			os << t.character();
		}

		else if (t.cat() == catNewline || (t.cat() == catEscape && t.cs() == "par")) {
			p.skip_spaces();
			context.new_paragraph(os);
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
			// special handling of size changes
			context.check_layout(os);
			bool const is_size = is_known(p.next_token().cs(), known_sizes);
			Token const prev = p.prev_token();
			string const s = parse_text(p, FLAG_BRACE_LAST, outer, context);
			if (s.empty() && (p.next_token().character() == '`' ||
			                  (prev.character() == '-' && p.next_token().character())))
				; // ignore it in {}`` or -{}-
			else if (is_size || s == "[" || s == "]" || s == "*")
				os << s;
			else {
				handle_ert(os, "{", context, false);
				// s will end the current layout and begin a new one if necessary
				os << s;
				handle_ert(os, "}", context);
			}
		}

		else if (t.cat() == catEnd) {
			if (flags & FLAG_BRACE_LAST) {
				return;
			}
			cerr << "stray '}' in text\n";
			handle_ert(os, "}", context);
		}

		else if (t.cat() == catComment) {
			context.check_layout(os);
			if (!t.cs().empty()) {
				handle_comment(os, '%' + t.cs(), context);
				if (p.next_token().cat() == catNewline) {
					// A newline after a comment line starts a new paragraph
					context.new_paragraph(os);
					p.skip_spaces();
				}
			} else {
				// "%\n" combination
				p.skip_spaces();
			}
		}

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
				return;
			}
			p.error("found 'end' unexpectedly");
		}

		else if (t.cs() == "item") {
			p.skip_spaces();
			string s;
			bool optarg = false;
			if (p.next_token().character() == '[') {
				p.get_token(); // eat '['
				Context newcontext(false, context.textclass);
				s = parse_text(p, FLAG_BRACK_LAST, outer, newcontext);
				optarg = true;
			}
			context.set_item();
			context.check_layout(os);
			if (optarg) {
				if (context.layout->labeltype != LABEL_MANUAL) {
					// lyx does not support \item[\mybullet] in itemize environments
					handle_ert(os, "[", context);
					os << s;
					handle_ert(os, "]", context);
				} else if (!s.empty()) {
					// The space is needed to separate the item from the rest of the sentence.
					os << s << ' ';
					p.skip_spaces();
				}
			}
		}

		else if (t.cs() == "bibitem") {
			context.set_item();
			context.check_layout(os);
			os << "\\bibitem ";
			os << p.getOpt();
			os << '{' << p.verbatim_item() << '}' << "\n";
		}

		else if (t.cs() == "def") {
			p.skip_spaces();
			context.check_layout(os);
			string name = p.get_token().cs();
			while (p.next_token().cat() != catBegin)
				name += p.get_token().asString();
			handle_ert(os, "\\def\\" + name + '{' + p.verbatim_item() + '}', context);
		}

		else if (t.cs() == "noindent") {
			p.skip_spaces();
			context.add_extra_stuff("\\noindent ");
		}

		else if (t.cs() == "appendix") {
			p.skip_spaces();
			context.add_extra_stuff("\\start_of_appendix ");
		}

		// Must attempt to parse "Section*" before "Section".
		else if ((p.next_token().asInput() == "*") &&
			 // The single '=' is meant here.
			 (newlayout = findLayout(context.textclass,
						 t.cs() + '*')).get() &&
			 newlayout->isCommand()) {
			p.get_token();
			output_command_layout(os, p, outer, context, newlayout);
			p.skip_spaces();
		}

		// The single '=' is meant here.
		else if ((newlayout = findLayout(context.textclass, t.cs())).get() &&
			 newlayout->isCommand()) {
			output_command_layout(os, p, outer, context, newlayout);
			p.skip_spaces();
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
			if (opts.find("scale") != opts.end()) {
				istringstream iss(opts["scale"]);
				double val;
				iss >> val;
				val = val*100;
				os << "\tscale " << val << '\n';
			}
			if (opts.find("angle") != opts.end())
				os << "\trotateAngle "
				   << opts["angle"] << '\n';
			if (opts.find("origin") != opts.end()) {
				ostringstream ss;
				string const opt = opts["origin"];
				if (opt.find('l') != string::npos) ss << "left";
				if (opt.find('r') != string::npos) ss << "right";
				if (opt.find('c') != string::npos) ss << "center";
				if (opt.find('t') != string::npos) ss << "Top";
				if (opt.find('b') != string::npos) ss << "Bottom";
				if (opt.find('B') != string::npos) ss << "Baseline";
				if (!ss.str().empty())
					os << "\trotateOrigin " << ss.str() << '\n';
				else
					cerr << "Warning: Ignoring unknown includegraphics origin argument '" << opt << "'\n";
			}
			if (opts.find("keepaspectratio") != opts.end())
				os << "\tkeepAspectRatio\n";
			if (opts.find("clip") != opts.end())
				os << "\tclip\n";
			if (opts.find("draft") != opts.end())
				os << "\tdraft\n";
			if (opts.find("bb") != opts.end())
				os << "\tBoundingBox "
				   << opts["bb"] << '\n';
			int numberOfbbOptions = 0;
			if (opts.find("bbllx") != opts.end())
				numberOfbbOptions++;
			if (opts.find("bblly") != opts.end())
				numberOfbbOptions++;
			if (opts.find("bburx") != opts.end())
				numberOfbbOptions++;
			if (opts.find("bbury") != opts.end())
				numberOfbbOptions++;
			if (numberOfbbOptions == 4)
				os << "\tBoundingBox "
				   << opts["bbllx"] << opts["bblly"]
				   << opts["bburx"] << opts["bbury"] << '\n';
			else if (numberOfbbOptions > 0)
				cerr << "Warning: Ignoring incomplete includegraphics boundingbox arguments.\n";
			numberOfbbOptions = 0;
			if (opts.find("natwidth") != opts.end())
				numberOfbbOptions++;
			if (opts.find("natheight") != opts.end())
				numberOfbbOptions++;
			if (numberOfbbOptions == 2)
				os << "\tBoundingBox 0bp 0bp "
				   << opts["natwidth"] << opts["natheight"] << '\n';
			else if (numberOfbbOptions > 0)
				cerr << "Warning: Ignoring incomplete includegraphics boundingbox arguments.\n";
			ostringstream special;
			if (opts.find("hiresbb") != opts.end())
				special << "hiresbb,";
			if (opts.find("trim") != opts.end())
				special << "trim,";
			if (opts.find("viewport") != opts.end())
				special << "viewport=" << opts["viewport"] << ',';
			if (opts.find("totalheight") != opts.end())
				special << "totalheight=" << opts["totalheight"] << ',';
			if (opts.find("type") != opts.end())
				special << "type=" << opts["type"] << ',';
			if (opts.find("ext") != opts.end())
				special << "ext=" << opts["ext"] << ',';
			if (opts.find("read") != opts.end())
				special << "read=" << opts["read"] << ',';
			if (opts.find("command") != opts.end())
				special << "command=" << opts["command"] << ',';
			string s_special = special.str();
			if (!s_special.empty()) {
				// We had special arguments. Remove the trailing ','.
				os << "\tspecial " << s_special.substr(0, s_special.size() - 1) << '\n';
			}
			// TODO: Handle the unknown settings better.
			// Warn about invalid options.
			// Check wether some option was given twice.
			end_inset(os);
		}

		else if (t.cs() == "footnote") {
			p.skip_spaces();
			context.check_layout(os);
			begin_inset(os, "Foot\n");
			os << "status collapsed\n\n";
			parse_text_in_inset(p, os, FLAG_ITEM, false, context);
			end_inset(os);
		}

		else if (t.cs() == "marginpar") {
			p.skip_spaces();
			context.check_layout(os);
			begin_inset(os, "Marginal\n");
			os << "status collapsed\n\n";
			parse_text_in_inset(p, os, FLAG_ITEM, false, context);
			end_inset(os);
		}

		else if (t.cs() == "ensuremath") {
			p.skip_spaces();
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
			p.skip_spaces();
		}

		else if (t.cs() == "makeindex" || t.cs() == "maketitle") {
			p.skip_spaces();
			skip_braces(p); // swallow this
		}

		else if (t.cs() == "tableofcontents") {
			p.skip_spaces();
			context.check_layout(os);
			begin_inset(os, "LatexCommand \\tableofcontents\n");
			end_inset(os);
			skip_braces(p); // swallow this
		}

		else if (t.cs() == "listoffigures") {
			p.skip_spaces();
			context.check_layout(os);
			begin_inset(os, "FloatList figure\n");
			end_inset(os);
			skip_braces(p); // swallow this
		}

		else if (t.cs() == "listoftables") {
			p.skip_spaces();
			context.check_layout(os);
			begin_inset(os, "FloatList table\n");
			end_inset(os);
			skip_braces(p); // swallow this
		}

		else if (t.cs() == "listof") {
			p.skip_spaces(true);
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

		else if (t.cs() == "textsl") {
			context.check_layout(os);
			os << "\n\\shape slanted \n";
			parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			os << "\n\\shape default \n";
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
			context.check_layout(os);
			begin_inset(os, "Quotes ");
			os << known_coded_quotes[where - known_quotes];
			end_inset(os);
			skip_braces(p);
		}

		else if (is_known(t.cs(), known_sizes)) {
			char const ** where = is_known(t.cs(), known_sizes);
			context.check_layout(os);
			os << "\n\\size " << known_coded_sizes[where - known_sizes] << "\n";
			p.skip_spaces();
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

		// Problem: \= creates a tabstop inside the tabbing environment
		// and else an accent. In the latter case we really would want
		// \={o} instead of \= o.
		else if (t.cs() == "=" && (flags & FLAG_TABBING))
			handle_ert(os, t.asInput(), context);

		else if (t.cs() == "H" || t.cs() == "c" || t.cs() == "^" || t.cs() == "'"
		      || t.cs() == "~" || t.cs() == "." || t.cs() == "=") {
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
			context.check_layout(os);
			string const next = p.next_token().asInput();
			if (next == "[")
				handle_ert(os, "\\\\" + p.getOpt(), context);
			else if (next == "*") {
				p.get_token();
				handle_ert(os, "\\\\*" + p.getOpt(), context);
			}
			else {
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
			string filename(p.getArg('{', '}'));
			string lyxname(lyx::support::ChangeExtension(filename, ".lyx"));
			if (tex2lyx(filename, lyxname)) {
				os << name << '{' << lyxname << "}\n";
			} else {
				os << name << '{' << filename << "}\n";
			}
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

		else if (t.cs() == "bibliographystyle") {
			// store new bibliographystyle
			bibliographystyle = p.verbatim_item();
			// output new bibliographystyle.
			// This is only necessary if used in some other macro than \bibliography.
			handle_ert(os, "\\bibliographystyle{" + bibliographystyle + "}", context);
		}

		else if (t.cs() == "bibliography") {
			context.check_layout(os);
			begin_inset(os, "LatexCommand ");
			os << "\\bibtex";
			// Do we have a bibliographystyle set?
			if (!bibliographystyle.empty()) {
				os << '[' << bibliographystyle << ']';
			}
			os << '{' << p.verbatim_item() << "}\n";
			end_inset(os);
		}

		else if (t.cs() == "parbox")
			parse_box(p, os, FLAG_ITEM, outer, context, true);

		else if (t.cs() == "smallskip" ||
		         t.cs() == "medskip" ||
			 t.cs() == "bigskip" ||
			 t.cs() == "vfill") {
			context.check_layout(os);
			begin_inset(os, "VSpace ");
			os << t.cs();
			end_inset(os);
		}

		else if (t.cs() == "vspace") {
			bool starred = false;
			if (p.next_token().asInput() == "*") {
				p.get_token();
				starred = true;
			}
			string const length = p.verbatim_item();
			string unit;
			string valstring;
			bool valid = splitLatexLength(length, valstring, unit);
			bool known_vspace = false;
			bool known_unit = false;
			double value;
			if (valid) {
				istringstream iss(valstring);
				iss >> value;
				if (value == 1.0) {
					if (unit == "\\smallskipamount") {
						unit = "smallskip";
						known_vspace = true;
					} else if (unit == "\\medskipamount") {
						unit = "medskip";
						known_vspace = true;
					} else if (unit == "\\bigskipamount") {
						unit = "bigskip";
						known_vspace = true;
					} else if (unit == "\\fill") {
						unit = "vfill";
						known_vspace = true;
					}
				} else {
					switch (unitFromString(unit)) {
					case LyXLength::SP:
					case LyXLength::PT:
					case LyXLength::BP:
					case LyXLength::DD:
					case LyXLength::MM:
					case LyXLength::PC:
					case LyXLength::CC:
					case LyXLength::CM:
					case LyXLength::IN:
					case LyXLength::EX:
					case LyXLength::EM:
					case LyXLength::MU:
						known_unit = true;
						break;
					default:
						break;
					}
				}
			}

			if (known_unit || known_vspace) {
				// Literal length or known variable
				context.check_layout(os);
				begin_inset(os, "VSpace ");
				if (known_unit)
					os << value;
				os << unit;
				if (starred)
					os << '*';
				end_inset(os);
			} else {
				// LyX can't handle other length variables in Inset VSpace
				string name = t.asInput();
				if (starred)
					name += '*';
				if (valid) {
					if (value == 1.0)
						handle_ert(os, name + '{' + unit + '}', context);
					else if (value == -1.0)
						handle_ert(os, name + "{-" + unit + '}', context);
					else
						handle_ert(os, name + '{' + valstring + unit + '}', context);
				} else
					handle_ert(os, name + '{' + length + '}', context);
			}
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
			string name = t.asInput();
			if (p.next_token().asInput() == "*") {
				// Starred commands like \vspace*{}
				p.get_token();				// Eat '*'
				name += '*';
			}
			if (! parse_command(name, p, os, outer, context))
				handle_ert(os, name, context);
		}

		if (flags & FLAG_LEAVE) {
			flags &= ~FLAG_LEAVE;
			break;
		}
	}
}


// }])
