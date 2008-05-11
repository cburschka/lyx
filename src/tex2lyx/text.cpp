/**
 * \file tex2lyx/text.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Jean-Marc Lasgouttes
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

// {[(

#include <config.h>

#include "tex2lyx.h"
#include "Context.h"
#include "FloatList.h"
#include "lengthcommon.h"
#include "support/lstrings.h"
#include "support/convert.h"
#include "support/filetools.h"

#include <boost/filesystem/operations.hpp>
#include <boost/tuple/tuple.hpp>

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

namespace lyx {

using support::addExtension;
using support::changeExtension;
using support::doesFileExist;
using support::FileName;
using support::makeAbsPath;
using support::makeRelPath;
using support::rtrim;
using support::suffixIs;
using support::contains;
using support::subst;

namespace fs = boost::filesystem;


void parse_text_in_inset(Parser & p, ostream & os, unsigned flags, bool outer,
		Context const & context)
{
	Context newcontext(true, context.textclass);
	newcontext.font = context.font;
	parse_text(p, os, flags, outer, newcontext);
	newcontext.check_end_layout(os);
}


namespace {

/// parses a paragraph snippet, useful for example for \\emph{...}
void parse_text_snippet(Parser & p, ostream & os, unsigned flags, bool outer,
		Context & context)
{
	Context newcontext(context);
	// Don't inherit the paragraph-level extra stuff
	newcontext.par_extra_stuff.clear();
	parse_text(p, os, flags, outer, newcontext);
	// Make sure that we don't create invalid .lyx files
	context.need_layout = newcontext.need_layout;
	context.need_end_layout = newcontext.need_end_layout;
}


/*!
 * Thin wrapper around parse_text_snippet() using a string.
 *
 * We completely ignore \c context.need_layout and \c context.need_end_layout,
 * because our return value is not used directly (otherwise the stream version
 * of parse_text_snippet() could be used). That means that the caller needs
 * to do layout management manually.
 * This is intended to parse text that does not create any layout changes.
 */
string parse_text_snippet(Parser & p, unsigned flags, const bool outer,
		  Context & context)
{
	Context newcontext(context);
	newcontext.need_layout = false;
	newcontext.need_end_layout = false;
	newcontext.new_layout_allowed = false;
	// Avoid warning by Context::~Context()
	newcontext.par_extra_stuff.clear();
	ostringstream os;
	parse_text_snippet(p, os, flags, outer, newcontext);
	return os.str();
}


char const * const known_latex_commands[] = { "ref", "cite", "label",
 "index", "printindex", "pageref", "url", "vref", "vpageref", "prettyref",
 "eqref", 0 };

/*!
 * natbib commands.
 * We can't put these into known_latex_commands because the argument order
 * is reversed in lyx if there are 2 arguments.
 * The starred forms are also known.
 */
char const * const known_natbib_commands[] = { "cite", "citet", "citep",
"citealt", "citealp", "citeauthor", "citeyear", "citeyearpar",
"citefullauthor", "Citet", "Citep", "Citealt", "Citealp", "Citeauthor", 0 };

/*!
 * jurabib commands.
 * We can't put these into known_latex_commands because the argument order
 * is reversed in lyx if there are 2 arguments.
 * No starred form other than "cite*" known.
 */
char const * const known_jurabib_commands[] = { "cite", "citet", "citep",
"citealt", "citealp", "citeauthor", "citeyear", "citeyearpar",
// jurabib commands not (yet) supported by LyX:
// "fullcite",
// "footcite", "footcitet", "footcitep", "footcitealt", "footcitealp",
// "footciteauthor", "footciteyear", "footciteyearpar",
"citefield", "citetitle", "cite*", 0 };

/// LaTeX names for quotes
char const * const known_quotes[] = { "dq", "guillemotleft", "flqq", "og",
"guillemotright", "frqq", "fg", "glq", "glqq", "textquoteleft", "grq", "grqq",
"quotedblbase", "textquotedblleft", "quotesinglbase", "textquoteright", "flq",
"guilsinglleft", "frq", "guilsinglright", 0};

/// the same as known_quotes with .lyx names
char const * const known_coded_quotes[] = { "prd", "ard", "ard", "ard",
"ald", "ald", "ald", "gls", "gld", "els", "els", "grd",
"gld", "grd", "gls", "ers", "fls",
"fls", "frs", "frs", 0};

/// LaTeX names for font sizes
char const * const known_sizes[] = { "tiny", "scriptsize", "footnotesize",
"small", "normalsize", "large", "Large", "LARGE", "huge", "Huge", 0};

/// the same as known_sizes with .lyx names
char const * const known_coded_sizes[] = { "default", "tiny", "scriptsize", "footnotesize",
"small", "normal", "large", "larger", "largest", "huge", "giant", 0};

/// LaTeX 2.09 names for font families
char const * const known_old_font_families[] = { "rm", "sf", "tt", 0};

/// LaTeX names for font families
char const * const known_font_families[] = { "rmfamily", "sffamily",
"ttfamily", 0};

/// the same as known_old_font_families and known_font_families with .lyx names
char const * const known_coded_font_families[] = { "roman", "sans",
"typewriter", 0};

/// LaTeX 2.09 names for font series
char const * const known_old_font_series[] = { "bf", 0};

/// LaTeX names for font series
char const * const known_font_series[] = { "bfseries", "mdseries", 0};

/// the same as known_old_font_series and known_font_series with .lyx names
char const * const known_coded_font_series[] = { "bold", "medium", 0};

/// LaTeX 2.09 names for font shapes
char const * const known_old_font_shapes[] = { "it", "sl", "sc", 0};

/// LaTeX names for font shapes
char const * const known_font_shapes[] = { "itshape", "slshape", "scshape",
"upshape", 0};

/// the same as known_old_font_shapes and known_font_shapes with .lyx names
char const * const known_coded_font_shapes[] = { "italic", "slanted",
"smallcaps", "up", 0};

/*!
 * Graphics file extensions known by the dvips driver of the graphics package.
 * These extensions are used to complete the filename of an included
 * graphics file if it does not contain an extension.
 * The order must be the same that latex uses to find a file, because we
 * will use the first extension that matches.
 * This is only an approximation for the common cases. If we would want to
 * do it right in all cases, we would need to know which graphics driver is
 * used and know the extensions of every driver of the graphics package.
 */
char const * const known_dvips_graphics_formats[] = {"eps", "ps", "eps.gz",
"ps.gz", "eps.Z", "ps.Z", 0};

/*!
 * Graphics file extensions known by the pdftex driver of the graphics package.
 * \sa known_dvips_graphics_formats
 */
char const * const known_pdftex_graphics_formats[] = {"png", "pdf", "jpg",
"mps", "tif", 0};

/*!
 * Known file extensions for TeX files as used by \\include.
 */
char const * const known_tex_extensions[] = {"tex", 0};

/// spaces known by InsetSpace
char const * const known_spaces[] = { " ", "space", ",", "thinspace", "quad",
"qquad", "enspace", "enskip", "negthinspace", 0};

/// the same as known_spaces with .lyx names
char const * const known_coded_spaces[] = { "space{}", "space{}",
"thinspace{}", "thinspace{}", "quad{}", "qquad{}", "enspace{}", "enskip{}",
"negthinspace{}", 0};


/// splits "x=z, y=b" into a map
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
 * \returns true if \p value and \p unit are valid.
 */
bool splitLatexLength(string const & len, string & value, string & unit)
{
	if (len.empty())
		return false;
	const string::size_type i = len.find_first_not_of(" -+0123456789.,");
	//'4,5' is a valid LaTeX length number. Change it to '4.5'
	string const length = subst(len, ',', '.');
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
		unit = support::ascii_lowercase(trim(string(len, i)));
	return true;
}


/// A simple function to translate a latex length to something lyx can
/// understand. Not perfect, but rather best-effort.
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


namespace {

/*!
 * Translates a LaTeX length into \p value, \p unit and
 * \p special parts suitable for a box inset.
 * The difference from translate_len() is that a box inset knows about
 * some special "units" that are stored in \p special.
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


/*!
 * Find a file with basename \p name in path \p path and an extension
 * in \p extensions.
 */
string find_file(string const & name, string const & path,
		 char const * const * extensions)
{
	// FIXME UNICODE encoding of name and path may be wrong (makeAbsPath
	// expects utf8)
	for (char const * const * what = extensions; *what; ++what) {
		string const trial = addExtension(name, *what);
		if (doesFileExist(makeAbsPath(trial, path)))
			return trial;
	}
	return string();
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


void handle_ert(ostream & os, string const & s, Context & context)
{
	// We must have a valid layout before outputting the ERT inset.
	context.check_layout(os);
	Context newcontext(true, context.textclass);
	begin_inset(os, "ERT");
	os << "\nstatus collapsed\n";
	newcontext.check_layout(os);
	for (string::const_iterator it = s.begin(), et = s.end(); it != et; ++it) {
		if (*it == '\\')
			os << "\n\\backslash\n";
		else if (*it == '\n') {
			newcontext.new_paragraph(os);
			newcontext.check_layout(os);
		} else
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
			os << "\n\\backslash\n";
		else
			os << *it;
	}
	// make sure that our comment is the last thing on the line
	newcontext.new_paragraph(os);
	newcontext.check_layout(os);
	newcontext.check_end_layout(os);
	end_inset(os);
}


class isLayout : public std::unary_function<Layout_ptr, bool> {
public:
	isLayout(string const name) : name_(name) {}
	bool operator()(Layout_ptr const & ptr) const {
		return ptr->latexname() == name_;
	}
private:
	string const name_;
};


Layout_ptr findLayout(TextClass const & textclass,
			 string const & name)
{
	TextClass::const_iterator beg = textclass.begin();
	TextClass::const_iterator end = textclass.end();

	TextClass::const_iterator
		it = std::find_if(beg, end, isLayout(name));

	return (it == end) ? Layout_ptr() : *it;
}


void eat_whitespace(Parser &, ostream &, Context &, bool);


void output_command_layout(ostream & os, Parser & p, bool outer,
			   Context & parent_context,
			   Layout_ptr newlayout)
{
	parent_context.check_end_layout(os);
	Context context(true, parent_context.textclass, newlayout,
			parent_context.layout, parent_context.font);
	if (parent_context.deeper_paragraph) {
		// We are beginning a nested environment after a
		// deeper paragraph inside the outer list environment.
		// Therefore we don't need to output a "begin deeper".
		context.need_end_deeper = true;
	}
	context.check_deeper(os);
	context.check_layout(os);
	if (context.layout->optionalargs > 0) {
		eat_whitespace(p, os, context, false);
		if (p.next_token().character() == '[') {
			p.get_token(); // eat '['
			begin_inset(os, "OptArg\n");
			os << "status collapsed\n\n";
			parse_text_in_inset(p, os, FLAG_BRACK_LAST, outer, context);
			end_inset(os);
			eat_whitespace(p, os, context, false);
		}
	}
	parse_text(p, os, FLAG_ITEM, outer, context);
	context.check_end_layout(os);
	if (parent_context.deeper_paragraph) {
		// We must suppress the "end deeper" because we
		// suppressed the "begin deeper" above.
		context.need_end_deeper = false;
	}
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
 * Parse all arguments of \p command
 */
void parse_arguments(string const & command,
		     vector<ArgumentType> const & template_arguments,
		     Parser & p, ostream & os, bool outer, Context & context)
{
	string ert = command;
	size_t no_arguments = template_arguments.size();
	for (size_t i = 0; i < no_arguments; ++i) {
		switch (template_arguments[i]) {
		case required:
			// This argument contains regular LaTeX
			handle_ert(os, ert + '{', context);
			eat_whitespace(p, os, context, false);
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
}


/*!
 * Check whether \p command is a known command. If yes,
 * handle the command with all arguments.
 * \return true if the command was parsed, false otherwise.
 */
bool parse_command(string const & command, Parser & p, ostream & os,
		   bool outer, Context & context)
{
	if (known_commands.find(command) != known_commands.end()) {
		parse_arguments(command, known_commands[command], p, os,
				outer, context);
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
	// We need to set the height to the LaTeX default of 1\\totalheight
	// for the case when no height argument is given
	string height_value = "1";
	string height_unit = "in";
	string height_special = "totalheight";
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
	if (contains(width_unit, '\\') || contains(height_unit, '\\')) {
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


/// parse an unknown environment
void parse_unknown_environment(Parser & p, string const & name, ostream & os,
			       unsigned flags, bool outer,
			       Context & parent_context)
{
	if (name == "tabbing")
		// We need to remember that we have to handle '\=' specially
		flags |= FLAG_TABBING;

	// We need to translate font changes and paragraphs inside the
	// environment to ERT if we have a non standard font.
	// Otherwise things like
	// \large\begin{foo}\huge bar\end{foo}
	// will not work.
	bool const specialfont =
		(parent_context.font != parent_context.normalfont);
	bool const new_layout_allowed = parent_context.new_layout_allowed;
	if (specialfont)
		parent_context.new_layout_allowed = false;
	handle_ert(os, "\\begin{" + name + "}", parent_context);
	parse_text_snippet(p, os, flags, outer, parent_context);
	handle_ert(os, "\\end{" + name + "}", parent_context);
	if (specialfont)
		parent_context.new_layout_allowed = new_layout_allowed;
}


void parse_environment(Parser & p, ostream & os, bool outer,
		       Context & parent_context)
{
	Layout_ptr newlayout;
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

	else if (name == "tabular" || name == "longtable") {
		eat_whitespace(p, os, parent_context, false);
		parent_context.check_layout(os);
		begin_inset(os, "Tabular ");
		handle_tabular(p, os, name == "longtable", parent_context);
		end_inset(os);
		p.skip_spaces();
	}

	else if (parent_context.textclass.floats().typeExist(unstarred_name)) {
		eat_whitespace(p, os, parent_context, false);
		parent_context.check_layout(os);
		begin_inset(os, "Float " + unstarred_name + "\n");
		if (p.next_token().asInput() == "[") {
			os << "placement " << p.getArg('[', ']') << '\n';
		}
		os << "wide " << convert<string>(is_starred)
		   << "\nsideways false"
		   << "\nstatus open\n\n";
		parse_text_in_inset(p, os, FLAG_END, outer, parent_context);
		end_inset(os);
		// We don't need really a new paragraph, but
		// we must make sure that the next item gets a \begin_layout.
		parent_context.new_paragraph(os);
		p.skip_spaces();
	}

	else if (name == "minipage") {
		eat_whitespace(p, os, parent_context, false);
		parse_box(p, os, FLAG_END, outer, parent_context, false);
		p.skip_spaces();
	}

	else if (name == "comment") {
		eat_whitespace(p, os, parent_context, false);
		parent_context.check_layout(os);
		begin_inset(os, "Note Comment\n");
		os << "status open\n";
		parse_text_in_inset(p, os, FLAG_END, outer, parent_context);
		end_inset(os);
		p.skip_spaces();
	}

	else if (name == "lyxgreyedout") {
		eat_whitespace(p, os, parent_context, false);
		parent_context.check_layout(os);
		begin_inset(os, "Note Greyedout\n");
		os << "status open\n";
		parse_text_in_inset(p, os, FLAG_END, outer, parent_context);
		end_inset(os);
		p.skip_spaces();
	}

	else if (name == "framed") {
		eat_whitespace(p, os, parent_context, false);
		parent_context.check_layout(os);
		begin_inset(os, "Note Framed\n");
		os << "status open\n";
		parse_text_in_inset(p, os, FLAG_END, outer, parent_context);
		end_inset(os);
		p.skip_spaces();
	}

	else if (name == "shaded") {
		eat_whitespace(p, os, parent_context, false);
		parent_context.check_layout(os);
		begin_inset(os, "Note Shaded\n");
		os << "status open\n";
		parse_text_in_inset(p, os, FLAG_END, outer, parent_context);
		end_inset(os);
		p.skip_spaces();
	}

	else if (!parent_context.new_layout_allowed)
		parse_unknown_environment(p, name, os, FLAG_END, outer,
					  parent_context);

	// Alignment settings and leading
	else if (name == "center" || name == "flushleft" || name == "flushright" ||
		 name == "centering" || name == "raggedright" || name == "raggedleft" ||
		 name == "singlespace" || name == "onehalfspace" ||
		 name == "doublespace" || name == "spacing") {
		eat_whitespace(p, os, parent_context, false);
		// We must begin a new paragraph if not already done
		if (! parent_context.atParagraphStart()) {
			parent_context.check_end_layout(os);
			parent_context.new_paragraph(os);
		}
		if (name == "flushleft" || name == "raggedright")
			parent_context.add_extra_stuff("\\align left\n");
		else if (name == "flushright" || name == "raggedleft")
			parent_context.add_extra_stuff("\\align right\n");
		else if (name == "center")
			parent_context.add_extra_stuff("\\align center\n");
		else if (name == "singlespace")
			parent_context.add_extra_stuff("\\paragraph_spacing single\n");
		else if (name == "onehalfspace")
			parent_context.add_extra_stuff("\\paragraph_spacing onehalf\n");
		else if (name == "doublespace")
			parent_context.add_extra_stuff("\\paragraph_spacing double\n");
		else if (name == "spacing")
			parent_context.add_extra_stuff("\\paragraph_spacing other " + p.verbatim_item() + "\n");
		parse_text(p, os, FLAG_END, outer, parent_context);
		// Just in case the environment is empty ..
		parent_context.extra_stuff.erase();
		// We must begin a new paragraph to reset the alignment
		parent_context.new_paragraph(os);
		p.skip_spaces();
	}

	// The single '=' is meant here.
	else if ((newlayout = findLayout(parent_context.textclass, name)).get() &&
		  newlayout->isEnvironment()) {
		eat_whitespace(p, os, parent_context, false);
		Context context(true, parent_context.textclass, newlayout,
				parent_context.layout, parent_context.font);
		if (parent_context.deeper_paragraph) {
			// We are beginning a nested environment after a
			// deeper paragraph inside the outer list environment.
			// Therefore we don't need to output a "begin deeper".
			context.need_end_deeper = true;
		}
		parent_context.check_end_layout(os);
		switch (context.layout->latextype) {
		case  LATEX_LIST_ENVIRONMENT:
			context.add_par_extra_stuff("\\labelwidthstring "
						    + p.verbatim_item() + '\n');
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
		if (parent_context.deeper_paragraph) {
			// We must suppress the "end deeper" because we
			// suppressed the "begin deeper" above.
			context.need_end_deeper = false;
		}
		context.check_end_deeper(os);
		parent_context.new_paragraph(os);
		p.skip_spaces();
	}

	else if (name == "appendix") {
		// This is no good latex style, but it works and is used in some documents...
		eat_whitespace(p, os, parent_context, false);
		parent_context.check_end_layout(os);
		Context context(true, parent_context.textclass, parent_context.layout,
				parent_context.layout, parent_context.font);
		context.check_layout(os);
		os << "\\start_of_appendix\n";
		parse_text(p, os, FLAG_END, outer, context);
		context.check_end_layout(os);
		p.skip_spaces();
	}

	else if (known_environments.find(name) != known_environments.end()) {
		vector<ArgumentType> arguments = known_environments[name];
		// The last "argument" denotes wether we may translate the
		// environment contents to LyX
		// The default required if no argument is given makes us
		// compatible with the reLyXre environment.
		ArgumentType contents = arguments.empty() ?
			required :
			arguments.back();
		if (!arguments.empty())
			arguments.pop_back();
		// See comment in parse_unknown_environment()
		bool const specialfont =
			(parent_context.font != parent_context.normalfont);
		bool const new_layout_allowed =
			parent_context.new_layout_allowed;
		if (specialfont)
			parent_context.new_layout_allowed = false;
		parse_arguments("\\begin{" + name + "}", arguments, p, os,
				outer, parent_context);
		if (contents == verbatim)
			handle_ert(os, p.verbatimEnvironment(name),
				   parent_context);
		else
			parse_text_snippet(p, os, FLAG_END, outer,
					   parent_context);
		handle_ert(os, "\\end{" + name + "}", parent_context);
		if (specialfont)
			parent_context.new_layout_allowed = new_layout_allowed;
	}

	else
		parse_unknown_environment(p, name, os, FLAG_END, outer,
					  parent_context);

	active_environments.pop_back();
}


/// parses a comment and outputs it to \p os.
void parse_comment(Parser & p, ostream & os, Token const & t, Context & context)
{
	BOOST_ASSERT(t.cat() == catComment);
	if (!t.cs().empty()) {
		context.check_layout(os);
		handle_comment(os, '%' + t.cs(), context);
		if (p.next_token().cat() == catNewline) {
			// A newline after a comment line starts a new
			// paragraph
			if (context.new_layout_allowed) {
				if(!context.atParagraphStart())
					// Only start a new paragraph if not already
					// done (we might get called recursively)
					context.new_paragraph(os);
			} else
				handle_ert(os, "\n", context);
			eat_whitespace(p, os, context, true);
		}
	} else {
		// "%\n" combination
		p.skip_spaces();
	}
}


/*!
 * Reads spaces and comments until the first non-space, non-comment token.
 * New paragraphs (double newlines or \\par) are handled like simple spaces
 * if \p eatParagraph is true.
 * Spaces are skipped, but comments are written to \p os.
 */
void eat_whitespace(Parser & p, ostream & os, Context & context,
		    bool eatParagraph)
{
	while (p.good()) {
		Token const & t = p.get_token();
		if (t.cat() == catComment)
			parse_comment(p, os, t, context);
		else if ((! eatParagraph && p.isParagraph()) ||
			 (t.cat() != catSpace && t.cat() != catNewline)) {
			p.putback();
			return;
		}
	}
}


/*!
 * Set a font attribute, parse text and reset the font attribute.
 * \param attribute Attribute name (e.g. \\family, \\shape etc.)
 * \param currentvalue Current value of the attribute. Is set to the new
 * value during parsing.
 * \param newvalue New value of the attribute
 */
void parse_text_attributes(Parser & p, ostream & os, unsigned flags, bool outer,
			   Context & context, string const & attribute,
			   string & currentvalue, string const & newvalue)
{
	context.check_layout(os);
	string const oldvalue = currentvalue;
	currentvalue = newvalue;
	os << '\n' << attribute << ' ' << newvalue << "\n";
	parse_text_snippet(p, os, flags, outer, context);
	context.check_layout(os);
	os << '\n' << attribute << ' ' << oldvalue << "\n";
	currentvalue = oldvalue;
}


/// get the arguments of a natbib or jurabib citation command
std::pair<string, string> getCiteArguments(Parser & p, bool natbibOrder)
{
	// We need to distinguish "" and "[]", so we can't use p.getOpt().

	// text before the citation
	string before;
	// text after the citation
	string after = p.getFullOpt();

	if (!after.empty()) {
		before = p.getFullOpt();
		if (natbibOrder && !before.empty())
			std::swap(before, after);
	}
	return std::make_pair(before, after);
}


/// Convert filenames with TeX macros and/or quotes to something LyX can understand
string const normalize_filename(string const & name)
{
	Parser p(trim(name, "\""));
	ostringstream os;
	while (p.good()) {
		Token const & t = p.get_token();
		if (t.cat() != catEscape)
			os << t.asInput();
		else if (t.cs() == "lyxdot") {
			// This is used by LyX for simple dots in relative
			// names
			os << '.';
			p.skip_spaces();
		} else if (t.cs() == "space") {
			os << ' ';
			p.skip_spaces();
		} else
			os << t.asInput();
	}
	return os.str();
}


/// Convert \p name from TeX convention (relative to master file) to LyX
/// convention (relative to .lyx file) if it is relative
void fix_relative_filename(string & name)
{
	if (lyx::support::absolutePath(name))
		return;
	// FIXME UNICODE encoding of name may be wrong (makeAbsPath expects
	// utf8)
	name = to_utf8(makeRelPath(from_utf8(makeAbsPath(name, getMasterFilePath()).absFilename()),
				   from_utf8(getParentFilePath())));
}


/// Parse a NoWeb Scrap section. The initial "<<" is already parsed.
void parse_noweb(Parser & p, ostream & os, Context & context)
{
	// assemble the rest of the keyword
	string name("<<");
	bool scrap = false;
	while (p.good()) {
		Token const & t = p.get_token();
		if (t.asInput() == ">" && p.next_token().asInput() == ">") {
			name += ">>";
			p.get_token();
			scrap = (p.good() && p.next_token().asInput() == "=");
			if (scrap)
				name += p.get_token().asInput();
			break;
		}
		name += t.asInput();
	}

	if (!scrap || !context.new_layout_allowed ||
	    !context.textclass.hasLayout(from_ascii("Scrap"))) {
		cerr << "Warning: Could not interpret '" << name
		     << "'. Ignoring it." << endl;
		return;
	}

	// We use new_paragraph instead of check_end_layout because the stuff
	// following the noweb chunk needs to start with a \begin_layout.
	// This may create a new paragraph even if there was none in the
	// noweb file, but the alternative is an invalid LyX file. Since
	// noweb code chunks are implemented with a layout style in LyX they
	// always must be in an own paragraph.
	context.new_paragraph(os);
	Context newcontext(true, context.textclass,
		context.textclass[from_ascii("Scrap")]);
	newcontext.check_layout(os);
	os << name;
	while (p.good()) {
		Token const & t = p.get_token();
		// We abuse the parser a bit, because this is no TeX syntax
		// at all.
		if (t.cat() == catEscape)
			os << subst(t.asInput(), "\\", "\n\\backslash\n");
		else
			os << subst(t.asInput(), "\n", "\n\\newline\n");
		// The scrap chunk is ended by an @ at the beginning of a line.
		// After the @ the line may contain a comment and/or
		// whitespace, but nothing else.
		if (t.asInput() == "@" && p.prev_token().cat() == catNewline &&
		    (p.next_token().cat() == catSpace ||
		     p.next_token().cat() == catNewline ||
		     p.next_token().cat() == catComment)) {
			while (p.good() && p.next_token().cat() == catSpace)
				os << p.get_token().asInput();
			if (p.next_token().cat() == catComment)
				// The comment includes a final '\n'
				os << p.get_token().asInput();
			else {
				if (p.next_token().cat() == catNewline)
					p.get_token();
				os << '\n';
			}
			break;
		}
	}
	newcontext.check_end_layout(os);
}

} // anonymous namespace


void parse_text(Parser & p, ostream & os, unsigned flags, bool outer,
		Context & context)
{
	Layout_ptr newlayout;
	// store the current selectlanguage to be used after \foreignlanguage
	string selectlang;
	// Store the latest bibliographystyle (needed for bibtex inset)
	string bibliographystyle;
	bool const use_natbib = used_packages.find("natbib") != used_packages.end();
	bool const use_jurabib = used_packages.find("jurabib") != used_packages.end();
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
		else if (t.asInput() == "`" && p.next_token().asInput() == "`") {
			context.check_layout(os);
			begin_inset(os, "Quotes ");
			os << "eld";
			end_inset(os);
			p.get_token();
			skip_braces(p);
		}
		else if (t.asInput() == "'" && p.next_token().asInput() == "'") {
			context.check_layout(os);
			begin_inset(os, "Quotes ");
			os << "erd";
			end_inset(os);
			p.get_token();
			skip_braces(p);
		}

		else if (t.asInput() == ">" && p.next_token().asInput() == ">") {
			context.check_layout(os);
			begin_inset(os, "Quotes ");
			os << "ald";
			end_inset(os);
			p.get_token();
			skip_braces(p);
		}

		else if (t.asInput() == "<" && p.next_token().asInput() == "<") {
			context.check_layout(os);
			begin_inset(os, "Quotes ");
			os << "ard";
			end_inset(os);
			p.get_token();
			skip_braces(p);
		}

		else if (t.asInput() == "<"
			 && p.next_token().asInput() == "<" && noweb_mode) {
			p.get_token();
			parse_noweb(p, os, context);
		}

		else if (t.cat() == catSpace || (t.cat() == catNewline && ! p.isParagraph()))
			check_space(p, os, context);

		else if (t.character() == '[' && noweb_mode &&
			 p.next_token().character() == '[') {
			// These can contain underscores
			p.putback();
			string const s = p.getFullOpt() + ']';
			if (p.next_token().character() == ']')
				p.get_token();
			else
				cerr << "Warning: Inserting missing ']' in '"
				     << s << "'." << endl;
			handle_ert(os, s, context);
		}

		else if (t.cat() == catLetter ||
			       t.cat() == catOther ||
			       t.cat() == catAlign ||
			       t.cat() == catParameter) {
			// This translates "&" to "\\&" which may be wrong...
			context.check_layout(os);
			os << t.character();
		}

		else if (p.isParagraph()) {
			if (context.new_layout_allowed)
				context.new_paragraph(os);
			else
				handle_ert(os, "\\par ", context);
			eat_whitespace(p, os, context, true);
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

		else if (t.cat() == catBegin &&
			 p.next_token().cat() == catEnd) {
			// {}
			Token const prev = p.prev_token();
			p.get_token();
			if (p.next_token().character() == '`' ||
			    (prev.character() == '-' &&
			     p.next_token().character() == '-'))
				; // ignore it in {}`` or -{}-
			else
				handle_ert(os, "{}", context);

		}

		else if (t.cat() == catBegin) {
			context.check_layout(os);
			// special handling of font attribute changes
			Token const prev = p.prev_token();
			Token const next = p.next_token();
			TeXFont const oldFont = context.font;
			if (next.character() == '[' ||
			    next.character() == ']' ||
			    next.character() == '*') {
				p.get_token();
				if (p.next_token().cat() == catEnd) {
					os << next.character();
					p.get_token();
				} else {
					p.putback();
					handle_ert(os, "{", context);
					parse_text_snippet(p, os,
							FLAG_BRACE_LAST,
							outer, context);
					handle_ert(os, "}", context);
				}
			} else if (! context.new_layout_allowed) {
				handle_ert(os, "{", context);
				parse_text_snippet(p, os, FLAG_BRACE_LAST,
						   outer, context);
				handle_ert(os, "}", context);
			} else if (is_known(next.cs(), known_sizes)) {
				// next will change the size, so we must
				// reset it here
				parse_text_snippet(p, os, FLAG_BRACE_LAST,
						   outer, context);
				if (!context.atParagraphStart())
					os << "\n\\size "
					   << context.font.size << "\n";
			} else if (is_known(next.cs(), known_font_families)) {
				// next will change the font family, so we
				// must reset it here
				parse_text_snippet(p, os, FLAG_BRACE_LAST,
						   outer, context);
				if (!context.atParagraphStart())
					os << "\n\\family "
					   << context.font.family << "\n";
			} else if (is_known(next.cs(), known_font_series)) {
				// next will change the font series, so we
				// must reset it here
				parse_text_snippet(p, os, FLAG_BRACE_LAST,
						   outer, context);
				if (!context.atParagraphStart())
					os << "\n\\series "
					   << context.font.series << "\n";
			} else if (is_known(next.cs(), known_font_shapes)) {
				// next will change the font shape, so we
				// must reset it here
				parse_text_snippet(p, os, FLAG_BRACE_LAST,
						   outer, context);
				if (!context.atParagraphStart())
					os << "\n\\shape "
					   << context.font.shape << "\n";
			} else if (is_known(next.cs(), known_old_font_families) ||
				   is_known(next.cs(), known_old_font_series) ||
				   is_known(next.cs(), known_old_font_shapes)) {
				// next will change the font family, series
				// and shape, so we must reset it here
				parse_text_snippet(p, os, FLAG_BRACE_LAST,
						   outer, context);
				if (!context.atParagraphStart())
					os <<  "\n\\family "
					   << context.font.family
					   << "\n\\series "
					   << context.font.series
					   << "\n\\shape "
					   << context.font.shape << "\n";
			} else {
				handle_ert(os, "{", context);
				parse_text_snippet(p, os, FLAG_BRACE_LAST,
						   outer, context);
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

		else if (t.cat() == catComment)
			parse_comment(p, os, t, context);

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
				s = parse_text_snippet(p, FLAG_BRACK_LAST,
						       outer, context);
				optarg = true;
			}
			context.set_item();
			context.check_layout(os);
			if (context.has_item) {
				// An item in an unknown list-like environment
				// FIXME: Do this in check_layout()!
				context.has_item = false;
				if (optarg)
					handle_ert(os, "\\item", context);
				else
					handle_ert(os, "\\item ", context);
			}
			if (optarg) {
				if (context.layout->labeltype != LABEL_MANUAL) {
					// lyx does not support \item[\mybullet]
					// in itemize environments
					handle_ert(os, "[", context);
					os << s;
					handle_ert(os, "]", context);
				} else if (!s.empty()) {
					// The space is needed to separate the
					// item from the rest of the sentence.
					os << s << ' ';
					eat_whitespace(p, os, context, false);
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
			context.check_layout(os);
			eat_whitespace(p, os, context, false);
			string name = p.get_token().cs();
			eat_whitespace(p, os, context, false);

			// parameter text
			bool simple = true;
			string paramtext;
			int arity = 0;
			while (p.next_token().cat() != catBegin) {
				if (p.next_token().cat() == catParameter) {
					// # found
					p.get_token();
					paramtext += "#";

					// followed by number?
					if (p.next_token().cat() == catOther) {
						char c = p.getChar();
						paramtext += c;
						// number = current arity + 1?
						if (c == arity + '0' + 1)
							++arity;
						else
							simple = false;
					} else
						paramtext += p.get_token().asString();
				} else {
					paramtext += p.get_token().asString();
					simple = false;
				}
			}

			// only output simple (i.e. compatible) macro as FormulaMacros
			string ert = "\\def\\" + name + ' ' + paramtext + '{' + p.verbatim_item() + '}';
			if (simple) {
				context.check_layout(os);
				begin_inset(os, "FormulaMacro");
				os << "\n" << ert;
				end_inset(os);
			} else
				handle_ert(os, ert, context);
		}

		else if (t.cs() == "noindent") {
			p.skip_spaces();
			context.add_par_extra_stuff("\\noindent\n");
		}

		else if (t.cs() == "appendix") {
			context.add_par_extra_stuff("\\start_of_appendix\n");
			// We need to start a new paragraph. Otherwise the
			// appendix in 'bla\appendix\chapter{' would start
			// too late.
			context.new_paragraph(os);
			// We need to make sure that the paragraph is
			// generated even if it is empty. Otherwise the
			// appendix in '\par\appendix\par\chapter{' would
			// start too late.
			context.check_layout(os);
			// FIXME: This is a hack to prevent paragraph
			// deletion if it is empty. Handle this better!
			handle_comment(os,
				"%dummy comment inserted by tex2lyx to "
				"ensure that this paragraph is not empty",
				context);
			// Both measures above may generate an additional
			// empty paragraph, but that does not hurt, because
			// whitespace does not matter here.
			eat_whitespace(p, os, context, true);
		}

		// Starred section headings
		// Must attempt to parse "Section*" before "Section".
		else if ((p.next_token().asInput() == "*") &&
			 context.new_layout_allowed &&
			 (newlayout = findLayout(context.textclass,
						 t.cs() + '*')).get() &&
			 newlayout->isCommand()) {
			TeXFont const oldFont = context.font;
			// save the current font size
			string const size = oldFont.size;
			// reset the font size to default, because the font size switches don't
			// affect section headings and the like
			context.font.size = known_coded_sizes[0];
			output_font_change(os, oldFont, context.font);
			// write the layout
			p.get_token();
			output_command_layout(os, p, outer, context, newlayout);
			// set the font size to the original value
			context.font.size = size;
			output_font_change(os, oldFont, context.font);
			p.skip_spaces();
		}

		// Section headings and the like
		else if (context.new_layout_allowed &&
			 (newlayout = findLayout(context.textclass, t.cs())).get() &&
			 newlayout->isCommand()) {
			TeXFont const oldFont = context.font;
			// save the current font size
			string const size = oldFont.size;
			// reset the font size to default, because the font size switches don't
			// affect section headings and the like
			context.font.size = known_coded_sizes[0];
			output_font_change(os, oldFont, context.font);
			// write the layout
			output_command_layout(os, p, outer, context, newlayout);
			// set the font size to the original value
			context.font.size = size;
			output_font_change(os, oldFont, context.font);
			p.skip_spaces();
		}

		// Special handling for \caption
		// FIXME: remove this when InsetCaption is supported.
		else if (context.new_layout_allowed &&
			 t.cs() == captionlayout->latexname()) {
			output_command_layout(os, p, outer, context, 
					      captionlayout);
			p.skip_spaces();
		}

		else if (t.cs() == "includegraphics") {
			bool const clip = p.next_token().asInput() == "*";
			if (clip)
				p.get_token();
			map<string, string> opts = split_map(p.getArg('[', ']'));
			if (clip)
				opts["clip"] = string();
			string name = normalize_filename(p.verbatim_item());

			string const path = getMasterFilePath();
			// We want to preserve relative / absolute filenames,
			// therefore path is only used for testing
			// FIXME UNICODE encoding of name and path may be
			// wrong (makeAbsPath expects utf8)
			if (!doesFileExist(makeAbsPath(name, path))) {
				// The file extension is probably missing.
				// Now try to find it out.
				string const dvips_name =
					find_file(name, path,
						  known_dvips_graphics_formats);
				string const pdftex_name =
					find_file(name, path,
						  known_pdftex_graphics_formats);
				if (!dvips_name.empty()) {
					if (!pdftex_name.empty()) {
						cerr << "This file contains the "
							"latex snippet\n"
							"\"\\includegraphics{"
						     << name << "}\".\n"
							"However, files\n\""
						     << dvips_name << "\" and\n\""
						     << pdftex_name << "\"\n"
							"both exist, so I had to make a "
							"choice and took the first one.\n"
							"Please move the unwanted one "
							"someplace else and try again\n"
							"if my choice was wrong."
						     << endl;
					}
					name = dvips_name;
				} else if (!pdftex_name.empty())
					name = pdftex_name;
			}

			// FIXME UNICODE encoding of name and path may be
			// wrong (makeAbsPath expects utf8)
			if (doesFileExist(makeAbsPath(name, path)))
				fix_relative_filename(name);
			else
				cerr << "Warning: Could not find graphics file '"
				     << name << "'." << endl;

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
				   << opts["bbllx"] << " " << opts["bblly"] << " "
				   << opts["bburx"] << " " << opts["bbury"] << '\n';
			else if (numberOfbbOptions > 0)
				cerr << "Warning: Ignoring incomplete includegraphics boundingbox arguments.\n";
			numberOfbbOptions = 0;
			if (opts.find("natwidth") != opts.end())
				numberOfbbOptions++;
			if (opts.find("natheight") != opts.end())
				numberOfbbOptions++;
			if (numberOfbbOptions == 2)
				os << "\tBoundingBox 0bp 0bp "
				   << opts["natwidth"] << " " << opts["natheight"] << '\n';
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
			// Check whether some option was given twice.
			end_inset(os);
		}

		else if (t.cs() == "footnote" ||
			 (t.cs() == "thanks" && context.layout->intitle)) {
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
			string const s = p.verbatim_item();
			if (s == "\xb1" || s == "\xb3" || s == "\xb2" || s == "\xb5")
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
			// FIXME: Somehow prevent title layouts if
			// "maketitle" was not found
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

		else if (t.cs() == "textrm")
			parse_text_attributes(p, os, FLAG_ITEM, outer,
					      context, "\\family",
					      context.font.family, "roman");

		else if (t.cs() == "textsf")
			parse_text_attributes(p, os, FLAG_ITEM, outer,
					      context, "\\family",
					      context.font.family, "sans");

		else if (t.cs() == "texttt")
			parse_text_attributes(p, os, FLAG_ITEM, outer,
					      context, "\\family",
					      context.font.family, "typewriter");

		else if (t.cs() == "textmd")
			parse_text_attributes(p, os, FLAG_ITEM, outer,
					      context, "\\series",
					      context.font.series, "medium");

		else if (t.cs() == "textbf")
			parse_text_attributes(p, os, FLAG_ITEM, outer,
					      context, "\\series",
					      context.font.series, "bold");

		else if (t.cs() == "textup")
			parse_text_attributes(p, os, FLAG_ITEM, outer,
					      context, "\\shape",
					      context.font.shape, "up");

		else if (t.cs() == "textit")
			parse_text_attributes(p, os, FLAG_ITEM, outer,
					      context, "\\shape",
					      context.font.shape, "italic");

		else if (t.cs() == "textsl")
			parse_text_attributes(p, os, FLAG_ITEM, outer,
					      context, "\\shape",
					      context.font.shape, "slanted");

		else if (t.cs() == "textsc")
			parse_text_attributes(p, os, FLAG_ITEM, outer,
					      context, "\\shape",
					      context.font.shape, "smallcaps");

		else if (t.cs() == "textnormal" || t.cs() == "normalfont") {
			context.check_layout(os);
			TeXFont oldFont = context.font;
			context.font.init();
			context.font.size = oldFont.size;
			os << "\n\\family " << context.font.family << "\n";
			os << "\n\\series " << context.font.series << "\n";
			os << "\n\\shape " << context.font.shape << "\n";
			if (t.cs() == "textnormal") {
				parse_text_snippet(p, os, FLAG_ITEM, outer, context);
				output_font_change(os, context.font, oldFont);
				context.font = oldFont;
			} else
				eat_whitespace(p, os, context, false);
		}

		else if (t.cs() == "textcolor") {
			// scheme is \textcolor{color name}{text}
			string const color = p.verbatim_item();
			// we only support the predefined colors of the color package
			if (color == "black" || color == "blue" || color == "cyan"
				|| color == "green" || color == "magenta" || color == "red"
				|| color == "white" || color == "yellow") {
					context.check_layout(os);
					os << "\n\\color " << color << "\n";
					parse_text_snippet(p, os, FLAG_ITEM, outer, context);
					context.check_layout(os);
					os << "\n\\color inherit\n";
			} else
				// for custom defined colors
				handle_ert(os, t.asInput() + "{" + color + "}", context);
		}

		else if (t.cs() == "underbar") {
			// Do NOT handle \underline.
			// \underbar cuts through y, g, q, p etc.,
			// \underline does not.
			context.check_layout(os);
			os << "\n\\bar under\n";
			parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			context.check_layout(os);
			os << "\n\\bar default\n";
		}

		else if (t.cs() == "emph" || t.cs() == "noun") {
			context.check_layout(os);
			os << "\n\\" << t.cs() << " on\n";
			parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			context.check_layout(os);
			os << "\n\\" << t.cs() << " default\n";
		}

		else if (t.cs() == "lyxline") {
			context.check_layout(os);
			os << "\\lyxline";
		}

		else if (use_natbib &&
			 is_known(t.cs(), known_natbib_commands) &&
			 ((t.cs() != "citefullauthor" &&
			   t.cs() != "citeyear" &&
			   t.cs() != "citeyearpar") ||
			  p.next_token().asInput() != "*")) {
			context.check_layout(os);
			// tex                       lyx
			// \citet[before][after]{a}  \citet[after][before]{a}
			// \citet[before][]{a}       \citet[][before]{a}
			// \citet[after]{a}          \citet[after]{a}
			// \citet{a}                 \citet{a}
			string command = '\\' + t.cs();
			if (p.next_token().asInput() == "*") {
				command += '*';
				p.get_token();
			}
			if (command == "\\citefullauthor")
				// alternative name for "\\citeauthor*"
				command = "\\citeauthor*";

			// text before the citation
			string before;
			// text after the citation
			string after;

			boost::tie(before, after) = getCiteArguments(p, true);
			if (command == "\\cite") {
				// \cite without optional argument means
				// \citet, \cite with at least one optional
				// argument means \citep.
				if (before.empty() && after.empty())
					command = "\\citet";
				else
					command = "\\citep";
			}
			if (before.empty() && after == "[]")
				// avoid \citet[]{a}
				after.erase();
			else if (before == "[]" && after == "[]") {
				// avoid \citet[][]{a}
				before.erase();
				after.erase();
			}
			begin_inset(os, "LatexCommand ");
			os << command << after << before
			   << '{' << p.verbatim_item() << "}\n";
			end_inset(os);
		}

		else if (use_jurabib &&
			 is_known(t.cs(), known_jurabib_commands)) {
			context.check_layout(os);
			string const command = '\\' + t.cs();
			char argumentOrder = '\0';
			vector<string> const & options = used_packages["jurabib"];
			if (std::find(options.begin(), options.end(),
				      "natbiborder") != options.end())
				argumentOrder = 'n';
			else if (std::find(options.begin(), options.end(),
					   "jurabiborder") != options.end())
				argumentOrder = 'j';

			// text before the citation
			string before;
			// text after the citation
			string after;

			boost::tie(before, after) =
				getCiteArguments(p, argumentOrder != 'j');
			string const citation = p.verbatim_item();
			if (!before.empty() && argumentOrder == '\0') {
				cerr << "Warning: Assuming argument order "
					"of jurabib version 0.6 for\n'"
				     << command << before << after << '{'
				     << citation << "}'.\n"
					"Add 'jurabiborder' to the jurabib "
					"package options if you used an\n"
					"earlier jurabib version." << endl;
			}
			begin_inset(os, "LatexCommand ");
			os << command << after << before
			   << '{' << citation << "}\n";
			end_inset(os);
		}

		else if (is_known(t.cs(), known_latex_commands)) {
			// This needs to be after the check for natbib and
			// jurabib commands, because "cite" has different
			// arguments with natbib and jurabib.
			context.check_layout(os);
			begin_inset(os, "LatexCommand ");
			os << '\\' << t.cs();
			// lyx cannot handle newlines in a latex command
			// FIXME: Move the substitution into parser::getOpt()?
			os << subst(p.getOpt(), "\n", " ");
			os << subst(p.getOpt(), "\n", " ");
			os << '{' << subst(p.verbatim_item(), "\n", " ") << "}\n";
			end_inset(os);
		}

		else if (is_known(t.cs(), known_quotes)) {
			char const * const * where = is_known(t.cs(), known_quotes);
			context.check_layout(os);
			begin_inset(os, "Quotes ");
			os << known_coded_quotes[where - known_quotes];
			end_inset(os);
			// LyX adds {} after the quote, so we have to eat
			// spaces here if there are any before a possible
			// {} pair.
			eat_whitespace(p, os, context, false);
			skip_braces(p);
		}

		else if (is_known(t.cs(), known_sizes) &&
			 context.new_layout_allowed) {
			char const * const * where = is_known(t.cs(), known_sizes);
			context.check_layout(os);
			TeXFont const oldFont = context.font;
			// the font size index differs by 1, because the known_coded_sizes
			// has additionally a "default" entry
			context.font.size = known_coded_sizes[where - known_sizes + 1];
			output_font_change(os, oldFont, context.font);
			eat_whitespace(p, os, context, false);
		}

		else if (is_known(t.cs(), known_font_families) &&
			 context.new_layout_allowed) {
			char const * const * where =
				is_known(t.cs(), known_font_families);
			context.check_layout(os);
			TeXFont const oldFont = context.font;
			context.font.family =
				known_coded_font_families[where - known_font_families];
			output_font_change(os, oldFont, context.font);
			eat_whitespace(p, os, context, false);
		}

		else if (is_known(t.cs(), known_font_series) &&
			 context.new_layout_allowed) {
			char const * const * where =
				is_known(t.cs(), known_font_series);
			context.check_layout(os);
			TeXFont const oldFont = context.font;
			context.font.series =
				known_coded_font_series[where - known_font_series];
			output_font_change(os, oldFont, context.font);
			eat_whitespace(p, os, context, false);
		}

		else if (is_known(t.cs(), known_font_shapes) &&
			 context.new_layout_allowed) {
			char const * const * where =
				is_known(t.cs(), known_font_shapes);
			context.check_layout(os);
			TeXFont const oldFont = context.font;
			context.font.shape =
				known_coded_font_shapes[where - known_font_shapes];
			output_font_change(os, oldFont, context.font);
			eat_whitespace(p, os, context, false);
		}
		else if (is_known(t.cs(), known_old_font_families) &&
			 context.new_layout_allowed) {
			char const * const * where =
				is_known(t.cs(), known_old_font_families);
			context.check_layout(os);
			TeXFont const oldFont = context.font;
			context.font.init();
			context.font.size = oldFont.size;
			context.font.family =
				known_coded_font_families[where - known_old_font_families];
			output_font_change(os, oldFont, context.font);
			eat_whitespace(p, os, context, false);
		}

		else if (is_known(t.cs(), known_old_font_series) &&
			 context.new_layout_allowed) {
			char const * const * where =
				is_known(t.cs(), known_old_font_series);
			context.check_layout(os);
			TeXFont const oldFont = context.font;
			context.font.init();
			context.font.size = oldFont.size;
			context.font.series =
				known_coded_font_series[where - known_old_font_series];
			output_font_change(os, oldFont, context.font);
			eat_whitespace(p, os, context, false);
		}

		else if (is_known(t.cs(), known_old_font_shapes) &&
			 context.new_layout_allowed) {
			char const * const * where =
				is_known(t.cs(), known_old_font_shapes);
			context.check_layout(os);
			TeXFont const oldFont = context.font;
			context.font.init();
			context.font.size = oldFont.size;
			context.font.shape =
				known_coded_font_shapes[where - known_old_font_shapes];
			output_font_change(os, oldFont, context.font);
			eat_whitespace(p, os, context, false);
		}

		else if (t.cs() == "selectlanguage") {
			context.check_layout(os);
			// save the language for the case that a \foreignlanguage is used 
			selectlang = subst(p.verbatim_item(), "\n", " ");
			os << "\\lang " << selectlang << "\n";
			
		}

		else if (t.cs() == "foreignlanguage") {
			context.check_layout(os);
			os << "\n\\lang " << subst(p.verbatim_item(), "\n", " ") << "\n";
			os << subst(p.verbatim_item(), "\n", " ");
			// set back to last selectlanguage
			os << "\n\\lang " << selectlang << "\n";
		}

		else if (t.cs() == "inputencoding")
			// write nothing because this is done by LyX using the "\lang"
			// information given by selectlanguage and foreignlanguage
			subst(p.verbatim_item(), "\n", " ");
		
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
			os << "\n\\backslash\n";
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

		else if (t.cs() == "verb") {
			context.check_layout(os);
			char const delimiter = p.next_token().character();
			string const arg = p.getArg(delimiter, delimiter);
			ostringstream oss;
			oss << "\\verb" << delimiter << arg << delimiter;
			handle_ert(os, oss.str(), context);
		}

		else if (t.cs() == "\"") {
			context.check_layout(os);
			string const name = p.verbatim_item();
			     if (name == "a") os << '\xe4';
			else if (name == "o") os << '\xf6';
			else if (name == "u") os << '\xfc';
			else if (name == "A") os << '\xc4';
			else if (name == "O") os << '\xd6';
			else if (name == "U") os << '\xdc';
			else handle_ert(os, "\"{" + name + "}", context);
		}

		// Problem: \= creates a tabstop inside the tabbing environment
		// and else an accent. In the latter case we really would want
		// \={o} instead of \= o.
		else if (t.cs() == "=" && (flags & FLAG_TABBING))
			handle_ert(os, t.asInput(), context);

		else if (t.cs() == "H" || t.cs() == "c" || t.cs() == "^"
			 || t.cs() == "'" || t.cs() == "`"
			 || t.cs() == "~" || t.cs() == "." || t.cs() == "=") {
			// we need the trim as the LyX parser chokes on such spaces
			// The argument of InsetLatexAccent is parsed as a
			// subset of LaTeX, so don't parse anything here,
			// but use the raw argument.
			// Otherwise we would convert \~{\i} wrongly.
			// This will of course not translate \~{\ss} to \~{ÃŸ},
			// but that does at least compile and does only look
			// strange on screen.
			context.check_layout(os);
			os << "\\i \\" << t.cs() << "{"
			   << trim(p.verbatim_item(), " ")
			   << "}\n";
		}

		else if (t.cs() == "ss") {
			context.check_layout(os);
			os << "\xdf";
			skip_braces(p); // eat {}
		}

		else if (t.cs() == "i" || t.cs() == "j" || t.cs() == "l" ||
			 t.cs() == "L") {
			context.check_layout(os);
			os << "\\i \\" << t.cs() << "{}\n";
			skip_braces(p); // eat {}
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
				os << "\n\\newline\n";
			}
		}

		else if (t.cs() == "newline") {
			context.check_layout(os);
			os << "\n\\" << t.cs() << "\n";
			skip_braces(p); // eat {}
		}

		else if (t.cs() == "input" || t.cs() == "include"
			 || t.cs() == "verbatiminput") {
			string name = '\\' + t.cs();
			if (t.cs() == "verbatiminput"
			    && p.next_token().asInput() == "*")
				name += p.get_token().asInput();
			context.check_layout(os);
			begin_inset(os, "Include ");
			string filename(normalize_filename(p.getArg('{', '}')));
			string const path = getMasterFilePath();
			// We want to preserve relative / absolute filenames,
			// therefore path is only used for testing
			// FIXME UNICODE encoding of filename and path may be
			// wrong (makeAbsPath expects utf8)
			if ((t.cs() == "include" || t.cs() == "input") &&
			    !doesFileExist(makeAbsPath(filename, path))) {
				// The file extension is probably missing.
				// Now try to find it out.
				string const tex_name =
					find_file(filename, path,
						  known_tex_extensions);
				if (!tex_name.empty())
					filename = tex_name;
			}
			// FIXME UNICODE encoding of filename and path may be
			// wrong (makeAbsPath expects utf8)
			if (doesFileExist(makeAbsPath(filename, path))) {
				string const abstexname =
					makeAbsPath(filename, path).absFilename();
				string const abslyxname =
					changeExtension(abstexname, ".lyx");
				fix_relative_filename(filename);
				string const lyxname =
					changeExtension(filename, ".lyx");
				if (t.cs() != "verbatiminput" &&
				    tex2lyx(abstexname, FileName(abslyxname))) {
					os << name << '{' << lyxname << "}\n";
				} else {
					os << name << '{' << filename << "}\n";
				}
			} else {
				cerr << "Warning: Could not find included file '"
				     << filename << "'." << endl;
				os << name << '{' << filename << "}\n";
			}
			os << "preview false\n";
			end_inset(os);
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

		//\makebox() is part of the picture environment and different from \makebox{}
		//\makebox{} will be parsed by parse_box when bug 2956 is fixed
		else if (t.cs() == "makebox") {
			string arg = "\\makebox";
			if (p.next_token().character() == '(')
				//the syntax is: \makebox(x,y)[position]{content}
				arg += p.getFullParentheseOpt();
			else
				//the syntax is: \makebox[width][position]{content}
				arg += p.getFullOpt();
			handle_ert(os, arg + p.getFullOpt(), context);
		}

		else if (t.cs() == "smallskip" ||
			 t.cs() == "medskip" ||
			 t.cs() == "bigskip" ||
			 t.cs() == "vfill") {
			context.check_layout(os);
			begin_inset(os, "VSpace ");
			os << t.cs();
			end_inset(os);
			skip_braces(p);
		}

		else if (is_known(t.cs(), known_spaces)) {
			char const * const * where = is_known(t.cs(), known_spaces);
			context.check_layout(os);
			begin_inset(os, "InsetSpace ");
			os << '\\' << known_coded_spaces[where - known_spaces]
			   << '\n';
			// LaTeX swallows whitespace after all spaces except
			// "\\,". We have to do that here, too, because LyX
			// adds "{}" which would make the spaces significant.
			if (t.cs() !=  ",")
				eat_whitespace(p, os, context, false);
			// LyX adds "{}" after all spaces except "\\ " and
			// "\\,", so we have to remove "{}".
			// "\\,{}" is equivalent to "\\," in LaTeX, so we
			// remove the braces after "\\,", too.
			if (t.cs() != " ")
				skip_braces(p);
		}

		else if (t.cs() == "newpage" ||
			 t.cs() == "clearpage" ||
			 t.cs() == "cleardoublepage") {
			context.check_layout(os);
			// FIXME: what about \\pagebreak?
			os << "\n\\" << t.cs() << "\n";
			skip_braces(p); // eat {}
		}

		else if (t.cs() == "newcommand" ||
			 t.cs() == "providecommand" ||
			 t.cs() == "renewcommand") {
			// these could be handled by parse_command(), but
			// we need to call add_known_command() here.
			string name = t.asInput();
			if (p.next_token().asInput() == "*") {
				// Starred form. Eat '*'
				p.get_token();
				name += '*';
			}
			string const command = p.verbatim_item();
			string const opt1 = p.getOpt();
			string const opt2 = p.getFullOpt();
			add_known_command(command, opt1, !opt2.empty());
			string const ert = name + '{' + command + '}' +
					   opt1 + opt2 +
					   '{' + p.verbatim_item() + '}';

			if (opt2.empty()) {
				context.check_layout(os);
				begin_inset(os, "FormulaMacro");
				os << "\n" << ert;
				end_inset(os);
			} else
				// we cannot handle optional argument, so only output ERT
				handle_ert(os, ert, context);
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
				}
				if (!known_vspace) {
					switch (unitFromString(unit)) {
					case Length::SP:
					case Length::PT:
					case Length::BP:
					case Length::DD:
					case Length::MM:
					case Length::PC:
					case Length::CC:
					case Length::CM:
					case Length::IN:
					case Length::EX:
					case Length::EM:
					case Length::MU:
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


} // namespace lyx
