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
#include "Encoding.h"
#include "FloatList.h"
#include "LaTeXPackages.h"
#include "Layout.h"
#include "Length.h"
#include "Preamble.h"

#include "insets/ExternalTemplate.h"

#include "support/lassert.h"
#include "support/convert.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxtime.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using namespace std;
using namespace lyx::support;

namespace lyx {


namespace {

void output_arguments(ostream &, Parser &, bool, bool, bool, Context &,
                      Layout::LaTeXArgMap const &);

}


void parse_text_in_inset(Parser & p, ostream & os, unsigned flags, bool outer,
		Context const & context, InsetLayout const * layout)
{
	bool const forcePlainLayout =
		layout ? layout->forcePlainLayout() : false;
	Context newcontext(true, context.textclass);
	if (forcePlainLayout)
		newcontext.layout = &context.textclass.plainLayout();
	else
		newcontext.font = context.font;
	if (layout)
		output_arguments(os, p, outer, false, false, newcontext,
		                 layout->latexargs());
	parse_text(p, os, flags, outer, newcontext);
	if (layout)
		output_arguments(os, p, outer, false, true, newcontext,
		                 layout->postcommandargs());
	newcontext.check_end_layout(os);
}


namespace {

void parse_text_in_inset(Parser & p, ostream & os, unsigned flags, bool outer,
		Context const & context, string const & name)
{
	InsetLayout const * layout = 0;
	DocumentClass::InsetLayouts::const_iterator it =
		context.textclass.insetLayouts().find(from_ascii(name));
	if (it != context.textclass.insetLayouts().end())
		layout = &(it->second);
	parse_text_in_inset(p, os, flags, outer, context, layout);
}

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


char const * const known_ref_commands[] = { "ref", "pageref", "vref",
 "vpageref", "prettyref", "nameref", "eqref", 0 };

char const * const known_coded_ref_commands[] = { "ref", "pageref", "vref",
 "vpageref", "formatted", "nameref", "eqref", 0 };

char const * const known_refstyle_commands[] = { "algref", "chapref", "corref",
 "eqref", "enuref", "figref", "fnref", "lemref", "parref", "partref", "propref",
 "secref", "subref", "tabref", "thmref", 0 };

char const * const known_refstyle_prefixes[] = { "alg", "chap", "cor",
 "eq", "enu", "fig", "fn", "lem", "par", "part", "prop",
 "sec", "sub", "tab", "thm", 0 };


/**
 * supported CJK encodings
 * JIS does not work with LyX's encoding conversion
 */
const char * const supported_CJK_encodings[] = {
"EUC-JP", "KS", "GB", "UTF8",
"Bg5", /*"JIS",*/ "SJIS", 0};

/**
 * the same as supported_CJK_encodings with their corresponding LyX language name
 * FIXME: The mapping "UTF8" => "chinese-traditional" is only correct for files
 *        created by LyX.
 * NOTE: "Bg5", "JIS" and "SJIS" are not supported by LyX, on re-export the
 *       encodings "UTF8", "EUC-JP" and "EUC-JP" will be used.
 * please keep this in sync with supported_CJK_encodings line by line!
 */
const char * const supported_CJK_languages[] = {
"japanese-cjk", "korean", "chinese-simplified", "chinese-traditional",
"chinese-traditional", /*"japanese-cjk",*/ "japanese-cjk", 0};

/*!
 * natbib commands.
 * The starred forms are also known except for "citefullauthor",
 * "citeyear" and "citeyearpar".
 */
char const * const known_natbib_commands[] = { "cite", "citet", "citep",
"citealt", "citealp", "citeauthor", "citeyear", "citeyearpar",
"citefullauthor", "Citet", "Citep", "Citealt", "Citealp", "Citeauthor", 0 };

/*!
 * jurabib commands.
 * No starred form other than "cite*" known.
 */
char const * const known_jurabib_commands[] = { "cite", "citet", "citep",
"citealt", "citealp", "citeauthor", "citeyear", "citeyearpar",
// jurabib commands not (yet) supported by LyX:
// "fullcite",
// "footcite", "footcitet", "footcitep", "footcitealt", "footcitealp",
// "footciteauthor", "footciteyear", "footciteyearpar",
"citefield", "citetitle", 0 };

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
char const * const known_coded_sizes[] = { "tiny", "scriptsize", "footnotesize",
"small", "normal", "large", "larger", "largest", "huge", "giant", 0};

/// LaTeX 2.09 names for font families
char const * const known_old_font_families[] = { "rm", "sf", "tt", 0};

/// LaTeX names for font families
char const * const known_font_families[] = { "rmfamily", "sffamily",
"ttfamily", 0};

/// LaTeX names for font family changing commands
char const * const known_text_font_families[] = { "textrm", "textsf",
"texttt", 0};

/// The same as known_old_font_families, known_font_families and
/// known_text_font_families with .lyx names
char const * const known_coded_font_families[] = { "roman", "sans",
"typewriter", 0};

/// LaTeX 2.09 names for font series
char const * const known_old_font_series[] = { "bf", 0};

/// LaTeX names for font series
char const * const known_font_series[] = { "bfseries", "mdseries", 0};

/// LaTeX names for font series changing commands
char const * const known_text_font_series[] = { "textbf", "textmd", 0};

/// The same as known_old_font_series, known_font_series and
/// known_text_font_series with .lyx names
char const * const known_coded_font_series[] = { "bold", "medium", 0};

/// LaTeX 2.09 names for font shapes
char const * const known_old_font_shapes[] = { "it", "sl", "sc", 0};

/// LaTeX names for font shapes
char const * const known_font_shapes[] = { "itshape", "slshape", "scshape",
"upshape", 0};

/// LaTeX names for font shape changing commands
char const * const known_text_font_shapes[] = { "textit", "textsl", "textsc",
"textup", 0};

/// The same as known_old_font_shapes, known_font_shapes and
/// known_text_font_shapes with .lyx names
char const * const known_coded_font_shapes[] = { "italic", "slanted",
"smallcaps", "up", 0};

/// Known special characters which need skip_spaces_braces() afterwards
char const * const known_special_chars[] = {"ldots",
"lyxarrow", "textcompwordmark",
"slash", "textasciitilde", "textasciicircum", "textbackslash", 0};

/// the same as known_special_chars with .lyx names
char const * const known_coded_special_chars[] = {"\\SpecialChar \\ldots{}\n",
"\\SpecialChar \\menuseparator\n", "\\SpecialChar \\textcompwordmark{}\n",
"\\SpecialChar \\slash{}\n", "~", "^", "\n\\backslash\n", 0};

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
char const * const known_spaces[] = { " ", "space", ",",
"thinspace", "quad", "qquad", "enspace", "enskip",
"negthinspace", "negmedspace", "negthickspace", "textvisiblespace",
"hfill", "dotfill", "hrulefill", "leftarrowfill", "rightarrowfill",
"upbracefill", "downbracefill", 0};

/// the same as known_spaces with .lyx names
char const * const known_coded_spaces[] = { "space{}", "space{}",
"thinspace{}", "thinspace{}", "quad{}", "qquad{}", "enspace{}", "enskip{}",
"negthinspace{}", "negmedspace{}", "negthickspace{}", "textvisiblespace{}",
"hfill{}", "dotfill{}", "hrulefill{}", "leftarrowfill{}", "rightarrowfill{}",
"upbracefill{}", "downbracefill{}", 0};

/// These are translated by LyX to commands like "\\LyX{}", so we have to put
/// them in ERT. "LaTeXe" must come before "LaTeX"!
char const * const known_phrases[] = {"LyX", "TeX", "LaTeXe", "LaTeX", 0};
char const * const known_coded_phrases[] = {"LyX", "TeX", "LaTeX2e", "LaTeX", 0};
int const known_phrase_lengths[] = {3, 5, 7, 0};

/// known TIPA combining diacritical marks
char const * const known_tipa_marks[] = {"textsubwedge", "textsubumlaut",
"textsubtilde", "textseagull", "textsubbridge", "textinvsubbridge",
"textsubsquare", "textsubrhalfring", "textsublhalfring", "textsubplus",
"textovercross", "textsubarch", "textsuperimposetilde", "textraising",
"textlowering", "textadvancing", "textretracting", "textdoublegrave",
"texthighrise", "textlowrise", "textrisefall", "textsyllabic",
"textsubring", 0};

/// TIPA tones that need special handling
char const * const known_tones[] = {"15", "51", "45", "12", "454", 0};

// string to store the float type to be able to determine the type of subfloats
string float_type = "";


/// splits "x=z, y=b" into a map and an ordered keyword vector
void split_map(string const & s, map<string, string> & res, vector<string> & keys)
{
	vector<string> v;
	split(s, v);
	res.clear();
	keys.resize(v.size());
	for (size_t i = 0; i < v.size(); ++i) {
		size_t const pos   = v[i].find('=');
		string const index = trimSpaceAndEol(v[i].substr(0, pos));
		string const value = trimSpaceAndEol(v[i].substr(pos + 1, string::npos));
		res[index] = value;
		keys[i] = index;
	}
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
		value = trimSpaceAndEol(string(length, 0, i));
	}
	if (value == "-")
		value = "-1.0";
	// 'cM' is a valid LaTeX length unit. Change it to 'cm'
	if (contains(len, '\\'))
		unit = trimSpaceAndEol(string(len, i));
	else
		unit = ascii_lowercase(trimSpaceAndEol(string(len, i)));
	return true;
}


/// A simple function to translate a latex length to something LyX can
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
	for (char const * const * what = extensions; *what; ++what) {
		string const trial = addExtension(name, *what);
		if (makeAbsPath(trial, path).exists())
			return trial;
	}
	return string();
}


void begin_inset(ostream & os, string const & name)
{
	os << "\n\\begin_inset " << name;
}


void begin_command_inset(ostream & os, string const & name,
                         string const & latexname)
{
	begin_inset(os, "CommandInset ");
	os << name << "\nLatexCommand " << latexname << '\n';
}


void end_inset(ostream & os)
{
	os << "\n\\end_inset\n\n";
}


bool skip_braces(Parser & p)
{
	if (p.next_token().cat() != catBegin)
		return false;
	p.get_token();
	if (p.next_token().cat() == catEnd) {
		p.get_token();
		return true;
	}
	p.putback();
	return false;
}


/// replace LaTeX commands in \p s from the unicodesymbols file with their
/// unicode points
docstring convert_unicodesymbols(docstring s)
{
	odocstringstream os;
	for (size_t i = 0; i < s.size();) {
		if (s[i] != '\\') {
			os.put(s[i++]);
			continue;
		}
		s = s.substr(i);
		bool termination;
		docstring rem;
		set<string> req;
		docstring parsed = encodings.fromLaTeXCommand(s,
				Encodings::TEXT_CMD, termination, rem, &req);
		set<string>::const_iterator it = req.begin();
		set<string>::const_iterator en = req.end();
		for (; it != en; ++it)
			preamble.registerAutomaticallyLoadedPackage(*it);
		os << parsed;
		s = rem;
		if (s.empty() || s[0] != '\\')
			i = 0;
		else
			i = 1;
	}
	return os.str();
}


/// try to convert \p s to a valid InsetCommand argument
string convert_command_inset_arg(string s)
{
	if (isAscii(s))
		// since we don't know the input encoding we can't use from_utf8
		s = to_utf8(convert_unicodesymbols(from_ascii(s)));
	// LyX cannot handle newlines in a latex command
	return subst(s, "\n", " ");
}


void output_ert(ostream & os, string const & s, Context & context)
{
	context.check_layout(os);
	for (string::const_iterator it = s.begin(), et = s.end(); it != et; ++it) {
		if (*it == '\\')
			os << "\n\\backslash\n";
		else if (*it == '\n') {
			context.new_paragraph(os);
			context.check_layout(os);
		} else
			os << *it;
	}
	context.check_end_layout(os);
}


void output_ert_inset(ostream & os, string const & s, Context & context)
{
	// We must have a valid layout before outputting the ERT inset.
	context.check_layout(os);
	Context newcontext(true, context.textclass);
	InsetLayout const & layout = context.textclass.insetLayout(from_ascii("ERT"));
	if (layout.forcePlainLayout())
		newcontext.layout = &context.textclass.plainLayout();
	begin_inset(os, "ERT");
	os << "\nstatus collapsed\n";
	output_ert(os, s, newcontext);
	end_inset(os);
}


Layout const * findLayout(TextClass const & textclass, string const & name, bool command)
{
	Layout const * layout = findLayoutWithoutModule(textclass, name, command);
	if (layout)
		return layout;
	if (checkModule(name, command))
		return findLayoutWithoutModule(textclass, name, command);
	return layout;
}


InsetLayout const * findInsetLayout(TextClass const & textclass, string const & name, bool command)
{
	InsetLayout const * insetlayout = findInsetLayoutWithoutModule(textclass, name, command);
	if (insetlayout)
		return insetlayout;
	if (checkModule(name, command))
		return findInsetLayoutWithoutModule(textclass, name, command);
	return insetlayout;
}


void eat_whitespace(Parser &, ostream &, Context &, bool);


/*!
 * Skips whitespace and braces.
 * This should be called after a command has been parsed that is not put into
 * ERT, and where LyX adds "{}" if needed.
 */
void skip_spaces_braces(Parser & p, bool keepws = false)
{
	/* The following four examples produce the same typeset output and
	   should be handled by this function:
	   - abc \j{} xyz
	   - abc \j {} xyz
	   - abc \j
	     {} xyz
	   - abc \j %comment
	     {} xyz
	 */
	// Unfortunately we need to skip comments, too.
	// We can't use eat_whitespace since writing them after the {}
	// results in different output in some cases.
	bool const skipped_spaces = p.skip_spaces(true);
	bool const skipped_braces = skip_braces(p);
	if (keepws && skipped_spaces && !skipped_braces)
		// put back the space (it is better handled by check_space)
		p.unskip_spaces(true);
}


void output_arguments(ostream & os, Parser & p, bool outer, bool need_layout, bool post,
                      Context & context, Layout::LaTeXArgMap const & latexargs)
{
	if (need_layout) {
		context.check_layout(os);
		need_layout = false;
	} else
		need_layout = true;
	int i = 0;
	Layout::LaTeXArgMap::const_iterator lait = latexargs.begin();
	Layout::LaTeXArgMap::const_iterator const laend = latexargs.end();
	for (; lait != laend; ++lait) {
		++i;
		eat_whitespace(p, os, context, false);
		if (lait->second.mandatory) {
			if (p.next_token().cat() != catBegin)
				break;
			p.get_token(); // eat '{'
			if (need_layout) {
				context.check_layout(os);
				need_layout = false;
			}
			begin_inset(os, "Argument ");
			if (post)
				os << "post:";
			os << i << "\nstatus collapsed\n\n";
			parse_text_in_inset(p, os, FLAG_BRACE_LAST, outer, context);
			end_inset(os);
		} else {
			if (p.next_token().cat() == catEscape ||
			    p.next_token().character() != '[')
				continue;
			p.get_token(); // eat '['
			if (need_layout) {
				context.check_layout(os);
				need_layout = false;
			}
			begin_inset(os, "Argument ");
			if (post)
				os << "post:";
			os << i << "\nstatus collapsed\n\n";
			parse_text_in_inset(p, os, FLAG_BRACK_LAST, outer, context);
			end_inset(os);
		}
		eat_whitespace(p, os, context, false);
	}
}


void output_command_layout(ostream & os, Parser & p, bool outer,
			   Context & parent_context,
			   Layout const * newlayout)
{
	TeXFont const oldFont = parent_context.font;
	// save the current font size
	string const size = oldFont.size;
	// reset the font size to default, because the font size switches
	// don't affect section headings and the like
	parent_context.font.size = Context::normalfont.size;
	// we only need to write the font change if we have an open layout
	if (!parent_context.atParagraphStart())
		output_font_change(os, oldFont, parent_context.font);
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
	output_arguments(os, p, outer, true, false, context,
	                 context.layout->latexargs());
	parse_text(p, os, FLAG_ITEM, outer, context);
	output_arguments(os, p, outer, false, true, context,
	                 context.layout->postcommandargs());
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
	// Set the font size to the original value. No need to output it here
	// (Context::begin_layout() will do that if needed)
	parent_context.font.size = size;
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
void check_space(Parser & p, ostream & os, Context & context)
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
		case req_group:
			// This argument contains regular LaTeX
			output_ert_inset(os, ert + '{', context);
			eat_whitespace(p, os, context, false);
			if (template_arguments[i] == required)
				parse_text(p, os, FLAG_ITEM, outer, context);
			else
				parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			ert = "}";
			break;
		case item:
			// This argument consists only of a single item.
			// The presence of '{' or not must be preserved.
			p.skip_spaces();
			if (p.next_token().cat() == catBegin)
				ert += '{' + p.verbatim_item() + '}';
			else
				ert += p.verbatim_item();
			break;
		case displaymath:
		case verbatim:
			// This argument may contain special characters
			ert += '{' + p.verbatim_item() + '}';
			break;
		case optional:
		case opt_group:
			// true because we must not eat whitespace
			// if an optional arg follows we must not strip the
			// brackets from this one
			if (i < no_arguments - 1 &&
			    template_arguments[i+1] == optional)
				ert += p.getFullOpt(true);
			else
				ert += p.getOpt(true);
			break;
		}
	}
	output_ert_inset(os, ert, context);
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
void parse_box(Parser & p, ostream & os, unsigned outer_flags,
               unsigned inner_flags, bool outer, Context & parent_context,
               string const & outer_type, string const & special,
               string const & inner_type)
{
	string position;
	string inner_pos;
	string hor_pos = "c";
	// We need to set the height to the LaTeX default of 1\\totalheight
	// for the case when no height argument is given
	string height_value = "1";
	string height_unit = "in";
	string height_special = "totalheight";
	string latex_height;
	string width_value;
	string width_unit;
	string latex_width;
	string width_special = "none";
	if (!inner_type.empty() && p.hasOpt()) {
		if (inner_type != "makebox")
			position = p.getArg('[', ']');
		else {
			latex_width = p.getArg('[', ']');
			translate_box_len(latex_width, width_value, width_unit, width_special);
			position = "t";
		}
		if (position != "t" && position != "c" && position != "b") {
			cerr << "invalid position " << position << " for "
			     << inner_type << endl;
			position = "c";
		}
		if (p.hasOpt()) {
			if (inner_type != "makebox") {
				latex_height = p.getArg('[', ']');
				translate_box_len(latex_height, height_value, height_unit, height_special);
			} else {
				string const opt = p.getArg('[', ']');
				if (!opt.empty()) {
					hor_pos = opt;
					if (hor_pos != "l" && hor_pos != "c" &&
					    hor_pos != "r" && hor_pos != "s") {
						cerr << "invalid hor_pos " << hor_pos
						     << " for " << inner_type << endl;
						hor_pos = "c";
					}
				}
			}

			if (p.hasOpt()) {
				inner_pos = p.getArg('[', ']');
				if (inner_pos != "c" && inner_pos != "t" &&
				    inner_pos != "b" && inner_pos != "s") {
					cerr << "invalid inner_pos "
					     << inner_pos << " for "
					     << inner_type << endl;
					inner_pos = position;
				}
			}
		}
	}
	if (inner_type.empty()) {
		if (special.empty() && outer_type != "framebox")
			latex_width = "1\\columnwidth";
		else {
			Parser p2(special);
			latex_width = p2.getArg('[', ']');
			string const opt = p2.getArg('[', ']');
			if (!opt.empty()) {
				hor_pos = opt;
				if (hor_pos != "l" && hor_pos != "c" &&
				    hor_pos != "r" && hor_pos != "s") {
					cerr << "invalid hor_pos " << hor_pos
					     << " for " << outer_type << endl;
					hor_pos = "c";
				}
			}
		}
	} else if (inner_type != "makebox")
		latex_width = p.verbatim_item();
	// if e.g. only \ovalbox{content} was used, set the width to 1\columnwidth
	// as this is LyX's standard for such cases (except for makebox)
	// \framebox is more special and handled below
	if (latex_width.empty() && inner_type != "makebox"
		&& outer_type != "framebox")
		latex_width = "1\\columnwidth";

	translate_len(latex_width, width_value, width_unit);

	bool shadedparbox = false;
	if (inner_type == "shaded") {
		eat_whitespace(p, os, parent_context, false);
		if (outer_type == "parbox") {
			// Eat '{'
			if (p.next_token().cat() == catBegin)
				p.get_token();
			eat_whitespace(p, os, parent_context, false);
			shadedparbox = true;
		}
		p.get_token();
		p.getArg('{', '}');
	}
	// If we already read the inner box we have to push the inner env
	if (!outer_type.empty() && !inner_type.empty() &&
	    (inner_flags & FLAG_END))
		active_environments.push_back(inner_type);
	// LyX can't handle length variables
	bool use_ert = contains(width_unit, '\\') || contains(height_unit, '\\');
	if (!use_ert && !outer_type.empty() && !inner_type.empty()) {
		// Look whether there is some content after the end of the
		// inner box, but before the end of the outer box.
		// If yes, we need to output ERT.
		p.pushPosition();
		if (inner_flags & FLAG_END)
			p.ertEnvironment(inner_type);
		else
			p.verbatim_item();
		p.skip_spaces(true);
		bool const outer_env(outer_type == "framed" || outer_type == "minipage");
		if ((outer_env && p.next_token().asInput() != "\\end") ||
		    (!outer_env && p.next_token().cat() != catEnd)) {
			// something is between the end of the inner box and
			// the end of the outer box, so we need to use ERT.
			use_ert = true;
		}
		p.popPosition();
	}
	// if only \makebox{content} was used we can set its width to 1\width
	// because this identic and also identic to \mbox
	// this doesn't work for \framebox{content}, thus we have to use ERT for this
	if (latex_width.empty() && inner_type == "makebox") {
		width_value = "1";
		width_unit = "in";
		width_special = "width";
	} else if (latex_width.empty() && outer_type == "framebox") {
		width_value.clear();
		width_unit.clear();
		width_special = "none";
	}
	if (use_ert) {
		ostringstream ss;
		if (!outer_type.empty()) {
			if (outer_flags & FLAG_END)
				ss << "\\begin{" << outer_type << '}';
			else {
				ss << '\\' << outer_type << '{';
				if (!special.empty())
					ss << special;
			}
		}
		if (!inner_type.empty()) {
			if (inner_type != "shaded") {
				if (inner_flags & FLAG_END)
					ss << "\\begin{" << inner_type << '}';
				else
					ss << '\\' << inner_type;
			}
			if (!position.empty())
				ss << '[' << position << ']';
			if (!latex_height.empty())
				ss << '[' << latex_height << ']';
			if (!inner_pos.empty())
				ss << '[' << inner_pos << ']';
			ss << '{' << latex_width << '}';
			if (!(inner_flags & FLAG_END))
				ss << '{';
		}
		if (inner_type == "shaded")
			ss << "\\begin{shaded}";
		output_ert_inset(os, ss.str(), parent_context);
		if (!inner_type.empty()) {
			parse_text(p, os, inner_flags, outer, parent_context);
			if (inner_flags & FLAG_END)
				output_ert_inset(os, "\\end{" + inner_type + '}',
				           parent_context);
			else
				output_ert_inset(os, "}", parent_context);
		}
		if (!outer_type.empty()) {
			// If we already read the inner box we have to pop
			// the inner env
			if (!inner_type.empty() && (inner_flags & FLAG_END))
				active_environments.pop_back();

			// Ensure that the end of the outer box is parsed correctly:
			// The opening brace has been eaten by parse_outer_box()
			if (!outer_type.empty() && (outer_flags & FLAG_ITEM)) {
				outer_flags &= ~FLAG_ITEM;
				outer_flags |= FLAG_BRACE_LAST;
			}
			parse_text(p, os, outer_flags, outer, parent_context);
			if (outer_flags & FLAG_END)
				output_ert_inset(os, "\\end{" + outer_type + '}',
				           parent_context);
			else
				output_ert_inset(os, "}", parent_context);
		}
	} else {
		// LyX does not like empty positions, so we have
		// to set them to the LaTeX default values here.
		if (position.empty())
			position = "c";
		if (inner_pos.empty())
			inner_pos = position;
		parent_context.check_layout(os);
		begin_inset(os, "Box ");
		if (outer_type == "framed")
			os << "Framed\n";
		else if (outer_type == "framebox" || outer_type == "fbox")
			os << "Boxed\n";
		else if (outer_type == "shadowbox")
			os << "Shadowbox\n";
		else if ((outer_type == "shaded" && inner_type.empty()) ||
			     (outer_type == "minipage" && inner_type == "shaded") ||
			     (outer_type == "parbox" && inner_type == "shaded")) {
			os << "Shaded\n";
			preamble.registerAutomaticallyLoadedPackage("color");
		} else if (outer_type == "doublebox")
			os << "Doublebox\n";
		else if (outer_type.empty() || outer_type == "mbox")
			os << "Frameless\n";
		else
			os << outer_type << '\n';
		os << "position \"" << position << "\"\n";
		os << "hor_pos \"" << hor_pos << "\"\n";
		if (outer_type == "mbox")
			os << "has_inner_box 1\n";
		else
			os << "has_inner_box " << !inner_type.empty() << "\n";
		os << "inner_pos \"" << inner_pos << "\"\n";
		os << "use_parbox " << (inner_type == "parbox" || shadedparbox)
		   << '\n';
		if (outer_type == "mbox")
			os << "use_makebox 1\n";
		else
			os << "use_makebox " << (inner_type == "makebox") << '\n';
		if (outer_type == "fbox" || outer_type == "mbox")
			os << "width \"\"\n";
		else
			os << "width \"" << width_value << width_unit << "\"\n";
		os << "special \"" << width_special << "\"\n";
		os << "height \"" << height_value << height_unit << "\"\n";
		os << "height_special \"" << height_special << "\"\n";
		os << "status open\n\n";

		// Unfortunately we can't use parse_text_in_inset:
		// InsetBox::forcePlainLayout() is hard coded and does not
		// use the inset layout. Apart from that do we call parse_text
		// up to two times, but need only one check_end_layout.
		bool const forcePlainLayout =
			(!inner_type.empty() || inner_type == "makebox") &&
			outer_type != "shaded" && outer_type != "framed";
		Context context(true, parent_context.textclass);
		if (forcePlainLayout)
			context.layout = &context.textclass.plainLayout();
		else
			context.font = parent_context.font;

		// If we have no inner box the contents will be read with the outer box
		if (!inner_type.empty())
			parse_text(p, os, inner_flags, outer, context);

		// Ensure that the end of the outer box is parsed correctly:
		// The opening brace has been eaten by parse_outer_box()
		if (!outer_type.empty() && (outer_flags & FLAG_ITEM)) {
			outer_flags &= ~FLAG_ITEM;
			outer_flags |= FLAG_BRACE_LAST;
		}

		// Find end of outer box, output contents if inner_type is
		// empty and output possible comments
		if (!outer_type.empty()) {
			// If we already read the inner box we have to pop
			// the inner env
			if (!inner_type.empty() && (inner_flags & FLAG_END))
				active_environments.pop_back();
			// This does not output anything but comments if
			// inner_type is not empty (see use_ert)
			parse_text(p, os, outer_flags, outer, context);
		}

		context.check_end_layout(os);
		end_inset(os);
#ifdef PRESERVE_LAYOUT
		// LyX puts a % after the end of the minipage
		if (p.next_token().cat() == catNewline && p.next_token().cs().size() > 1) {
			// new paragraph
			//output_ert_inset(os, "%dummy", parent_context);
			p.get_token();
			p.skip_spaces();
			parent_context.new_paragraph(os);
		}
		else if (p.next_token().cat() == catSpace || p.next_token().cat() == catNewline) {
			//output_ert_inset(os, "%dummy", parent_context);
			p.get_token();
			p.skip_spaces();
			// We add a protected space if something real follows
			if (p.good() && p.next_token().cat() != catComment) {
				begin_inset(os, "space ~\n");
				end_inset(os);
			}
		}
#endif
	}
}


void parse_outer_box(Parser & p, ostream & os, unsigned flags, bool outer,
                     Context & parent_context, string const & outer_type,
                     string const & special)
{
	eat_whitespace(p, os, parent_context, false);
	if (flags & FLAG_ITEM) {
		// Eat '{'
		if (p.next_token().cat() == catBegin)
			p.get_token();
		else
			cerr << "Warning: Ignoring missing '{' after \\"
			     << outer_type << '.' << endl;
		eat_whitespace(p, os, parent_context, false);
	}
	string inner;
	unsigned int inner_flags = 0;
	p.pushPosition();
	if (outer_type == "minipage" || outer_type == "parbox") {
		p.skip_spaces(true);
		while (p.hasOpt()) {
			p.getArg('[', ']');
			p.skip_spaces(true);
		}
		p.getArg('{', '}');
		p.skip_spaces(true);
		if (outer_type == "parbox") {
			// Eat '{'
			if (p.next_token().cat() == catBegin)
				p.get_token();
			p.skip_spaces(true);
		}
	}
	if (outer_type == "shaded" || outer_type == "fbox"
		|| outer_type == "mbox") {
		// These boxes never have an inner box
		;
	} else if (p.next_token().asInput() == "\\parbox") {
		inner = p.get_token().cs();
		inner_flags = FLAG_ITEM;
	} else if (p.next_token().asInput() == "\\begin") {
		// Is this a minipage or shaded box?
		p.pushPosition();
		p.get_token();
		inner = p.getArg('{', '}');
		p.popPosition();
		if (inner == "minipage" || inner == "shaded")
			inner_flags = FLAG_END;
		else
			inner = "";
	}
	p.popPosition();
	if (inner_flags == FLAG_END) {
		if (inner != "shaded")
		{
			p.get_token();
			p.getArg('{', '}');
			eat_whitespace(p, os, parent_context, false);
		}
		parse_box(p, os, flags, FLAG_END, outer, parent_context,
		          outer_type, special, inner);
	} else {
		if (inner_flags == FLAG_ITEM) {
			p.get_token();
			eat_whitespace(p, os, parent_context, false);
		}
		parse_box(p, os, flags, inner_flags, outer, parent_context,
		          outer_type, special, inner);
	}
}


void parse_listings(Parser & p, ostream & os, Context & parent_context, bool in_line)
{
	parent_context.check_layout(os);
	begin_inset(os, "listings\n");
	if (p.hasOpt()) {
		string arg = p.verbatimOption();
		os << "lstparams " << '"' << arg << '"' << '\n';
		if (arg.find("\\color") != string::npos)
	                preamble.registerAutomaticallyLoadedPackage("color");
	}
	if (in_line)
		os << "inline true\n";
	else
		os << "inline false\n";
	os << "status collapsed\n";
	Context context(true, parent_context.textclass);
	context.layout = &parent_context.textclass.plainLayout();
	string s;
	if (in_line) {
		// set catcodes to verbatim early, just in case.
		p.setCatcodes(VERBATIM_CATCODES);
		string delim = p.get_token().asInput();
		//FIXME: handler error condition
		s = p.verbatimStuff(delim).second;
//		context.new_paragraph(os);
	} else
		s = p.verbatimEnvironment("lstlisting");
	output_ert(os, s, context);
	end_inset(os);
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
	output_ert_inset(os, "\\begin{" + name + "}", parent_context);
	parse_text_snippet(p, os, flags, outer, parent_context);
	output_ert_inset(os, "\\end{" + name + "}", parent_context);
	if (specialfont)
		parent_context.new_layout_allowed = new_layout_allowed;
}


void parse_environment(Parser & p, ostream & os, bool outer,
                       string & last_env, Context & parent_context)
{
	Layout const * newlayout;
	InsetLayout const * newinsetlayout = 0;
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
		if (is_display_math_env(name)) {
			// Prevent the conversion of a line break to a space
			// (bug 7668). This does not change the output, but
			// looks ugly in LyX.
			eat_whitespace(p, os, parent_context, false);
		}
	}

	else if (is_known(name, preamble.polyglossia_languages)) {
		// We must begin a new paragraph if not already done
		if (! parent_context.atParagraphStart()) {
			parent_context.check_end_layout(os);
			parent_context.new_paragraph(os);
		}
		// save the language in the context so that it is
		// handled by parse_text
		parent_context.font.language = preamble.polyglossia2lyx(name);
		parse_text(p, os, FLAG_END, outer, parent_context);
		// Just in case the environment is empty
		parent_context.extra_stuff.erase();
		// We must begin a new paragraph to reset the language
		parent_context.new_paragraph(os);
		p.skip_spaces();
	}

	else if (unstarred_name == "tabular" || name == "longtable") {
		eat_whitespace(p, os, parent_context, false);
		string width = "0pt";
		if (name == "tabular*") {
			width = lyx::translate_len(p.getArg('{', '}'));
			eat_whitespace(p, os, parent_context, false);
		}
		parent_context.check_layout(os);
		begin_inset(os, "Tabular ");
		handle_tabular(p, os, name, width, parent_context);
		end_inset(os);
		p.skip_spaces();
	}

	else if (parent_context.textclass.floats().typeExist(unstarred_name)) {
		eat_whitespace(p, os, parent_context, false);
		string const opt = p.hasOpt() ? p.getArg('[', ']') : string();
		eat_whitespace(p, os, parent_context, false);
		parent_context.check_layout(os);
		begin_inset(os, "Float " + unstarred_name + "\n");
		// store the float type for subfloats
		// subfloats only work with figures and tables
		if (unstarred_name == "figure")
			float_type = unstarred_name;
		else if (unstarred_name == "table")
			float_type = unstarred_name;
		else
			float_type = "";
		if (!opt.empty())
			os << "placement " << opt << '\n';
		if (contains(opt, "H"))
                	preamble.registerAutomaticallyLoadedPackage("float");
		else {
			Floating const & fl = parent_context.textclass.floats()
				.getType(unstarred_name);
		        if (!fl.floattype().empty() && fl.usesFloatPkg())
                		preamble.registerAutomaticallyLoadedPackage("float");
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
		// the float is parsed thus delete the type
		float_type = "";
	}

	else if (unstarred_name == "sidewaysfigure"
		|| unstarred_name == "sidewaystable") {
		eat_whitespace(p, os, parent_context, false);
		parent_context.check_layout(os);
		if (unstarred_name == "sidewaysfigure")
			begin_inset(os, "Float figure\n");
		else
			begin_inset(os, "Float table\n");
		os << "wide " << convert<string>(is_starred)
		   << "\nsideways true"
		   << "\nstatus open\n\n";
		parse_text_in_inset(p, os, FLAG_END, outer, parent_context);
		end_inset(os);
		// We don't need really a new paragraph, but
		// we must make sure that the next item gets a \begin_layout.
		parent_context.new_paragraph(os);
		p.skip_spaces();
		preamble.registerAutomaticallyLoadedPackage("rotfloat");
	}

	else if (name == "wrapfigure" || name == "wraptable") {
		// syntax is \begin{wrapfigure}[lines]{placement}[overhang]{width}
		eat_whitespace(p, os, parent_context, false);
		parent_context.check_layout(os);
		// default values
		string lines = "0";
		string overhang = "0col%";
		// parse
		if (p.hasOpt())
			lines = p.getArg('[', ']');
		string const placement = p.getArg('{', '}');
		if (p.hasOpt())
			overhang = p.getArg('[', ']');
		string const width = p.getArg('{', '}');
		// write
		if (name == "wrapfigure")
			begin_inset(os, "Wrap figure\n");
		else
			begin_inset(os, "Wrap table\n");
		os << "lines " << lines
		   << "\nplacement " << placement
		   << "\noverhang " << lyx::translate_len(overhang)
		   << "\nwidth " << lyx::translate_len(width)
		   << "\nstatus open\n\n";
		parse_text_in_inset(p, os, FLAG_END, outer, parent_context);
		end_inset(os);
		// We don't need really a new paragraph, but
		// we must make sure that the next item gets a \begin_layout.
		parent_context.new_paragraph(os);
		p.skip_spaces();
		preamble.registerAutomaticallyLoadedPackage("wrapfig");
	}

	else if (name == "minipage") {
		eat_whitespace(p, os, parent_context, false);
		// Test whether this is an outer box of a shaded box
		p.pushPosition();
		// swallow arguments
		while (p.hasOpt()) {
			p.getArg('[', ']');
			p.skip_spaces(true);
		}
		p.getArg('{', '}');
		p.skip_spaces(true);
		Token t = p.get_token();
		bool shaded = false;
		if (t.asInput() == "\\begin") {
			p.skip_spaces(true);
			if (p.getArg('{', '}') == "shaded")
				shaded = true;
		}
		p.popPosition();
		if (shaded)
			parse_outer_box(p, os, FLAG_END, outer,
			                parent_context, name, "shaded");
		else
			parse_box(p, os, 0, FLAG_END, outer, parent_context,
			          "", "", name);
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
		skip_braces(p); // eat {} that might by set by LyX behind comments
		preamble.registerAutomaticallyLoadedPackage("verbatim");
	}

	else if (name == "verbatim") {
		// FIXME: this should go in the generic code that
		// handles environments defined in layout file that
		// have "PassThru 1". However, the code over there is
		// already too complicated for my taste.
		parent_context.new_paragraph(os);
		Context context(true, parent_context.textclass,
				&parent_context.textclass[from_ascii("Verbatim")]);
		string s = p.verbatimEnvironment("verbatim");
		output_ert(os, s, context);
		p.skip_spaces();
	}

	else if (name == "IPA") {
		eat_whitespace(p, os, parent_context, false);
		parent_context.check_layout(os);
		begin_inset(os, "IPA\n");
		parse_text_in_inset(p, os, FLAG_END, outer, parent_context);
		end_inset(os);
		p.skip_spaces();
		preamble.registerAutomaticallyLoadedPackage("tipa");
		preamble.registerAutomaticallyLoadedPackage("tipx");
	}

	else if (name == "CJK") {
		// the scheme is \begin{CJK}{encoding}{mapping}text\end{CJK}
		// It is impossible to decide if a CJK environment was in its own paragraph or within
		// a line. We therefore always assume a paragraph since the latter is a rare case.
		eat_whitespace(p, os, parent_context, false);
		parent_context.check_end_layout(os);
		// store the encoding to be able to reset it
		string const encoding_old = p.getEncoding();
		string const encoding = p.getArg('{', '}');
		// FIXME: For some reason JIS does not work. Although the text
		// in tests/CJK.tex is identical with the SJIS version if you
		// convert both snippets using the recode command line utility,
		// the resulting .lyx file contains some extra characters if
		// you set buggy_encoding to false for JIS.
		bool const buggy_encoding = encoding == "JIS";
		if (!buggy_encoding)
			p.setEncoding(encoding, Encoding::CJK);
		else {
			// FIXME: This will read garbage, since the data is not encoded in utf8.
			p.setEncoding("UTF-8");
		}
		// LyX only supports the same mapping for all CJK
		// environments, so we might need to output everything as ERT
		string const mapping = trim(p.getArg('{', '}'));
		char const * const * const where =
			is_known(encoding, supported_CJK_encodings);
		if (!buggy_encoding && !preamble.fontCJKSet())
			preamble.fontCJK(mapping);
		bool knownMapping = mapping == preamble.fontCJK();
		if (buggy_encoding || !knownMapping || !where) {
			parent_context.check_layout(os);
			output_ert_inset(os, "\\begin{" + name + "}{" + encoding + "}{" + mapping + "}",
				       parent_context);
			// we must parse the content as verbatim because e.g. JIS can contain
			// normally invalid characters
			// FIXME: This works only for the most simple cases.
			//        Since TeX control characters are not parsed,
			//        things like comments are completely wrong.
			string const s = p.plainEnvironment("CJK");
			for (string::const_iterator it = s.begin(), et = s.end(); it != et; ++it) {
				if (*it == '\\')
					output_ert_inset(os, "\\", parent_context);
				else if (*it == '$')
					output_ert_inset(os, "$", parent_context);
				else if (*it == '\n' && it + 1 != et && s.begin() + 1 != it)
					os << "\n ";
				else
					os << *it;
			}
			output_ert_inset(os, "\\end{" + name + "}",
				       parent_context);
		} else {
			string const lang =
				supported_CJK_languages[where - supported_CJK_encodings];
			// store the language because we must reset it at the end
			string const lang_old = parent_context.font.language;
			parent_context.font.language = lang;
			parse_text_in_inset(p, os, FLAG_END, outer, parent_context);
			parent_context.font.language = lang_old;
			parent_context.new_paragraph(os);
		}
		p.setEncoding(encoding_old);
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
		if (!preamble.notefontcolor().empty())
			preamble.registerAutomaticallyLoadedPackage("color");
	}

	else if (name == "btSect") {
		eat_whitespace(p, os, parent_context, false);
		parent_context.check_layout(os);
		begin_command_inset(os, "bibtex", "bibtex");
		string bibstyle = "plain";
		if (p.hasOpt()) {
			bibstyle = p.getArg('[', ']');
			p.skip_spaces(true);
		}
		string const bibfile = p.getArg('{', '}');
		eat_whitespace(p, os, parent_context, false);
		Token t = p.get_token();
		if (t.asInput() == "\\btPrintCited") {
			p.skip_spaces(true);
			os << "btprint " << '"' << "btPrintCited" << '"' << "\n";
		}
		if (t.asInput() == "\\btPrintNotCited") {
			p.skip_spaces(true);
			os << "btprint " << '"' << "btPrintNotCited" << '"' << "\n";
		}
		if (t.asInput() == "\\btPrintAll") {
			p.skip_spaces(true);
			os << "btprint " << '"' << "btPrintAll" << '"' << "\n";
		}
		os << "bibfiles " << '"' << bibfile << '"' << "\n";
		os << "options " << '"' << bibstyle << '"' <<  "\n";
		parse_text_in_inset(p, os, FLAG_END, outer, parent_context);
		end_inset(os);
		p.skip_spaces();
	}

	else if (name == "framed" || name == "shaded") {
		eat_whitespace(p, os, parent_context, false);
		parse_outer_box(p, os, FLAG_END, outer, parent_context, name, "");
		p.skip_spaces();
	}

	else if (name == "lstlisting") {
		eat_whitespace(p, os, parent_context, false);
		parse_listings(p, os, parent_context, false);
		p.skip_spaces();
	}

	else if (!parent_context.new_layout_allowed)
		parse_unknown_environment(p, name, os, FLAG_END, outer,
					  parent_context);

	// Alignment and spacing settings
	// FIXME (bug xxxx): These settings can span multiple paragraphs and
	//					 therefore are totally broken!
	// Note that \centering, raggedright, and raggedleft cannot be handled, as
	// they are commands not environments. They are furthermore switches that
	// can be ended by another switches, but also by commands like \footnote or
	// \parbox. So the only safe way is to leave them untouched.
	else if (name == "center" || name == "centering" ||
		 name == "flushleft" || name == "flushright" ||
		 name == "singlespace" || name == "onehalfspace" ||
		 name == "doublespace" || name == "spacing") {
		eat_whitespace(p, os, parent_context, false);
		// We must begin a new paragraph if not already done
		if (! parent_context.atParagraphStart()) {
			parent_context.check_end_layout(os);
			parent_context.new_paragraph(os);
		}
		if (name == "flushleft")
			parent_context.add_extra_stuff("\\align left\n");
		else if (name == "flushright")
			parent_context.add_extra_stuff("\\align right\n");
		else if (name == "center" || name == "centering")
			parent_context.add_extra_stuff("\\align center\n");
		else if (name == "singlespace")
			parent_context.add_extra_stuff("\\paragraph_spacing single\n");
		else if (name == "onehalfspace") {
			parent_context.add_extra_stuff("\\paragraph_spacing onehalf\n");
			preamble.registerAutomaticallyLoadedPackage("setspace");
		} else if (name == "doublespace") {
			parent_context.add_extra_stuff("\\paragraph_spacing double\n");
			preamble.registerAutomaticallyLoadedPackage("setspace");
		} else if (name == "spacing") {
			parent_context.add_extra_stuff("\\paragraph_spacing other " + p.verbatim_item() + "\n");
			preamble.registerAutomaticallyLoadedPackage("setspace");
		}
		parse_text(p, os, FLAG_END, outer, parent_context);
		// Just in case the environment is empty
		parent_context.extra_stuff.erase();
		// We must begin a new paragraph to reset the alignment
		parent_context.new_paragraph(os);
		p.skip_spaces();
	}

	// The single '=' is meant here.
	else if ((newlayout = findLayout(parent_context.textclass, name, false))) {
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
		if (last_env == name) {
			// we need to output a separator since LyX would export
			// the two environments as one otherwise (bug 5716)
			docstring const sep = from_ascii("--Separator--");
			TeX2LyXDocClass const & textclass(parent_context.textclass);
			if (textclass.hasLayout(sep)) {
				Context newcontext(parent_context);
				newcontext.layout = &(textclass[sep]);
				newcontext.check_layout(os);
				newcontext.check_end_layout(os);
			} else {
				parent_context.check_layout(os);
				begin_inset(os, "Note Note\n");
				os << "status closed\n";
				Context newcontext(true, textclass,
						&(textclass.defaultLayout()));
				newcontext.check_layout(os);
				newcontext.check_end_layout(os);
				end_inset(os);
				parent_context.check_end_layout(os);
			}
		}
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
		// handle known optional and required arguments
		// Unfortunately LyX can't handle arguments of list arguments (bug 7468):
		// It is impossible to place anything after the environment name,
		// but before the first \\item.
		if (context.layout->latextype == LATEX_ENVIRONMENT)
			output_arguments(os, p, outer, false, false, context,
			                 context.layout->latexargs());
		parse_text(p, os, FLAG_END, outer, context);
		if (context.layout->latextype == LATEX_ENVIRONMENT)
			output_arguments(os, p, outer, false, true, context,
			                 context.layout->postcommandargs());
		context.check_end_layout(os);
		if (parent_context.deeper_paragraph) {
			// We must suppress the "end deeper" because we
			// suppressed the "begin deeper" above.
			context.need_end_deeper = false;
		}
		context.check_end_deeper(os);
		parent_context.new_paragraph(os);
		p.skip_spaces();
		if (!preamble.titleLayoutFound())
			preamble.titleLayoutFound(newlayout->intitle);
		set<string> const & req = newlayout->requires();
		set<string>::const_iterator it = req.begin();
		set<string>::const_iterator en = req.end();
		for (; it != en; ++it)
			preamble.registerAutomaticallyLoadedPackage(*it);
	}

	// The single '=' is meant here.
	else if ((newinsetlayout = findInsetLayout(parent_context.textclass, name, false))) {
		eat_whitespace(p, os, parent_context, false);
		parent_context.check_layout(os);
		begin_inset(os, "Flex ");
		os << to_utf8(newinsetlayout->name()) << '\n'
		   << "status collapsed\n";
		if (newinsetlayout->isPassThru()) {
			string const arg = p.verbatimEnvironment(name);
			Context context(true, parent_context.textclass, 
					&parent_context.textclass.plainLayout(),
					parent_context.layout);
			output_ert(os, arg, parent_context);
		} else
			parse_text_in_inset(p, os, FLAG_END, false, parent_context, newinsetlayout);
		end_inset(os);
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
			output_ert_inset(os, p.ertEnvironment(name),
				   parent_context);
		else
			parse_text_snippet(p, os, FLAG_END, outer,
					   parent_context);
		output_ert_inset(os, "\\end{" + name + "}", parent_context);
		if (specialfont)
			parent_context.new_layout_allowed = new_layout_allowed;
	}

	else
		parse_unknown_environment(p, name, os, FLAG_END, outer,
					  parent_context);

	last_env = name;
	active_environments.pop_back();
}


/// parses a comment and outputs it to \p os.
void parse_comment(Parser & p, ostream & os, Token const & t, Context & context)
{
	LASSERT(t.cat() == catComment, return);
	if (!t.cs().empty()) {
		context.check_layout(os);
		output_ert_inset(os, '%' + t.cs(), context);
		if (p.next_token().cat() == catNewline) {
			// A newline after a comment line starts a new
			// paragraph
			if (context.new_layout_allowed) {
				if(!context.atParagraphStart())
					// Only start a new paragraph if not already
					// done (we might get called recursively)
					context.new_paragraph(os);
			} else
				output_ert_inset(os, "\n", context);
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
void get_cite_arguments(Parser & p, bool natbibOrder,
	string & before, string & after)
{
	// We need to distinguish "" and "[]", so we can't use p.getOpt().

	// text before the citation
	before.clear();
	// text after the citation
	after = p.getFullOpt();

	if (!after.empty()) {
		before = p.getFullOpt();
		if (natbibOrder && !before.empty())
			swap(before, after);
	}
}


/// Convert filenames with TeX macros and/or quotes to something LyX
/// can understand
string const normalize_filename(string const & name)
{
	Parser p(name);
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
		} else if (t.cs() == "string") {
			// Convert \string" to " and \string~ to ~
			Token const & n = p.next_token();
			if (n.asInput() != "\"" && n.asInput() != "~")
				os << t.asInput();
		} else
			os << t.asInput();
	}
	// Strip quotes. This is a bit complicated (see latex_path()).
	string full = os.str();
	if (!full.empty() && full[0] == '"') {
		string base = removeExtension(full);
		string ext = getExtension(full);
		if (!base.empty() && base[base.length()-1] == '"')
			// "a b"
			// "a b".tex
			return addExtension(trim(base, "\""), ext);
		if (full[full.length()-1] == '"')
			// "a b.c"
			// "a b.c".tex
			return trim(full, "\"");
	}
	return full;
}


/// Convert \p name from TeX convention (relative to master file) to LyX
/// convention (relative to .lyx file) if it is relative
void fix_child_filename(string & name)
{
	string const absMasterTeX = getMasterFilePath(true);
	bool const isabs = FileName::isAbsolute(name);
	// convert from "relative to .tex master" to absolute original path
	if (!isabs)
		name = makeAbsPath(name, absMasterTeX).absFileName();
	bool copyfile = copyFiles();
	string const absParentLyX = getParentFilePath(false);
	string abs = name;
	if (copyfile) {
		// convert from absolute original path to "relative to master file"
		string const rel = to_utf8(makeRelPath(from_utf8(name),
		                                       from_utf8(absMasterTeX)));
		// re-interpret "relative to .tex file" as "relative to .lyx file"
		// (is different if the master .lyx file resides in a
		// different path than the master .tex file)
		string const absMasterLyX = getMasterFilePath(false);
		abs = makeAbsPath(rel, absMasterLyX).absFileName();
		// Do not copy if the new path is impossible to create. Example:
		// absMasterTeX = "/foo/bar/"
		// absMasterLyX = "/bar/"
		// name = "/baz.eps" => new absolute name would be "/../baz.eps"
		if (contains(name, "/../"))
			copyfile = false;
	}
	if (copyfile) {
		if (isabs)
			name = abs;
		else {
			// convert from absolute original path to
			// "relative to .lyx file"
			name = to_utf8(makeRelPath(from_utf8(abs),
			                           from_utf8(absParentLyX)));
		}
	}
	else if (!isabs) {
		// convert from absolute original path to "relative to .lyx file"
		name = to_utf8(makeRelPath(from_utf8(name),
		                           from_utf8(absParentLyX)));
	}
}


void copy_file(FileName const & src, string dstname)
{
	if (!copyFiles())
		return;
	string const absParent = getParentFilePath(false);
	FileName dst;
	if (FileName::isAbsolute(dstname))
		dst = FileName(dstname);
	else
		dst = makeAbsPath(dstname, absParent);
	string const absMaster = getMasterFilePath(false);
	FileName const srcpath = src.onlyPath();
	FileName const dstpath = dst.onlyPath();
	if (equivalent(srcpath, dstpath))
		return;
	if (!dstpath.isDirectory()) {
		if (!dstpath.createPath()) {
			cerr << "Warning: Could not create directory for file `"
			     << dst.absFileName() << "´." << endl;
			return;
		}
	}
	if (dst.isReadableFile()) {
		if (overwriteFiles())
			cerr << "Warning: Overwriting existing file `"
			     << dst.absFileName() << "´." << endl;
		else {
			cerr << "Warning: Not overwriting existing file `"
			     << dst.absFileName() << "´." << endl;
			return;
		}
	}
	if (!src.copyTo(dst))
		cerr << "Warning: Could not copy file `" << src.absFileName()
		     << "´ to `" << dst.absFileName() << "´." << endl;
}


/// Parse a literate Chunk section. The initial "<<" is already parsed.
bool parse_chunk(Parser & p, ostream & os, Context & context)
{
	// check whether a chunk is possible here.
	if (!context.textclass.hasInsetLayout(from_ascii("Flex:Chunk"))) {
		return false;
	}

	p.pushPosition();

	// read the parameters
	Parser::Arg const params = p.verbatimStuff(">>=\n", false);
	if (!params.first) {
		p.popPosition();
		return false;
	}

	Parser::Arg const code = p.verbatimStuff("\n@");
	if (!code.first) {
		p.popPosition();
		return false;
	}
	string const post_chunk = p.verbatimStuff("\n").second + '\n';
	if (post_chunk[0] != ' ' && post_chunk[0] != '\n') {
		p.popPosition();
		return false;
	}
	// The last newline read is important for paragraph handling
	p.putback();
	p.deparse();

	//cerr << "params=[" << params.second << "], code=[" << code.second << "]" <<endl;
	// We must have a valid layout before outputting the Chunk inset.
	context.check_layout(os);
	Context chunkcontext(true, context.textclass);
	chunkcontext.layout = &context.textclass.plainLayout();
	begin_inset(os, "Flex Chunk");
	os << "\nstatus open\n";
	if (!params.second.empty()) {
		chunkcontext.check_layout(os);
		Context paramscontext(true, context.textclass);
		paramscontext.layout = &context.textclass.plainLayout();
		begin_inset(os, "Argument 1");
		os << "\nstatus open\n";
		output_ert(os, params.second, paramscontext);
		end_inset(os);
	}
	output_ert(os, code.second, chunkcontext);
	end_inset(os);

	p.dropPosition();
	return true;
}


/// detects \\def, \\long\\def and \\global\\long\\def with ws and comments
bool is_macro(Parser & p)
{
	Token first = p.curr_token();
	if (first.cat() != catEscape || !p.good())
		return false;
	if (first.cs() == "def")
		return true;
	if (first.cs() != "global" && first.cs() != "long")
		return false;
	Token second = p.get_token();
	int pos = 1;
	while (p.good() && !p.isParagraph() && (second.cat() == catSpace ||
	       second.cat() == catNewline || second.cat() == catComment)) {
		second = p.get_token();
		pos++;
	}
	bool secondvalid = second.cat() == catEscape;
	Token third;
	bool thirdvalid = false;
	if (p.good() && first.cs() == "global" && secondvalid &&
	    second.cs() == "long") {
		third = p.get_token();
		pos++;
		while (p.good() && !p.isParagraph() &&
		       (third.cat() == catSpace ||
		        third.cat() == catNewline ||
		        third.cat() == catComment)) {
			third = p.get_token();
			pos++;
		}
		thirdvalid = third.cat() == catEscape;
	}
	for (int i = 0; i < pos; ++i)
		p.putback();
	if (!secondvalid)
		return false;
	if (!thirdvalid)
		return (first.cs() == "global" || first.cs() == "long") &&
		       second.cs() == "def";
	return first.cs() == "global" && second.cs() == "long" &&
	       third.cs() == "def";
}


/// Parse a macro definition (assumes that is_macro() returned true)
void parse_macro(Parser & p, ostream & os, Context & context)
{
	context.check_layout(os);
	Token first = p.curr_token();
	Token second;
	Token third;
	string command = first.asInput();
	if (first.cs() != "def") {
		p.get_token();
		eat_whitespace(p, os, context, false);
		second = p.curr_token();
		command += second.asInput();
		if (second.cs() != "def") {
			p.get_token();
			eat_whitespace(p, os, context, false);
			third = p.curr_token();
			command += third.asInput();
		}
	}
	eat_whitespace(p, os, context, false);
	string const name = p.get_token().cs();
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
				string s = p.get_token().asInput();
				paramtext += s;
				// number = current arity + 1?
				if (s.size() == 1 && s[0] == arity + '0' + 1)
					++arity;
				else
					simple = false;
			} else
				paramtext += p.get_token().cs();
		} else {
			paramtext += p.get_token().cs();
			simple = false;
		}
	}

	// only output simple (i.e. compatible) macro as FormulaMacros
	string ert = '\\' + name + ' ' + paramtext + '{' + p.verbatim_item() + '}';
	if (simple) {
		context.check_layout(os);
		begin_inset(os, "FormulaMacro");
		os << "\n\\def" << ert;
		end_inset(os);
	} else
		output_ert_inset(os, command + ert, context);
}


void registerExternalTemplatePackages(string const & name)
{
	external::TemplateManager const & etm = external::TemplateManager::get();
	external::Template const * const et = etm.getTemplateByName(name);
	if (!et)
		return;
	external::Template::Formats::const_iterator cit = et->formats.end();
	if (pdflatex)
	        cit = et->formats.find("PDFLaTeX");
	if (cit == et->formats.end())
		// If the template has not specified a PDFLaTeX output,
		// we try the LaTeX format.
		cit = et->formats.find("LaTeX");
	if (cit == et->formats.end())
		return;
	vector<string>::const_iterator qit = cit->second.requirements.begin();
	vector<string>::const_iterator qend = cit->second.requirements.end();
	for (; qit != qend; ++qit)
		preamble.registerAutomaticallyLoadedPackage(*qit);
}

} // anonymous namespace


void parse_text(Parser & p, ostream & os, unsigned flags, bool outer,
		Context & context)
{
	Layout const * newlayout = 0;
	InsetLayout const * newinsetlayout = 0;
	char const * const * where = 0;
	// Store the latest bibliographystyle, addcontentslineContent and
	// nocite{*} option (needed for bibtex inset)
	string btprint;
	string contentslineContent;
	string bibliographystyle = "default";
	bool const use_natbib = isProvided("natbib");
	bool const use_jurabib = isProvided("jurabib");
	string last_env;
	while (p.good()) {
		Token const & t = p.get_token();

	// it is impossible to determine the correct encoding for non-CJK Japanese.
	// Therefore write a note at the beginning of the document
	if (is_nonCJKJapanese) {
		context.check_layout(os);
		begin_inset(os, "Note Note\n");
		os << "status open\n\\begin_layout Plain Layout\n"
		   << "\\series bold\n"
		   << "Important information:\n"
		   << "\\end_layout\n\n"
		   << "\\begin_layout Plain Layout\n"
		   << "The original LaTeX source for this document is in Japanese (pLaTeX).\n"
		   << " It was therefore impossible for tex2lyx to determine the correct encoding.\n"
		   << " The iconv encoding " << p.getEncoding() << " was used.\n"
		   << " If this is incorrect, you must run the tex2lyx program on the command line\n"
		   << " and specify the encoding using the -e command-line switch.\n"
		   << " In addition, you might want to double check that the desired output encoding\n"
		   << " is correctly selected in Document > Settings > Language.\n"
		   << "\\end_layout\n";
		end_inset(os);
		is_nonCJKJapanese = false;
	}

#ifdef FILEDEBUG
		debugToken(cerr, t, flags);
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

		if (t.cat() != catEscape && t.character() == ']' &&
		    (flags & FLAG_BRACK_LAST))
			return;
		if (t.cat() == catEnd && (flags & FLAG_BRACE_LAST))
			return;

		// If there is anything between \end{env} and \begin{env} we
		// don't need to output a separator.
		if (t.cat() != catSpace && t.cat() != catNewline &&
		    t.asInput() != "\\begin")
			last_env = "";

		//
		// cat codes
		//
		bool const starred = p.next_token().asInput() == "*";
		string const starredname(starred ? (t.cs() + '*') : t.cs());
		if (t.cat() == catMath) {
			// we are inside some text mode thingy, so opening new math is allowed
			context.check_layout(os);
			begin_inset(os, "Formula ");
			Token const & n = p.get_token();
			bool const display(n.cat() == catMath && outer);
			if (display) {
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
			if (display) {
				// Prevent the conversion of a line break to a
				// space (bug 7668). This does not change the
				// output, but looks ugly in LyX.
				eat_whitespace(p, os, context, false);
			}
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

		else if (t.asInput() == "<"
			 && p.next_token().asInput() == "<") {
			bool has_chunk = false;
			if (noweb_mode) {
				p.pushPosition();
				p.get_token();
				has_chunk = parse_chunk(p, os, context);
				if (!has_chunk)
					p.popPosition();
			}

			if (!has_chunk) {
				context.check_layout(os);
				begin_inset(os, "Quotes ");
				//FIXME: this is a right danish quote;
				// why not a left french quote?
				os << "ard";
				end_inset(os);
				p.get_token();
				skip_braces(p);
			}
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
			output_ert_inset(os, s, context);
		}

		else if (t.cat() == catLetter) {
			context.check_layout(os);
			// Workaround for bug 4752.
			// FIXME: This whole code block needs to be removed
			//        when the bug is fixed and tex2lyx produces
			//        the updated file format.
			// The replacement algorithm in LyX is so stupid that
			// it even translates a phrase if it is part of a word.
			bool handled = false;
			for (int const * l = known_phrase_lengths; *l; ++l) {
				string phrase = t.cs();
				for (int i = 1; i < *l && p.next_token().isAlnumASCII(); ++i)
					phrase += p.get_token().cs();
				if (is_known(phrase, known_coded_phrases)) {
					output_ert_inset(os, phrase, context);
					handled = true;
					break;
				} else {
					for (size_t i = 1; i < phrase.length(); ++i)
						p.putback();
				}
			}
			if (!handled)
				os << t.cs();
		}

		else if (t.cat() == catOther ||
			       t.cat() == catAlign ||
			       t.cat() == catParameter) {
			// This translates "&" to "\\&" which may be wrong...
			context.check_layout(os);
			os << t.cs();
		}

		else if (p.isParagraph()) {
			if (context.new_layout_allowed)
				context.new_paragraph(os);
			else
				output_ert_inset(os, "\\par ", context);
			eat_whitespace(p, os, context, true);
		}

		else if (t.cat() == catActive) {
			context.check_layout(os);
			if (t.character() == '~') {
				if (context.layout->free_spacing)
					os << ' ';
				else {
					begin_inset(os, "space ~\n");
					end_inset(os);
				}
			} else
				os << t.cs();
		}

		else if (t.cat() == catBegin) {
			Token const next = p.next_token();
			Token const end = p.next_next_token();
			if (next.cat() == catEnd) {
				// {}
				Token const prev = p.prev_token();
				p.get_token();
				if (p.next_token().character() == '`' ||
				    (prev.character() == '-' &&
				     p.next_token().character() == '-'))
					; // ignore it in {}`` or -{}-
				else
					output_ert_inset(os, "{}", context);
			} else if (next.cat() == catEscape &&
			           is_known(next.cs(), known_quotes) &&
			           end.cat() == catEnd) {
				// Something like {\textquoteright} (e.g.
				// from writer2latex). LyX writes
				// \textquoteright{}, so we may skip the
				// braces here for better readability.
				parse_text_snippet(p, os, FLAG_BRACE_LAST,
				                   outer, context);
			} else if (p.next_token().asInput() == "\\ascii") {
				// handle the \ascii characters
				// (the case without braces is handled later)
				// the code is "{\ascii\xxx}"
				p.get_token(); // eat \ascii
				string name2 = p.get_token().asInput();
				p.get_token(); // eat the final '}'
				string const name = "{\\ascii" + name2 + "}";
				bool termination;
				docstring rem;
				set<string> req;
				// get the character from unicodesymbols
				docstring s = encodings.fromLaTeXCommand(from_utf8(name),
					Encodings::TEXT_CMD, termination, rem, &req);
				if (!s.empty()) {
					context.check_layout(os);
					os << to_utf8(s);
					if (!rem.empty())
						output_ert_inset(os,
							to_utf8(rem), context);
					for (set<string>::const_iterator it = req.begin();
					     it != req.end(); ++it)
						preamble.registerAutomaticallyLoadedPackage(*it);
				} else
					// we did not find a non-ert version
					output_ert_inset(os, name, context);
			} else {
			context.check_layout(os);
			// special handling of font attribute changes
			Token const prev = p.prev_token();
			TeXFont const oldFont = context.font;
			if (next.character() == '[' ||
			    next.character() == ']' ||
			    next.character() == '*') {
				p.get_token();
				if (p.next_token().cat() == catEnd) {
					os << next.cs();
					p.get_token();
				} else {
					p.putback();
					output_ert_inset(os, "{", context);
					parse_text_snippet(p, os,
							FLAG_BRACE_LAST,
							outer, context);
					output_ert_inset(os, "}", context);
				}
			} else if (! context.new_layout_allowed) {
				output_ert_inset(os, "{", context);
				parse_text_snippet(p, os, FLAG_BRACE_LAST,
						   outer, context);
				output_ert_inset(os, "}", context);
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
				output_ert_inset(os, "{", context);
				parse_text_snippet(p, os, FLAG_BRACE_LAST,
						   outer, context);
				output_ert_inset(os, "}", context);
				}
			}
		}

		else if (t.cat() == catEnd) {
			if (flags & FLAG_BRACE_LAST) {
				return;
			}
			cerr << "stray '}' in text\n";
			output_ert_inset(os, "}", context);
		}

		else if (t.cat() == catComment)
			parse_comment(p, os, t, context);

		//
		// control sequences
		//

		else if (t.cs() == "(" || t.cs() == "[") {
			bool const simple = t.cs() == "(";
			context.check_layout(os);
			begin_inset(os, "Formula");
			os << " \\" << t.cs();
			parse_math(p, os, simple ? FLAG_SIMPLE2 : FLAG_EQUATION, MATH_MODE);
			os << '\\' << (simple ? ')' : ']');
			end_inset(os);
			if (!simple) {
				// Prevent the conversion of a line break to a
				// space (bug 7668). This does not change the
				// output, but looks ugly in LyX.
				eat_whitespace(p, os, context, false);
			}
		}

		else if (t.cs() == "begin")
			parse_environment(p, os, outer, last_env,
			                  context);

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
			string s;
			bool const optarg = p.hasOpt();
			if (optarg) {
				// FIXME: This swallows comments, but we cannot use
				//        eat_whitespace() since we must not output
				//        anything before the item.
				p.skip_spaces(true);
				s = p.verbatimOption();
			} else
				p.skip_spaces(false);
			context.set_item();
			context.check_layout(os);
			if (context.has_item) {
				// An item in an unknown list-like environment
				// FIXME: Do this in check_layout()!
				context.has_item = false;
				if (optarg)
					output_ert_inset(os, "\\item", context);
				else
					output_ert_inset(os, "\\item ", context);
			}
			if (optarg) {
				if (context.layout->labeltype != LABEL_MANUAL) {
					// handle option of itemize item
					begin_inset(os, "Argument item:1\n");
					os << "status open\n";
					os << "\n\\begin_layout Plain Layout\n";
					Parser p2(s + ']');
					os << parse_text_snippet(p2,
						FLAG_BRACK_LAST, outer, context);
					// we must not use context.check_end_layout(os)
					// because that would close the outer itemize layout
					os << "\n\\end_layout\n";
					end_inset(os);
					eat_whitespace(p, os, context, false);
				} else if (!s.empty()) {
					// LyX adds braces around the argument,
					// so we need to remove them here.
					if (s.size() > 2 && s[0] == '{' &&
					    s[s.size()-1] == '}')
						s = s.substr(1, s.size()-2);
					// If the argument contains a space we
					// must put it into ERT: Otherwise LyX
					// would misinterpret the space as
					// item delimiter (bug 7663)
					if (contains(s, ' ')) {
						output_ert_inset(os, s, context);
					} else {
						Parser p2(s + ']');
						os << parse_text_snippet(p2,
							FLAG_BRACK_LAST, outer, context);
					}
					// The space is needed to separate the
					// item from the rest of the sentence.
					os << ' ';
					eat_whitespace(p, os, context, false);
				}
			}
		}

		else if (t.cs() == "bibitem") {
			context.set_item();
			context.check_layout(os);
			eat_whitespace(p, os, context, false);
			string label = convert_command_inset_arg(p.verbatimOption());
			string key = convert_command_inset_arg(p.verbatim_item());
			if (contains(label, '\\') || contains(key, '\\')) {
				// LyX can't handle LaTeX commands in labels or keys
				output_ert_inset(os, t.asInput() + '[' + label +
				               "]{" + p.verbatim_item() + '}',
				           context);
			} else {
				begin_command_inset(os, "bibitem", "bibitem");
				os << "label \"" << label << "\"\n"
				      "key \"" << key << "\"\n";
				end_inset(os);
			}
		}

		else if (is_macro(p)) {
			// catch the case of \def\inputGnumericTable
			bool macro = true;
			if (t.cs() == "def") {
				Token second = p.next_token();
				if (second.cs() == "inputGnumericTable") {
					p.pushPosition();
					p.get_token();
					skip_braces(p);
					Token third = p.get_token();
					p.popPosition();
					if (third.cs() == "input") {
						p.get_token();
						skip_braces(p);
						p.get_token();
						string name = normalize_filename(p.verbatim_item());
						string const path = getMasterFilePath(true);
						// We want to preserve relative / absolute filenames,
						// therefore path is only used for testing
						// The file extension is in every case ".tex".
						// So we need to remove this extension and check for
						// the original one.
						name = removeExtension(name);
						if (!makeAbsPath(name, path).exists()) {
							char const * const Gnumeric_formats[] = {"gnumeric",
								"ods", "xls", 0};
							string const Gnumeric_name =
								find_file(name, path, Gnumeric_formats);
							if (!Gnumeric_name.empty())
								name = Gnumeric_name;
						}
						FileName const absname = makeAbsPath(name, path);
						if (absname.exists()) {
							fix_child_filename(name);
							copy_file(absname, name);
						} else
							cerr << "Warning: Could not find file '"
							     << name << "'." << endl;
						context.check_layout(os);
						begin_inset(os, "External\n\ttemplate ");
						os << "GnumericSpreadsheet\n\tfilename "
						   << name << "\n";
						end_inset(os);
						context.check_layout(os);
						macro = false;
						// register the packages that are automatically loaded
						// by the Gnumeric template
						registerExternalTemplatePackages("GnumericSpreadsheet");
					}
				}
			}
			if (macro)
				parse_macro(p, os, context);
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
			output_ert_inset(os,
				"%dummy comment inserted by tex2lyx to "
				"ensure that this paragraph is not empty",
				context);
			// Both measures above may generate an additional
			// empty paragraph, but that does not hurt, because
			// whitespace does not matter here.
			eat_whitespace(p, os, context, true);
		}

		// Must catch empty dates before findLayout is called below
		else if (t.cs() == "date") {
			eat_whitespace(p, os, context, false);
			p.pushPosition();
			string const date = p.verbatim_item();
			p.popPosition();
			if (date.empty()) {
				preamble.suppressDate(true);
				p.verbatim_item();
			} else {
				preamble.suppressDate(false);
				if (context.new_layout_allowed &&
				    (newlayout = findLayout(context.textclass,
				                            t.cs(), true))) {
					// write the layout
					output_command_layout(os, p, outer,
							context, newlayout);
					parse_text_snippet(p, os, FLAG_ITEM, outer, context);
					if (!preamble.titleLayoutFound())
						preamble.titleLayoutFound(newlayout->intitle);
					set<string> const & req = newlayout->requires();
					set<string>::const_iterator it = req.begin();
					set<string>::const_iterator en = req.end();
					for (; it != en; ++it)
						preamble.registerAutomaticallyLoadedPackage(*it);
				} else
					output_ert_inset(os,
						"\\date{" + p.verbatim_item() + '}',
						context);
			}
		}

		// Starred section headings
		// Must attempt to parse "Section*" before "Section".
		else if ((p.next_token().asInput() == "*") &&
			 context.new_layout_allowed &&
			 (newlayout = findLayout(context.textclass, t.cs() + '*', true))) {
			// write the layout
			p.get_token();
			output_command_layout(os, p, outer, context, newlayout);
			p.skip_spaces();
			if (!preamble.titleLayoutFound())
				preamble.titleLayoutFound(newlayout->intitle);
			set<string> const & req = newlayout->requires();
			for (set<string>::const_iterator it = req.begin(); it != req.end(); ++it)
				preamble.registerAutomaticallyLoadedPackage(*it);
		}

		// Section headings and the like
		else if (context.new_layout_allowed &&
			 (newlayout = findLayout(context.textclass, t.cs(), true))) {
			// write the layout
			output_command_layout(os, p, outer, context, newlayout);
			p.skip_spaces();
			if (!preamble.titleLayoutFound())
				preamble.titleLayoutFound(newlayout->intitle);
			set<string> const & req = newlayout->requires();
			for (set<string>::const_iterator it = req.begin(); it != req.end(); ++it)
				preamble.registerAutomaticallyLoadedPackage(*it);
		}

		else if (t.cs() == "subfloat") {
			// the syntax is \subfloat[caption]{content}
			// if it is a table of figure depends on the surrounding float
			bool has_caption = false;
			p.skip_spaces();
			// do nothing if there is no outer float
			if (!float_type.empty()) {
				context.check_layout(os);
				p.skip_spaces();
				begin_inset(os, "Float " + float_type + "\n");
				os << "wide false"
				   << "\nsideways false"
				   << "\nstatus collapsed\n\n";
				// test for caption
				string caption;
				if (p.next_token().cat() != catEscape &&
						p.next_token().character() == '[') {
							p.get_token(); // eat '['
							caption = parse_text_snippet(p, FLAG_BRACK_LAST, outer, context);
							has_caption = true;
				}
				// the content
				parse_text_in_inset(p, os, FLAG_ITEM, outer, context);
				// the caption comes always as the last
				if (has_caption) {
					// we must make sure that the caption gets a \begin_layout
					os << "\n\\begin_layout Plain Layout";
					p.skip_spaces();
					begin_inset(os, "Caption Standard\n");
					Context newcontext(true, context.textclass,
					                   0, 0, context.font);
					newcontext.check_layout(os);
					os << caption << "\n";
					newcontext.check_end_layout(os);
					// We don't need really a new paragraph, but
					// we must make sure that the next item gets a \begin_layout.
					//newcontext.new_paragraph(os);
					end_inset(os);
					p.skip_spaces();
				}
				// We don't need really a new paragraph, but
				// we must make sure that the next item gets a \begin_layout.
				if (has_caption)
					context.new_paragraph(os);
				end_inset(os);
				p.skip_spaces();
				context.check_end_layout(os);
				// close the layout we opened
				if (has_caption)
					os << "\n\\end_layout\n";
			} else {
				// if the float type is not supported or there is no surrounding float
				// output it as ERT
				if (p.hasOpt()) {
					string opt_arg = convert_command_inset_arg(p.getArg('[', ']'));
					output_ert_inset(os, t.asInput() + '[' + opt_arg +
				               "]{" + p.verbatim_item() + '}', context);
				} else
					output_ert_inset(os, t.asInput() + "{" + p.verbatim_item() + '}', context);
			}
		}

		else if (t.cs() == "includegraphics") {
			bool const clip = p.next_token().asInput() == "*";
			if (clip)
				p.get_token();
			string const arg = p.getArg('[', ']');
			map<string, string> opts;
			vector<string> keys;
			split_map(arg, opts, keys);
			if (clip)
				opts["clip"] = string();
			string name = normalize_filename(p.verbatim_item());

			string const path = getMasterFilePath(true);
			// We want to preserve relative / absolute filenames,
			// therefore path is only used for testing
			if (!makeAbsPath(name, path).exists()) {
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
				} else if (!pdftex_name.empty()) {
					name = pdftex_name;
					pdflatex = true;
				}
			}

			FileName const absname = makeAbsPath(name, path);
			if (absname.exists()) {
				fix_child_filename(name);
				copy_file(absname, name);
			} else
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
			if (opts.find("angle") != opts.end()) {
				os << "\trotateAngle "
				   << opts["angle"] << '\n';
				vector<string>::const_iterator a =
					find(keys.begin(), keys.end(), "angle");
				vector<string>::const_iterator s =
					find(keys.begin(), keys.end(), "width");
				if (s == keys.end())
					s = find(keys.begin(), keys.end(), "height");
				if (s == keys.end())
					s = find(keys.begin(), keys.end(), "scale");
				if (s != keys.end() && distance(s, a) > 0)
					os << "\tscaleBeforeRotation\n";
			}
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
			preamble.registerAutomaticallyLoadedPackage("graphicx");
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

		else if (t.cs() == "lstinline") {
			p.skip_spaces();
			parse_listings(p, os, context, true);
		}

		else if (t.cs() == "ensuremath") {
			p.skip_spaces();
			context.check_layout(os);
			string const s = p.verbatim_item();
			//FIXME: this never triggers in UTF8
			if (s == "\xb1" || s == "\xb3" || s == "\xb2" || s == "\xb5")
				os << s;
			else
				output_ert_inset(os, "\\ensuremath{" + s + "}",
					   context);
		}

		else if (t.cs() == "makeindex" || t.cs() == "maketitle") {
			if (preamble.titleLayoutFound()) {
				// swallow this
				skip_spaces_braces(p);
			} else
				output_ert_inset(os, t.asInput(), context);
		}

		else if (t.cs() == "tableofcontents" || t.cs() == "lstlistoflistings") {
			context.check_layout(os);
			begin_command_inset(os, "toc", t.cs());
			end_inset(os);
			skip_spaces_braces(p);
			if (t.cs() == "lstlistoflistings")
				preamble.registerAutomaticallyLoadedPackage("listings");
		}

		else if (t.cs() == "listoffigures" || t.cs() == "listoftables") {
			context.check_layout(os);
			if (t.cs() == "listoffigures")
				begin_inset(os, "FloatList figure\n");
			else
				begin_inset(os, "FloatList table\n");
			end_inset(os);
			skip_spaces_braces(p);
		}

		else if (t.cs() == "listof") {
			p.skip_spaces(true);
			string const name = p.get_token().cs();
			if (context.textclass.floats().typeExist(name)) {
				context.check_layout(os);
				begin_inset(os, "FloatList ");
				os << name << "\n";
				end_inset(os);
				p.get_token(); // swallow second arg
			} else
				output_ert_inset(os, "\\listof{" + name + "}", context);
		}

		else if ((where = is_known(t.cs(), known_text_font_families)))
			parse_text_attributes(p, os, FLAG_ITEM, outer,
				context, "\\family", context.font.family,
				known_coded_font_families[where - known_text_font_families]);

		else if ((where = is_known(t.cs(), known_text_font_series)))
			parse_text_attributes(p, os, FLAG_ITEM, outer,
				context, "\\series", context.font.series,
				known_coded_font_series[where - known_text_font_series]);

		else if ((where = is_known(t.cs(), known_text_font_shapes)))
			parse_text_attributes(p, os, FLAG_ITEM, outer,
				context, "\\shape", context.font.shape,
				known_coded_font_shapes[where - known_text_font_shapes]);

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
					preamble.registerAutomaticallyLoadedPackage("color");
			} else
				// for custom defined colors
				output_ert_inset(os, t.asInput() + "{" + color + "}", context);
		}

		else if (t.cs() == "underbar" || t.cs() == "uline") {
			// \underbar is not 100% correct (LyX outputs \uline
			// of ulem.sty). The difference is that \ulem allows
			// line breaks, and \underbar does not.
			// Do NOT handle \underline.
			// \underbar cuts through y, g, q, p etc.,
			// \underline does not.
			context.check_layout(os);
			os << "\n\\bar under\n";
			parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			context.check_layout(os);
			os << "\n\\bar default\n";
			preamble.registerAutomaticallyLoadedPackage("ulem");
		}

		else if (t.cs() == "sout") {
			context.check_layout(os);
			os << "\n\\strikeout on\n";
			parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			context.check_layout(os);
			os << "\n\\strikeout default\n";
			preamble.registerAutomaticallyLoadedPackage("ulem");
		}

		else if (t.cs() == "uuline" || t.cs() == "uwave" ||
		         t.cs() == "emph" || t.cs() == "noun") {
			context.check_layout(os);
			os << "\n\\" << t.cs() << " on\n";
			parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			context.check_layout(os);
			os << "\n\\" << t.cs() << " default\n";
			if (t.cs() == "uuline" || t.cs() == "uwave")
				preamble.registerAutomaticallyLoadedPackage("ulem");
		}

		else if (t.cs() == "lyxadded" || t.cs() == "lyxdeleted") {
			context.check_layout(os);
			string name = p.getArg('{', '}');
			string localtime = p.getArg('{', '}');
			preamble.registerAuthor(name);
			Author const & author = preamble.getAuthor(name);
			// from_asctime_utc() will fail if LyX decides to output the
			// time in the text language.
			time_t ptime = from_asctime_utc(localtime);
			if (ptime == static_cast<time_t>(-1)) {
				cerr << "Warning: Could not parse time `" << localtime
				     << "´ for change tracking, using current time instead.\n";
				ptime = current_time();
			}
			if (t.cs() == "lyxadded")
				os << "\n\\change_inserted ";
			else
				os << "\n\\change_deleted ";
			os << author.bufferId() << ' ' << ptime << '\n';
			parse_text_snippet(p, os, FLAG_ITEM, outer, context);
			bool dvipost    = LaTeXPackages::isAvailable("dvipost");
			bool xcolorulem = LaTeXPackages::isAvailable("ulem") &&
			                  LaTeXPackages::isAvailable("xcolor");
			// No need to test for luatex, since luatex comes in
			// two flavours (dvi and pdf), like latex, and those
			// are detected by pdflatex.
			if (pdflatex || xetex) {
				if (xcolorulem) {
					preamble.registerAutomaticallyLoadedPackage("ulem");
					preamble.registerAutomaticallyLoadedPackage("xcolor");
					preamble.registerAutomaticallyLoadedPackage("pdfcolmk");
				}
			} else {
				if (dvipost) {
					preamble.registerAutomaticallyLoadedPackage("dvipost");
				} else if (xcolorulem) {
					preamble.registerAutomaticallyLoadedPackage("ulem");
					preamble.registerAutomaticallyLoadedPackage("xcolor");
				}
			}
		}

		else if (t.cs() == "textipa") {
			context.check_layout(os);
			begin_inset(os, "IPA\n");
			parse_text_in_inset(p, os, FLAG_ITEM, outer, context);
			end_inset(os);
			preamble.registerAutomaticallyLoadedPackage("tipa");
			preamble.registerAutomaticallyLoadedPackage("tipx");
		}

		else if (t.cs() == "texttoptiebar" || t.cs() == "textbottomtiebar") {
			context.check_layout(os);
			begin_inset(os, "IPADeco " + t.cs().substr(4) + "\n");
			os << "status open\n";
			parse_text_in_inset(p, os, FLAG_ITEM, outer, context);
			end_inset(os);
			p.skip_spaces();
		}

		else if (t.cs() == "textvertline") {
			// FIXME: This is not correct, \textvertline is higher than |
			os << "|";
			skip_braces(p);
			continue;
		}

		else if (t.cs() == "tone" ) {
			context.check_layout(os);
			// register the tone package
			preamble.registerAutomaticallyLoadedPackage("tone");
			string content = trimSpaceAndEol(p.verbatim_item());
			string command = t.asInput() + "{" + content + "}";
			// some tones can be detected by unicodesymbols, some need special code
			if (is_known(content, known_tones)) {
				os << "\\IPAChar " << command << "\n";
				continue;
			}
			// try to see whether the string is in unicodesymbols
			bool termination;
			docstring rem;
			set<string> req;
			docstring s = encodings.fromLaTeXCommand(from_utf8(command),
				Encodings::TEXT_CMD | Encodings::MATH_CMD,
				termination, rem, &req);
			if (!s.empty()) {
				os << to_utf8(s);
				if (!rem.empty())
					output_ert_inset(os, to_utf8(rem), context);
				for (set<string>::const_iterator it = req.begin();
				     it != req.end(); ++it)
					preamble.registerAutomaticallyLoadedPackage(*it);
			} else
				// we did not find a non-ert version
				output_ert_inset(os, command, context);
		}

		else if (t.cs() == "phantom" || t.cs() == "hphantom" ||
			     t.cs() == "vphantom") {
			context.check_layout(os);
			if (t.cs() == "phantom")
				begin_inset(os, "Phantom Phantom\n");
			if (t.cs() == "hphantom")
				begin_inset(os, "Phantom HPhantom\n");
			if (t.cs() == "vphantom")
				begin_inset(os, "Phantom VPhantom\n");
			os << "status open\n";
			parse_text_in_inset(p, os, FLAG_ITEM, outer, context,
			                    "Phantom");
			end_inset(os);
		}

		else if (t.cs() == "href") {
			context.check_layout(os);
			string target = convert_command_inset_arg(p.verbatim_item());
			string name = convert_command_inset_arg(p.verbatim_item());
			string type;
			size_t i = target.find(':');
			if (i != string::npos) {
				type = target.substr(0, i + 1);
				if (type == "mailto:" || type == "file:")
					target = target.substr(i + 1);
				// handle the case that name is equal to target, except of "http://"
				else if (target.substr(i + 3) == name && type == "http:")
					target = name;
			}
			begin_command_inset(os, "href", "href");
			if (name != target)
				os << "name \"" << name << "\"\n";
			os << "target \"" << target << "\"\n";
			if (type == "mailto:" || type == "file:")
				os << "type \"" << type << "\"\n";
			end_inset(os);
			skip_spaces_braces(p);
		}

		else if (t.cs() == "lyxline") {
			// swallow size argument (it is not used anyway)
			p.getArg('{', '}');
			if (!context.atParagraphStart()) {
				// so our line is in the middle of a paragraph
				// we need to add a new line, lest this line
				// follow the other content on that line and
				// run off the side of the page
				// FIXME: This may create an empty paragraph,
				//        but without that it would not be
				//        possible to set noindent below.
				//        Fortunately LaTeX does not care
				//        about the empty paragraph.
				context.new_paragraph(os);
			}
			if (preamble.indentParagraphs()) {
				// we need to unindent, lest the line be too long
				context.add_par_extra_stuff("\\noindent\n");
			}
			context.check_layout(os);
			begin_command_inset(os, "line", "rule");
			os << "offset \"0.5ex\"\n"
			      "width \"100line%\"\n"
			      "height \"1pt\"\n";
			end_inset(os);
		}

		else if (t.cs() == "rule") {
			string const offset = (p.hasOpt() ? p.getArg('[', ']') : string());
			string const width = p.getArg('{', '}');
			string const thickness = p.getArg('{', '}');
			context.check_layout(os);
			begin_command_inset(os, "line", "rule");
			if (!offset.empty())
				os << "offset \"" << translate_len(offset) << "\"\n";
			os << "width \"" << translate_len(width) << "\"\n"
				  "height \"" << translate_len(thickness) << "\"\n";
			end_inset(os);
		}

		else if (is_known(t.cs(), known_phrases) ||
		         (t.cs() == "protect" &&
		          p.next_token().cat() == catEscape &&
		          is_known(p.next_token().cs(), known_phrases))) {
			// LyX sometimes puts a \protect in front, so we have to ignore it
			// FIXME: This needs to be changed when bug 4752 is fixed.
			where = is_known(
				t.cs() == "protect" ? p.get_token().cs() : t.cs(),
				known_phrases);
			context.check_layout(os);
			os << known_coded_phrases[where - known_phrases];
			skip_spaces_braces(p);
		}

		// handle refstyle first to catch \eqref which can also occur
		// without refstyle. Only recognize these commands if
		// refstyle.sty was found in the preamble (otherwise \eqref
		// and user defined ref commands could be misdetected).
		else if ((where = is_known(t.cs(), known_refstyle_commands)) &&
		         preamble.refstyle()) {
			context.check_layout(os);
			begin_command_inset(os, "ref", "formatted");
			os << "reference \"";
			os << known_refstyle_prefixes[where - known_refstyle_commands]
			   << ":";
			os << convert_command_inset_arg(p.verbatim_item())
			   << "\"\n";
			end_inset(os);
			preamble.registerAutomaticallyLoadedPackage("refstyle");
		}

		// if refstyle is used, we must not convert \prettyref to a
		// formatted reference, since that would result in a refstyle command.
		else if ((where = is_known(t.cs(), known_ref_commands)) &&
		         (t.cs() != "prettyref" || !preamble.refstyle())) {
			string const opt = p.getOpt();
			if (opt.empty()) {
				context.check_layout(os);
				begin_command_inset(os, "ref",
					known_coded_ref_commands[where - known_ref_commands]);
				os << "reference \""
				   << convert_command_inset_arg(p.verbatim_item())
				   << "\"\n";
				end_inset(os);
				if (t.cs() == "vref" || t.cs() == "vpageref")
					preamble.registerAutomaticallyLoadedPackage("varioref");
				else if (t.cs() == "prettyref")
					preamble.registerAutomaticallyLoadedPackage("prettyref");
			} else {
				// LyX does not yet support optional arguments of ref commands
				output_ert_inset(os, t.asInput() + '[' + opt + "]{" +
			               p.verbatim_item() + '}', context);
			}
		}

		else if (use_natbib &&
			 is_known(t.cs(), known_natbib_commands) &&
			 ((t.cs() != "citefullauthor" &&
			   t.cs() != "citeyear" &&
			   t.cs() != "citeyearpar") ||
			  p.next_token().asInput() != "*")) {
			context.check_layout(os);
			string command = t.cs();
			if (p.next_token().asInput() == "*") {
				command += '*';
				p.get_token();
			}
			if (command == "citefullauthor")
				// alternative name for "\\citeauthor*"
				command = "citeauthor*";

			// text before the citation
			string before;
			// text after the citation
			string after;
			get_cite_arguments(p, true, before, after);

			if (command == "cite") {
				// \cite without optional argument means
				// \citet, \cite with at least one optional
				// argument means \citep.
				if (before.empty() && after.empty())
					command = "citet";
				else
					command = "citep";
			}
			if (before.empty() && after == "[]")
				// avoid \citet[]{a}
				after.erase();
			else if (before == "[]" && after == "[]") {
				// avoid \citet[][]{a}
				before.erase();
				after.erase();
			}
			// remove the brackets around after and before
			if (!after.empty()) {
				after.erase(0, 1);
				after.erase(after.length() - 1, 1);
				after = convert_command_inset_arg(after);
			}
			if (!before.empty()) {
				before.erase(0, 1);
				before.erase(before.length() - 1, 1);
				before = convert_command_inset_arg(before);
			}
			begin_command_inset(os, "citation", command);
			os << "after " << '"' << after << '"' << "\n";
			os << "before " << '"' << before << '"' << "\n";
			os << "key \""
			   << convert_command_inset_arg(p.verbatim_item())
			   << "\"\n";
			end_inset(os);
			// Need to set the cite engine if natbib is loaded by
			// the document class directly
			if (preamble.citeEngine() == "basic")
				preamble.citeEngine("natbib");
		}

		else if (use_jurabib &&
			 is_known(t.cs(), known_jurabib_commands) &&
		         (t.cs() == "cite" || p.next_token().asInput() != "*")) {
			context.check_layout(os);
			string command = t.cs();
			if (p.next_token().asInput() == "*") {
				command += '*';
				p.get_token();
			}
			char argumentOrder = '\0';
			vector<string> const options =
				preamble.getPackageOptions("jurabib");
			if (find(options.begin(), options.end(),
				      "natbiborder") != options.end())
				argumentOrder = 'n';
			else if (find(options.begin(), options.end(),
					   "jurabiborder") != options.end())
				argumentOrder = 'j';

			// text before the citation
			string before;
			// text after the citation
			string after;
			get_cite_arguments(p, argumentOrder != 'j', before, after);

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
			if (!after.empty()) {
				after.erase(0, 1);
				after.erase(after.length() - 1, 1);
			}
			if (!before.empty()) {
				before.erase(0, 1);
				before.erase(before.length() - 1, 1);
			}
			begin_command_inset(os, "citation", command);
			os << "after " << '"' << after << '"' << "\n";
			os << "before " << '"' << before << '"' << "\n";
			os << "key " << '"' << citation << '"' << "\n";
			end_inset(os);
			// Need to set the cite engine if jurabib is loaded by
			// the document class directly
			if (preamble.citeEngine() == "basic")
				preamble.citeEngine("jurabib");
		}

		else if (t.cs() == "cite"
			|| t.cs() == "nocite") {
			context.check_layout(os);
			string after = convert_command_inset_arg(p.getArg('[', ']'));
			string key = convert_command_inset_arg(p.verbatim_item());
			// store the case that it is "\nocite{*}" to use it later for
			// the BibTeX inset
			if (key != "*") {
				begin_command_inset(os, "citation", t.cs());
				os << "after " << '"' << after << '"' << "\n";
				os << "key " << '"' << key << '"' << "\n";
				end_inset(os);
			} else if (t.cs() == "nocite")
				btprint = key;
		}

		else if (t.cs() == "index" ||
		         (t.cs() == "sindex" && preamble.use_indices() == "true")) {
			context.check_layout(os);
			string const arg = (t.cs() == "sindex" && p.hasOpt()) ?
				p.getArg('[', ']') : "";
			string const kind = arg.empty() ? "idx" : arg;
			begin_inset(os, "Index ");
			os << kind << "\nstatus collapsed\n";
			parse_text_in_inset(p, os, FLAG_ITEM, false, context, "Index");
			end_inset(os);
			if (kind != "idx")
				preamble.registerAutomaticallyLoadedPackage("splitidx");
		}

		else if (t.cs() == "nomenclature") {
			context.check_layout(os);
			begin_command_inset(os, "nomenclature", "nomenclature");
			string prefix = convert_command_inset_arg(p.getArg('[', ']'));
			if (!prefix.empty())
				os << "prefix " << '"' << prefix << '"' << "\n";
			os << "symbol " << '"'
			   << convert_command_inset_arg(p.verbatim_item());
			os << "\"\ndescription \""
			   << convert_command_inset_arg(p.verbatim_item())
			   << "\"\n";
			end_inset(os);
			preamble.registerAutomaticallyLoadedPackage("nomencl");
		}

		else if (t.cs() == "label") {
			context.check_layout(os);
			begin_command_inset(os, "label", "label");
			os << "name \""
			   << convert_command_inset_arg(p.verbatim_item())
			   << "\"\n";
			end_inset(os);
		}

		else if (t.cs() == "printindex" || t.cs() == "printsubindex") {
			context.check_layout(os);
			string commandname = t.cs();
			bool star = false;
			if (p.next_token().asInput() == "*") {
				commandname += "*";
				star = true;
				p.get_token();
			}
			begin_command_inset(os, "index_print", commandname);
			string const indexname = p.getArg('[', ']');
			if (!star) {
				if (indexname.empty())
					os << "type \"idx\"\n";
				else
					os << "type \"" << indexname << "\"\n";
			}
			end_inset(os);
			skip_spaces_braces(p);
			preamble.registerAutomaticallyLoadedPackage("makeidx");
			if (preamble.use_indices() == "true")
				preamble.registerAutomaticallyLoadedPackage("splitidx");
		}

		else if (t.cs() == "printnomenclature") {
			string width = "";
			string width_type = "";
			context.check_layout(os);
			begin_command_inset(os, "nomencl_print", "printnomenclature");
			// case of a custom width
			if (p.hasOpt()) {
				width = p.getArg('[', ']');
				width = translate_len(width);
				width_type = "custom";
			}
			// case of no custom width
			// the case of no custom width but the width set
			// via \settowidth{\nomlabelwidth}{***} cannot be supported
			// because the user could have set anything, not only the width
			// of the longest label (which would be width_type = "auto")
			string label = convert_command_inset_arg(p.getArg('{', '}'));
			if (label.empty() && width_type.empty())
				width_type = "none";
			os << "set_width \"" << width_type << "\"\n";
			if (width_type == "custom")
				os << "width \"" << width << '\"';
			end_inset(os);
			skip_spaces_braces(p);
			preamble.registerAutomaticallyLoadedPackage("nomencl");
		}

		else if ((t.cs() == "textsuperscript" || t.cs() == "textsubscript")) {
			context.check_layout(os);
			begin_inset(os, "script ");
			os << t.cs().substr(4) << '\n';
			parse_text_in_inset(p, os, FLAG_ITEM, false, context);
			end_inset(os);
			if (t.cs() == "textsubscript")
				preamble.registerAutomaticallyLoadedPackage("subscript");
		}

		else if ((where = is_known(t.cs(), known_quotes))) {
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

		else if ((where = is_known(t.cs(), known_sizes)) &&
			 context.new_layout_allowed) {
			context.check_layout(os);
			TeXFont const oldFont = context.font;
			context.font.size = known_coded_sizes[where - known_sizes];
			output_font_change(os, oldFont, context.font);
			eat_whitespace(p, os, context, false);
		}

		else if ((where = is_known(t.cs(), known_font_families)) &&
			 context.new_layout_allowed) {
			context.check_layout(os);
			TeXFont const oldFont = context.font;
			context.font.family =
				known_coded_font_families[where - known_font_families];
			output_font_change(os, oldFont, context.font);
			eat_whitespace(p, os, context, false);
		}

		else if ((where = is_known(t.cs(), known_font_series)) &&
			 context.new_layout_allowed) {
			context.check_layout(os);
			TeXFont const oldFont = context.font;
			context.font.series =
				known_coded_font_series[where - known_font_series];
			output_font_change(os, oldFont, context.font);
			eat_whitespace(p, os, context, false);
		}

		else if ((where = is_known(t.cs(), known_font_shapes)) &&
			 context.new_layout_allowed) {
			context.check_layout(os);
			TeXFont const oldFont = context.font;
			context.font.shape =
				known_coded_font_shapes[where - known_font_shapes];
			output_font_change(os, oldFont, context.font);
			eat_whitespace(p, os, context, false);
		}
		else if ((where = is_known(t.cs(), known_old_font_families)) &&
			 context.new_layout_allowed) {
			context.check_layout(os);
			TeXFont const oldFont = context.font;
			context.font.init();
			context.font.size = oldFont.size;
			context.font.family =
				known_coded_font_families[where - known_old_font_families];
			output_font_change(os, oldFont, context.font);
			eat_whitespace(p, os, context, false);
		}

		else if ((where = is_known(t.cs(), known_old_font_series)) &&
			 context.new_layout_allowed) {
			context.check_layout(os);
			TeXFont const oldFont = context.font;
			context.font.init();
			context.font.size = oldFont.size;
			context.font.series =
				known_coded_font_series[where - known_old_font_series];
			output_font_change(os, oldFont, context.font);
			eat_whitespace(p, os, context, false);
		}

		else if ((where = is_known(t.cs(), known_old_font_shapes)) &&
			 context.new_layout_allowed) {
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
			// save the language for the case that a
			// \foreignlanguage is used
			context.font.language = babel2lyx(p.verbatim_item());
			os << "\n\\lang " << context.font.language << "\n";
		}

		else if (t.cs() == "foreignlanguage") {
			string const lang = babel2lyx(p.verbatim_item());
			parse_text_attributes(p, os, FLAG_ITEM, outer,
			                      context, "\\lang",
			                      context.font.language, lang);
		}

		else if (prefixIs(t.cs(), "text") && preamble.usePolyglossia()
			 && is_known(t.cs().substr(4), preamble.polyglossia_languages)) {
			// scheme is \textLANGUAGE{text} where LANGUAGE is in polyglossia_languages[]
			string lang;
			// We have to output the whole command if it has an option
			// because LyX doesn't support this yet, see bug #8214,
			// only if there is a single option specifying a variant, we can handle it.
			if (p.hasOpt()) {
				string langopts = p.getOpt();
				// check if the option contains a variant, if yes, extract it
				string::size_type pos_var = langopts.find("variant");
				string::size_type i = langopts.find(',');
				string::size_type k = langopts.find('=', pos_var);
				if (pos_var != string::npos && i == string::npos) {
					string variant;
					variant = langopts.substr(k + 1, langopts.length() - k - 2);
					lang = preamble.polyglossia2lyx(variant);
					parse_text_attributes(p, os, FLAG_ITEM, outer,
						                  context, "\\lang",
						                  context.font.language, lang);
				} else
					output_ert_inset(os, t.asInput() + langopts, context);
			} else {
				lang = preamble.polyglossia2lyx(t.cs().substr(4, string::npos));
				parse_text_attributes(p, os, FLAG_ITEM, outer,
					                  context, "\\lang",
					                  context.font.language, lang);
			}
		}

		else if (t.cs() == "inputencoding") {
			// nothing to write here
			string const enc = subst(p.verbatim_item(), "\n", " ");
			p.setEncoding(enc, Encoding::inputenc);
		}

		else if ((where = is_known(t.cs(), known_special_chars))) {
			context.check_layout(os);
			os << known_coded_special_chars[where - known_special_chars];
			skip_spaces_braces(p);
		}

		else if ((t.cs() == "nobreakdash" && p.next_token().asInput() == "-") ||
		         (t.cs() == "@" && p.next_token().asInput() == ".")) {
			context.check_layout(os);
			os << "\\SpecialChar \\" << t.cs()
			   << p.get_token().asInput() << '\n';
		}

		else if (t.cs() == "textquotedbl") {
			context.check_layout(os);
			os << "\"";
			skip_braces(p);
		}

		else if (t.cs() == "_" || t.cs() == "&" || t.cs() == "#"
			    || t.cs() == "$" || t.cs() == "{" || t.cs() == "}"
			    || t.cs() == "%" || t.cs() == "-") {
			context.check_layout(os);
			if (t.cs() == "-")
				os << "\\SpecialChar \\-\n";
			else
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
					output_ert_inset(os, "\\char`", context);
				}
			} else {
				output_ert_inset(os, "\\char", context);
			}
		}

		else if (t.cs() == "verb") {
			context.check_layout(os);
			// set catcodes to verbatim early, just in case.
			p.setCatcodes(VERBATIM_CATCODES);
			string delim = p.get_token().asInput();
			Parser::Arg arg = p.verbatimStuff(delim);
			if (arg.first)
				output_ert_inset(os, "\\verb" + delim 
						 + arg.second + delim, context);
			else
				cerr << "invalid \\verb command. Skipping" << endl;
		}

		// Problem: \= creates a tabstop inside the tabbing environment
		// and else an accent. In the latter case we really would want
		// \={o} instead of \= o.
		else if (t.cs() == "=" && (flags & FLAG_TABBING))
			output_ert_inset(os, t.asInput(), context);

		else if (t.cs() == "\\") {
			context.check_layout(os);
			if (p.hasOpt())
				output_ert_inset(os, "\\\\" + p.getOpt(), context);
			else if (p.next_token().asInput() == "*") {
				p.get_token();
				// getOpt() eats the following space if there
				// is no optional argument, but that is OK
				// here since it has no effect in the output.
				output_ert_inset(os, "\\\\*" + p.getOpt(), context);
			}
			else {
				begin_inset(os, "Newline newline");
				end_inset(os);
			}
		}

		else if (t.cs() == "newline" ||
		         (t.cs() == "linebreak" && !p.hasOpt())) {
			context.check_layout(os);
			begin_inset(os, "Newline ");
			os << t.cs();
			end_inset(os);
			skip_spaces_braces(p);
		}

		else if (t.cs() == "input" || t.cs() == "include"
			 || t.cs() == "verbatiminput") {
			string name = t.cs();
			if (t.cs() == "verbatiminput"
			    && p.next_token().asInput() == "*")
				name += p.get_token().asInput();
			context.check_layout(os);
			string filename(normalize_filename(p.getArg('{', '}')));
			string const path = getMasterFilePath(true);
			// We want to preserve relative / absolute filenames,
			// therefore path is only used for testing
			if ((t.cs() == "include" || t.cs() == "input") &&
			    !makeAbsPath(filename, path).exists()) {
				// The file extension is probably missing.
				// Now try to find it out.
				string const tex_name =
					find_file(filename, path,
						  known_tex_extensions);
				if (!tex_name.empty())
					filename = tex_name;
			}
			bool external = false;
			string outname;
			if (makeAbsPath(filename, path).exists()) {
				string const abstexname =
					makeAbsPath(filename, path).absFileName();
				string const absfigname =
					changeExtension(abstexname, ".fig");
				fix_child_filename(filename);
				string const lyxname = changeExtension(filename,
					roundtripMode() ? ".lyx.lyx" : ".lyx");
				string const abslyxname = makeAbsPath(
					lyxname, getParentFilePath(false)).absFileName();
				bool xfig = false;
				if (!skipChildren())
					external = FileName(absfigname).exists();
				if (t.cs() == "input" && !skipChildren()) {
					string const ext = getExtension(abstexname);

					// Combined PS/LaTeX:
					// x.eps, x.pstex_t (old xfig)
					// x.pstex, x.pstex_t (new xfig, e.g. 3.2.5)
					FileName const absepsname(
						changeExtension(abstexname, ".eps"));
					FileName const abspstexname(
						changeExtension(abstexname, ".pstex"));
					bool const xfigeps =
						(absepsname.exists() ||
						 abspstexname.exists()) &&
						ext == "pstex_t";

					// Combined PDF/LaTeX:
					// x.pdf, x.pdftex_t (old xfig)
					// x.pdf, x.pdf_t (new xfig, e.g. 3.2.5)
					FileName const abspdfname(
						changeExtension(abstexname, ".pdf"));
					bool const xfigpdf =
						abspdfname.exists() &&
						(ext == "pdftex_t" || ext == "pdf_t");
					if (xfigpdf)
						pdflatex = true;

					// Combined PS/PDF/LaTeX:
					// x_pspdftex.eps, x_pspdftex.pdf, x.pspdftex
					string const absbase2(
						removeExtension(abstexname) + "_pspdftex");
					FileName const abseps2name(
						addExtension(absbase2, ".eps"));
					FileName const abspdf2name(
						addExtension(absbase2, ".pdf"));
					bool const xfigboth =
						abspdf2name.exists() &&
						abseps2name.exists() && ext == "pspdftex";

					xfig = xfigpdf || xfigeps || xfigboth;
					external = external && xfig;
				}
				if (external) {
					outname = changeExtension(filename, ".fig");
					FileName abssrc(changeExtension(abstexname, ".fig"));
					copy_file(abssrc, outname);
				} else if (xfig) {
					// Don't try to convert, the result
					// would be full of ERT.
					outname = filename;
					FileName abssrc(abstexname);
					copy_file(abssrc, outname);
				} else if (t.cs() != "verbatiminput" &&
				           !skipChildren() &&
				    tex2lyx(abstexname, FileName(abslyxname),
					    p.getEncoding())) {
					outname = lyxname;
					// no need to call copy_file
					// tex2lyx creates the file
				} else {
					outname = filename;
					FileName abssrc(abstexname);
					copy_file(abssrc, outname);
				}
			} else {
				cerr << "Warning: Could not find included file '"
				     << filename << "'." << endl;
				outname = filename;
			}
			if (external) {
				begin_inset(os, "External\n");
				os << "\ttemplate XFig\n"
				   << "\tfilename " << outname << '\n';
				registerExternalTemplatePackages("XFig");
			} else {
				begin_command_inset(os, "include", name);
				outname = subst(outname, "\"", "\\\"");
				os << "preview false\n"
				      "filename \"" << outname << "\"\n";
				if (t.cs() == "verbatiminput")
					preamble.registerAutomaticallyLoadedPackage("verbatim");
			}
			end_inset(os);
		}

		else if (t.cs() == "bibliographystyle") {
			// store new bibliographystyle
			bibliographystyle = p.verbatim_item();
			// If any other command than \bibliography, \addcontentsline
			// and \nocite{*} follows, we need to output the style
			// (because it might be used by that command).
			// Otherwise, it will automatically be output by LyX.
			p.pushPosition();
			bool output = true;
			for (Token t2 = p.get_token(); p.good(); t2 = p.get_token()) {
				if (t2.cat() == catBegin)
					break;
				if (t2.cat() != catEscape)
					continue;
				if (t2.cs() == "nocite") {
					if (p.getArg('{', '}') == "*")
						continue;
				} else if (t2.cs() == "bibliography")
					output = false;
				else if (t2.cs() == "phantomsection") {
					output = false;
					continue;
				}
				else if (t2.cs() == "addcontentsline") {
					// get the 3 arguments of \addcontentsline
					p.getArg('{', '}');
					p.getArg('{', '}');
					contentslineContent = p.getArg('{', '}');
					// if the last argument is not \refname we must output
					if (contentslineContent == "\\refname")
						output = false;
				}
				break;
			}
			p.popPosition();
			if (output) {
				output_ert_inset(os,
					"\\bibliographystyle{" + bibliographystyle + '}',
					context);
			}
		}

		else if (t.cs() == "phantomsection") {
			// we only support this if it occurs between
			// \bibliographystyle and \bibliography
			if (bibliographystyle.empty())
				output_ert_inset(os, "\\phantomsection", context);
		}

		else if (t.cs() == "addcontentsline") {
			context.check_layout(os);
			// get the 3 arguments of \addcontentsline
			string const one = p.getArg('{', '}');
			string const two = p.getArg('{', '}');
			string const three = p.getArg('{', '}');
			// only if it is a \refname, we support if for the bibtex inset
			if (contentslineContent != "\\refname") {
				output_ert_inset(os,
					"\\addcontentsline{" + one + "}{" + two + "}{"+ three + '}',
					context);
			}
		}

		else if (t.cs() == "bibliography") {
			context.check_layout(os);
			string BibOpts;
			begin_command_inset(os, "bibtex", "bibtex");
			if (!btprint.empty()) {
				os << "btprint " << '"' << "btPrintAll" << '"' << "\n";
				// clear the string because the next BibTeX inset can be without the
				// \nocite{*} option
				btprint.clear();
			}
			os << "bibfiles " << '"' << p.verbatim_item() << '"' << "\n";
			// Do we have addcontentsline?
			if (contentslineContent == "\\refname") {
				BibOpts = "bibtotoc";
				// clear string because next BibTeX inset can be without addcontentsline
				contentslineContent.clear();
			}
			// Do we have a bibliographystyle set?
			if (!bibliographystyle.empty()) {
				if (BibOpts.empty())
					BibOpts = bibliographystyle;
				else
					BibOpts = BibOpts + ',' + bibliographystyle;
				// clear it because each bibtex entry has its style
				// and we need an empty string to handle \phantomsection
				bibliographystyle.clear();
			}
			os << "options " << '"' << BibOpts << '"' << "\n";
			end_inset(os);
		}

		else if (t.cs() == "parbox") {
			// Test whether this is an outer box of a shaded box
			p.pushPosition();
			// swallow arguments
			while (p.hasOpt()) {
				p.getArg('[', ']');
				p.skip_spaces(true);
			}
			p.getArg('{', '}');
			p.skip_spaces(true);
			// eat the '{'
			if (p.next_token().cat() == catBegin)
				p.get_token();
			p.skip_spaces(true);
			Token to = p.get_token();
			bool shaded = false;
			if (to.asInput() == "\\begin") {
				p.skip_spaces(true);
				if (p.getArg('{', '}') == "shaded")
					shaded = true;
			}
			p.popPosition();
			if (shaded) {
				parse_outer_box(p, os, FLAG_ITEM, outer,
				                context, "parbox", "shaded");
			} else
				parse_box(p, os, 0, FLAG_ITEM, outer, context,
				          "", "", t.cs());
		}

		else if (t.cs() == "fbox" || t.cs() == "mbox" ||
			     t.cs() == "ovalbox" || t.cs() == "Ovalbox" ||
		         t.cs() == "shadowbox" || t.cs() == "doublebox")
			parse_outer_box(p, os, FLAG_ITEM, outer, context, t.cs(), "");

		else if (t.cs() == "framebox") {
			if (p.next_token().character() == '(') {
				//the syntax is: \framebox(x,y)[position]{content}
				string arg = t.asInput();
				arg += p.getFullParentheseArg();
				arg += p.getFullOpt();
				eat_whitespace(p, os, context, false);
				output_ert_inset(os, arg + '{', context);
				parse_text(p, os, FLAG_ITEM, outer, context);
				output_ert_inset(os, "}", context);
			} else {
				//the syntax is: \framebox[width][position]{content}
				string special = p.getFullOpt();
				special += p.getOpt();
				parse_outer_box(p, os, FLAG_ITEM, outer,
					            context, t.cs(), special);
			}
		}

		//\makebox() is part of the picture environment and different from \makebox{}
		//\makebox{} will be parsed by parse_box
		else if (t.cs() == "makebox") {
			if (p.next_token().character() == '(') {
				//the syntax is: \makebox(x,y)[position]{content}
				string arg = t.asInput();
				arg += p.getFullParentheseArg();
				arg += p.getFullOpt();
				eat_whitespace(p, os, context, false);
				output_ert_inset(os, arg + '{', context);
				parse_text(p, os, FLAG_ITEM, outer, context);
				output_ert_inset(os, "}", context);
			} else
				//the syntax is: \makebox[width][position]{content}
				parse_box(p, os, 0, FLAG_ITEM, outer, context,
				          "", "", t.cs());
		}

		else if (t.cs() == "smallskip" ||
			 t.cs() == "medskip" ||
			 t.cs() == "bigskip" ||
			 t.cs() == "vfill") {
			context.check_layout(os);
			begin_inset(os, "VSpace ");
			os << t.cs();
			end_inset(os);
			skip_spaces_braces(p);
		}

		else if ((where = is_known(t.cs(), known_spaces))) {
			context.check_layout(os);
			begin_inset(os, "space ");
			os << '\\' << known_coded_spaces[where - known_spaces]
			   << '\n';
			end_inset(os);
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
		         (t.cs() == "pagebreak" && !p.hasOpt()) ||
		         t.cs() == "clearpage" ||
		         t.cs() == "cleardoublepage") {
			context.check_layout(os);
			begin_inset(os, "Newpage ");
			os << t.cs();
			end_inset(os);
			skip_spaces_braces(p);
		}

		else if (t.cs() == "DeclareRobustCommand" ||
		         t.cs() == "DeclareRobustCommandx" ||
		         t.cs() == "newcommand" ||
		         t.cs() == "newcommandx" ||
		         t.cs() == "providecommand" ||
		         t.cs() == "providecommandx" ||
		         t.cs() == "renewcommand" ||
		         t.cs() == "renewcommandx") {
			// DeclareRobustCommand, DeclareRobustCommandx,
			// providecommand and providecommandx could be handled
			// by parse_command(), but we need to call
			// add_known_command() here.
			string name = t.asInput();
			if (p.next_token().asInput() == "*") {
				// Starred form. Eat '*'
				p.get_token();
				name += '*';
			}
			string const command = p.verbatim_item();
			string const opt1 = p.getFullOpt();
			string const opt2 = p.getFullOpt();
			add_known_command(command, opt1, !opt2.empty());
			string const ert = name + '{' + command + '}' +
					   opt1 + opt2 +
					   '{' + p.verbatim_item() + '}';

			if (t.cs() == "DeclareRobustCommand" ||
			    t.cs() == "DeclareRobustCommandx" ||
			    t.cs() == "providecommand" ||
			    t.cs() == "providecommandx" ||
			    name[name.length()-1] == '*')
				output_ert_inset(os, ert, context);
			else {
				context.check_layout(os);
				begin_inset(os, "FormulaMacro");
				os << "\n" << ert;
				end_inset(os);
			}
		}

		else if (t.cs() == "let" && p.next_token().asInput() != "*") {
			// let could be handled by parse_command(),
			// but we need to call add_known_command() here.
			string ert = t.asInput();
			string name;
			p.skip_spaces();
			if (p.next_token().cat() == catBegin) {
				name = p.verbatim_item();
				ert += '{' + name + '}';
			} else {
				name = p.verbatim_item();
				ert += name;
			}
			string command;
			p.skip_spaces();
			if (p.next_token().cat() == catBegin) {
				command = p.verbatim_item();
				ert += '{' + command + '}';
			} else {
				command = p.verbatim_item();
				ert += command;
			}
			// If command is known, make name known too, to parse
			// its arguments correctly. For this reason we also
			// have commands in syntax.default that are hardcoded.
			CommandMap::iterator it = known_commands.find(command);
			if (it != known_commands.end())
				known_commands[t.asInput()] = it->second;
			output_ert_inset(os, ert, context);
		}

		else if (t.cs() == "hspace" || t.cs() == "vspace") {
			if (starred)
				p.get_token();
			string name = t.asInput();
			string const length = p.verbatim_item();
			string unit;
			string valstring;
			bool valid = splitLatexLength(length, valstring, unit);
			bool known_hspace = false;
			bool known_vspace = false;
			bool known_unit = false;
			double value;
			if (valid) {
				istringstream iss(valstring);
				iss >> value;
				if (value == 1.0) {
					if (t.cs()[0] == 'h') {
						if (unit == "\\fill") {
							if (!starred) {
								unit = "";
								name = "\\hfill";
							}
							known_hspace = true;
						}
					} else {
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
				}
				if (!known_hspace && !known_vspace) {
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
					default: {
						//unitFromString(unit) fails for relative units like Length::PCW
						// therefore handle them separately
						if (unit == "\\paperwidth" || unit == "\\columnwidth"
							|| unit == "\\textwidth" || unit == "\\linewidth"
							|| unit == "\\textheight" || unit == "\\paperheight")
							known_unit = true;
						break;
							 }
					}
				}
			}

			// check for glue lengths
			bool is_gluelength = false;
			string gluelength = length;
			string::size_type i = length.find(" minus");
			if (i == string::npos) {
				i = length.find(" plus");
				if (i != string::npos)
					is_gluelength = true;
			} else
				is_gluelength = true;
			// if yes transform "9xx minus 8yy plus 7zz"
			// to "9xx-8yy+7zz"
			if (is_gluelength) {
				i = gluelength.find(" minus");
				if (i != string::npos)
					gluelength.replace(i, 7, "-");
				i = gluelength.find(" plus");
				if (i != string::npos)
					gluelength.replace(i, 6, "+");
			}

			if (t.cs()[0] == 'h' && (known_unit || known_hspace || is_gluelength)) {
				// Literal horizontal length or known variable
				context.check_layout(os);
				begin_inset(os, "space ");
				os << name;
				if (starred)
					os << '*';
				os << '{';
				if (known_hspace)
					os << unit;
				os << "}";
				if (known_unit && !known_hspace)
					os << "\n\\length " << translate_len(length);
				if (is_gluelength)
					os << "\n\\length " << gluelength;
				end_inset(os);
			} else if (known_unit || known_vspace || is_gluelength) {
				// Literal vertical length or known variable
				context.check_layout(os);
				begin_inset(os, "VSpace ");
				if (known_vspace)
					os << unit;
				if (known_unit && !known_vspace)
					os << translate_len(length);
				if (is_gluelength)
					os << gluelength;
				if (starred)
					os << '*';
				end_inset(os);
			} else {
				// LyX can't handle other length variables in Inset VSpace/space
				if (starred)
					name += '*';
				if (valid) {
					if (value == 1.0)
						output_ert_inset(os, name + '{' + unit + '}', context);
					else if (value == -1.0)
						output_ert_inset(os, name + "{-" + unit + '}', context);
					else
						output_ert_inset(os, name + '{' + valstring + unit + '}', context);
				} else
					output_ert_inset(os, name + '{' + length + '}', context);
			}
		}

		// The single '=' is meant here.
		else if ((newinsetlayout = findInsetLayout(context.textclass, starredname, true))) {
			if (starred)
				p.get_token();
			p.skip_spaces();
			context.check_layout(os);
			docstring const name = newinsetlayout->name();
			bool const caption = name.find(from_ascii("Caption:")) == 0;
			if (caption) {
				begin_inset(os, "Caption ");
				os << to_utf8(name.substr(8)) << '\n';
			} else {
				begin_inset(os, "Flex ");
				os << to_utf8(name) << '\n'
				   << "status collapsed\n";
			}
			if (newinsetlayout->isPassThru()) {
				// set catcodes to verbatim early, just in case.
				p.setCatcodes(VERBATIM_CATCODES);
				string delim = p.get_token().asInput();
				if (delim != "{")
					cerr << "Warning: bad delimiter for command " << t.asInput() << endl;
				//FIXME: handle error condition
				string const arg = p.verbatimStuff("}").second;
				Context newcontext(true, context.textclass);
				if (newinsetlayout->forcePlainLayout())
					newcontext.layout = &context.textclass.plainLayout();
				output_ert(os, arg, newcontext);
			} else
				
				parse_text_in_inset(p, os, FLAG_ITEM, false, context, newinsetlayout);
			if (caption)
				p.skip_spaces();
			end_inset(os);
		}

		else if (t.cs() == "includepdf") {
			p.skip_spaces();
			string const arg = p.getArg('[', ']');
			map<string, string> opts;
			vector<string> keys;
			split_map(arg, opts, keys);
			string name = normalize_filename(p.verbatim_item());
			string const path = getMasterFilePath(true);
			// We want to preserve relative / absolute filenames,
			// therefore path is only used for testing
			if (!makeAbsPath(name, path).exists()) {
				// The file extension is probably missing.
				// Now try to find it out.
				char const * const pdfpages_format[] = {"pdf", 0};
				string const pdftex_name =
					find_file(name, path, pdfpages_format);
				if (!pdftex_name.empty()) {
					name = pdftex_name;
					pdflatex = true;
				}
			}
			FileName const absname = makeAbsPath(name, path);
			if (absname.exists())
			{
				fix_child_filename(name);
				copy_file(absname, name);
			} else
				cerr << "Warning: Could not find file '"
				     << name << "'." << endl;
			// write output
			context.check_layout(os);
			begin_inset(os, "External\n\ttemplate ");
			os << "PDFPages\n\tfilename "
			   << name << "\n";
			// parse the options
			if (opts.find("pages") != opts.end())
				os << "\textra LaTeX \"pages="
				   << opts["pages"] << "\"\n";
			if (opts.find("angle") != opts.end())
				os << "\trotateAngle "
				   << opts["angle"] << '\n';
			if (opts.find("origin") != opts.end()) {
				ostringstream ss;
				string const opt = opts["origin"];
				if (opt == "tl") ss << "topleft";
				if (opt == "bl") ss << "bottomleft";
				if (opt == "Bl") ss << "baselineleft";
				if (opt == "c") ss << "center";
				if (opt == "tc") ss << "topcenter";
				if (opt == "bc") ss << "bottomcenter";
				if (opt == "Bc") ss << "baselinecenter";
				if (opt == "tr") ss << "topright";
				if (opt == "br") ss << "bottomright";
				if (opt == "Br") ss << "baselineright";
				if (!ss.str().empty())
					os << "\trotateOrigin " << ss.str() << '\n';
				else
					cerr << "Warning: Ignoring unknown includegraphics origin argument '" << opt << "'\n";
			}
			if (opts.find("width") != opts.end())
				os << "\twidth "
				   << translate_len(opts["width"]) << '\n';
			if (opts.find("height") != opts.end())
				os << "\theight "
				   << translate_len(opts["height"]) << '\n';
			if (opts.find("keepaspectratio") != opts.end())
				os << "\tkeepAspectRatio\n";
			end_inset(os);
			context.check_layout(os);
			registerExternalTemplatePackages("PDFPages");
		}

		else if (t.cs() == "loadgame") {
			p.skip_spaces();
			string name = normalize_filename(p.verbatim_item());
			string const path = getMasterFilePath(true);
			// We want to preserve relative / absolute filenames,
			// therefore path is only used for testing
			if (!makeAbsPath(name, path).exists()) {
				// The file extension is probably missing.
				// Now try to find it out.
				char const * const lyxskak_format[] = {"fen", 0};
				string const lyxskak_name =
					find_file(name, path, lyxskak_format);
				if (!lyxskak_name.empty())
					name = lyxskak_name;
			}
			FileName const absname = makeAbsPath(name, path);
			if (absname.exists())
			{
				fix_child_filename(name);
				copy_file(absname, name);
			} else
				cerr << "Warning: Could not find file '"
				     << name << "'." << endl;
			context.check_layout(os);
			begin_inset(os, "External\n\ttemplate ");
			os << "ChessDiagram\n\tfilename "
			   << name << "\n";
			end_inset(os);
			context.check_layout(os);
			// after a \loadgame follows a \showboard
			if (p.get_token().asInput() == "showboard")
				p.get_token();
			registerExternalTemplatePackages("ChessDiagram");
		}

		else {
			// try to see whether the string is in unicodesymbols
			// Only use text mode commands, since we are in text mode here,
			// and math commands may be invalid (bug 6797)
			string name = t.asInput();
			// handle the dingbats, cyrillic and greek
			if (name == "\\ding" || name == "\\textcyr" ||
			    (name == "\\textgreek" && !preamble.usePolyglossia()))
				name = name + '{' + p.getArg('{', '}') + '}';
			// handle the ifsym characters
			else if (name == "\\textifsymbol") {
				string const optif = p.getFullOpt();
				string const argif = p.getArg('{', '}');
				name = name + optif + '{' + argif + '}';
			}
			// handle the \ascii characters
			// the case of \ascii within braces, as LyX outputs it, is already
			// handled for t.cat() == catBegin
			else if (name == "\\ascii") {
				// the code is "\asci\xxx"
				name = "{" + name + p.get_token().asInput() + "}";
				skip_braces(p);
			}
			// handle some TIPA special characters
			else if (preamble.isPackageUsed("tipa")) {
				if (name == "\\textglobfall") {
					name = "End";
					skip_braces(p);
				} else if (name == "\\s") {
					// fromLaTeXCommand() does not yet
					// recognize tipa short cuts
					name = "\\textsyllabic";
				} else if (name == "\\=" &&
				           p.next_token().asInput() == "*") {
					// fromLaTeXCommand() does not yet
					// recognize tipa short cuts
					p.get_token();
					name = "\\b";
				} else if (name == "\\textdoublevertline") {
					// FIXME: This is not correct,
					// \textvertline is higher than \textbardbl
					name = "\\textbardbl";
					skip_braces(p);
				} else if (name == "\\!" ) {
					if (p.next_token().asInput() == "b") {
						p.get_token();	// eat 'b'
						name = "\\texthtb";
						skip_braces(p);
					} else if (p.next_token().asInput() == "d") {
						p.get_token();
						name = "\\texthtd";
						skip_braces(p);
					} else if (p.next_token().asInput() == "g") {
						p.get_token();
						name = "\\texthtg";
						skip_braces(p);
					} else if (p.next_token().asInput() == "G") {
						p.get_token();
						name = "\\texthtscg";
						skip_braces(p);
					} else if (p.next_token().asInput() == "j") {
						p.get_token();
						name = "\\texthtbardotlessj";
						skip_braces(p);
					} else if (p.next_token().asInput() == "o") {
						p.get_token();
						name = "\\textbullseye";
						skip_braces(p);
					}
				} else if (name == "\\*" ) {
					if (p.next_token().asInput() == "k") {
						p.get_token();
						name = "\\textturnk";
						skip_braces(p);
					} else if (p.next_token().asInput() == "r") {
						p.get_token();	// eat 'b'
						name = "\\textturnr";
						skip_braces(p);
					} else if (p.next_token().asInput() == "t") {
						p.get_token();
						name = "\\textturnt";
						skip_braces(p);
					} else if (p.next_token().asInput() == "w") {
						p.get_token();
						name = "\\textturnw";
						skip_braces(p);
					}
				}
			}
			if ((name.size() == 2 &&
			     contains("\"'.=^`bcdHkrtuv~", name[1]) &&
			     p.next_token().asInput() != "*") ||
			    is_known(name.substr(1), known_tipa_marks)) {
				// name is a command that corresponds to a
				// combining character in unicodesymbols.
				// Append the argument, fromLaTeXCommand()
				// will either convert it to a single
				// character or a combining sequence.
				name += '{' + p.verbatim_item() + '}';
			}
			// now get the character from unicodesymbols
			bool termination;
			docstring rem;
			set<string> req;
			docstring s = encodings.fromLaTeXCommand(from_utf8(name),
					Encodings::TEXT_CMD, termination, rem, &req);
			if (!s.empty()) {
				context.check_layout(os);
				os << to_utf8(s);
				if (!rem.empty())
					output_ert_inset(os, to_utf8(rem), context);
				if (termination)
					skip_spaces_braces(p);
				for (set<string>::const_iterator it = req.begin(); it != req.end(); ++it)
					preamble.registerAutomaticallyLoadedPackage(*it);
			}
			//cerr << "#: " << t << " mode: " << mode << endl;
			// heuristic: read up to next non-nested space
			/*
			string s = t.asInput();
			string z = p.verbatim_item();
			while (p.good() && z != " " && !z.empty()) {
				//cerr << "read: " << z << endl;
				s += z;
				z = p.verbatim_item();
			}
			cerr << "found ERT: " << s << endl;
			output_ert_inset(os, s + ' ', context);
			*/
			else {
				if (t.asInput() == name &&
				    p.next_token().asInput() == "*") {
					// Starred commands like \vspace*{}
					p.get_token();	// Eat '*'
					name += '*';
				}
				if (!parse_command(name, p, os, outer, context))
					output_ert_inset(os, name, context);
			}
		}

		if (flags & FLAG_LEAVE) {
			flags &= ~FLAG_LEAVE;
			break;
		}
	}
}


string guessLanguage(Parser & p, string const & lang)
{
	typedef std::map<std::string, size_t> LangMap;
	// map from language names to number of characters
	LangMap used;
	used[lang] = 0;
	for (char const * const * i = supported_CJK_languages; *i; i++)
		used[string(*i)] = 0;

	while (p.good()) {
		Token const t = p.get_token();
		// comments are not counted for any language
		if (t.cat() == catComment)
			continue;
		// commands are not counted as well, but we need to detect
		// \begin{CJK} and switch encoding if needed
		if (t.cat() == catEscape) {
			if (t.cs() == "inputencoding") {
				string const enc = subst(p.verbatim_item(), "\n", " ");
				p.setEncoding(enc, Encoding::inputenc);
				continue;
			}
			if (t.cs() != "begin")
				continue;
		} else {
			// Non-CJK content is counted for lang.
			// We do not care about the real language here:
			// If we have more non-CJK contents than CJK contents,
			// we simply use the language that was specified as
			// babel main language.
			used[lang] += t.asInput().length();
			continue;
		}
		// Now we are starting an environment
		p.pushPosition();
		string const name = p.getArg('{', '}');
		if (name != "CJK") {
			p.popPosition();
			continue;
		}
		// It is a CJK environment
		p.popPosition();
		/* name = */ p.getArg('{', '}');
		string const encoding = p.getArg('{', '}');
		/* mapping = */ p.getArg('{', '}');
		string const encoding_old = p.getEncoding();
		char const * const * const where =
			is_known(encoding, supported_CJK_encodings);
		if (where)
			p.setEncoding(encoding, Encoding::CJK);
		else
			p.setEncoding("UTF-8");
		string const text = p.ertEnvironment("CJK");
		p.setEncoding(encoding_old);
		p.skip_spaces();
		if (!where) {
			// ignore contents in unknown CJK encoding
			continue;
		}
		// the language of the text
		string const cjk =
			supported_CJK_languages[where - supported_CJK_encodings];
		used[cjk] += text.length();
	}
	LangMap::const_iterator use = used.begin();
	for (LangMap::const_iterator it = used.begin(); it != used.end(); ++it) {
		if (it->second > use->second)
			use = it;
	}
	return use->first;
}

// }])


} // namespace lyx
