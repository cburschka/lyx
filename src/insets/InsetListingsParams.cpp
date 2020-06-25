/**
 * \file InsetListingsParams.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <algorithm>

#include "InsetListingsParams.h"

#include "Length.h"
#include "Lexer.h"

#include "support/convert.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

enum param_type {
	ALL,  // accept all
	TRUEFALSE, // accept 'true' or 'false'
	INTEGER, // accept an integer
	LENGTH,  // accept a latex length
	SKIP,    // accept a skip or a length
	ONEOF,  // accept one of a few values
	SUBSETOF // accept a string composed of given characters
};


/// Listings package parameter information.
// FIXME: make this class visible outside of this file so that
// FIXME: it can be used directly in the frontend and in the LyX format
// FIXME: parsing.
class ListingsParam {
public:
	/// Default ctor for STL containers.
	ListingsParam(): onoff_(false), type_(ALL)
	{}
	/// Main ctor.
	ListingsParam(string const & v, bool o, param_type t,
		string const & i, docstring const & h)
		: value_(v), onoff_(o), type_(t), info_(i), hint_(h)
	{}
	/// Validate a parameter.
	/// \retval an empty string if \c par is valid.
	/// \retval otherwise an explanation WRT to \c par invalidity.
	docstring validate(string const & par) const;
private:
	/// default value
	string value_;
public:
	/// for option with value "true", "false".
	/// if onoff is true,
	///   "true":  option
	///   "false":
	///   "other": option="other"
	/// onoff is false,
	///   "true":  option=true
	///   "false": option=false
	// FIXME: this is public because of InsetListingParam::addParam()
	bool onoff_;
private:
	/// validator type.
	/// ALL:
	/// TRUEFALSE:
	/// INTEGER:
	/// LENGTH:
	///     info is ignored.
	/// ONEOF
	///     info is a \n separated string with allowed values
	/// SUBSETOF
	///     info is a string from which par is composed of
	///     (e.g. floatplacement can be one or more of *tbph)
	param_type type_;
	/// information which meaning depends on parameter type.
	/// \sa type_
	string info_;
	/// a help message that is displayed in the gui.
	docstring hint_;
};


char const * allowed_skips = "\\smallskipamount,\\medskipamount,\\bigskipamount";


docstring ListingsParam::validate(string const & par) const
{
	bool unclosed = false;
	string par2 = par;
	// braces are allowed
	if (prefixIs(par, "{") && suffixIs(par, "}") && !suffixIs(par, "\\}"))
		par2 = par.substr(1, par.size() - 2);

	// check for unmatched braces
	int braces = 0;
	for (size_t i = 0; i < par2.size(); ++i) {
		if (par2[i] == '{' && (i == 0 || par2[i-1] != '\\'))
			++braces;
		else if (par2[i] == '}' && (i == 0 || par2[i-1] != '\\'))
			--braces;
	}
	unclosed = braces != 0;

	switch (type_) {

	case ALL:
		if (par2.empty() && !onoff_) {
			if (!hint_.empty())
				return hint_;
			else
				return _("A value is expected.");
		}
		if (unclosed)
			return _("Unbalanced braces!");
		return docstring();

	case TRUEFALSE:
		if (par2.empty() && !onoff_) {
			if (!hint_.empty())
				return hint_;
			else
				return _("Please specify true or false.");
		}
		if (par2 != "true" && par2 != "false")
			return _("Only true or false is allowed.");
		if (unclosed)
			return _("Unbalanced braces!");
		return docstring();

	case INTEGER:
		if (!isStrInt(par2)) {
			if (!hint_.empty())
				return hint_;
			else
				return _("Please specify an integer value.");
		}
		if (convert<int>(par2) == 0 && par2[0] != '0')
			return _("An integer is expected.");
		if (unclosed)
			return _("Unbalanced braces!");
		return docstring();

	case LENGTH:
		if (par2.empty() && !onoff_) {
			if (!hint_.empty())
				return hint_;
			else
				return _("Please specify a LaTeX length expression.");
		}
		if (!isValidLength(par2))
			return _("Invalid LaTeX length expression.");
		if (unclosed)
			return _("Unbalanced braces!");
		return docstring();

	case SKIP:
		if (par2.empty() && !onoff_) {
			if (!hint_.empty())
				return hint_;
			else
				return bformat(_("Please specify a LaTeX length expression or a skip amount (%1$s)"),
					       from_ascii(subst(allowed_skips, ",", ", ")));
		}
		if (!isValidLength(par2) && tokenPos(allowed_skips, ',', par2) == -1)
			return _("Not a valid LaTeX length expression or skip amount.");
		if (unclosed)
			return _("Unbalanced braces!");
		return docstring();

	case ONEOF: {
		if (par2.empty() && !onoff_) {
			if (!hint_.empty())
				return hint_;
			else
				return bformat(_("Please specify one of %1$s."),
							   from_utf8(info_));
		}
		// break value to allowed strings
		vector<string> lists;
		string v;
		for (size_t i = 0; i != info_.size(); ++i) {
			if (info_[i] == '\n') {
				lists.push_back(v);
				v = string();
			} else
				v += info_[i];
		}
		if (!v.empty())
			lists.push_back(v);

		// good, find the string
		if (find(lists.begin(), lists.end(), par2) != lists.end()) {
			if (unclosed)
				return _("Unbalanced braces!");
			return docstring();
		}
		// otherwise, produce a meaningful error message.
		string matching_names;
		for (vector<string>::iterator it = lists.begin();
			it != lists.end(); ++it) {
			if (it->size() >= par2.size() && it->substr(0, par2.size()) == par2) {
				if (matching_names.empty())
					matching_names += *it;
				else
					matching_names += ", " + *it;
			}
		}
		if (matching_names.empty())
			return bformat(_("Try one of %1$s."), from_utf8(info_));
		else
			return bformat(_("I guess you mean %1$s."), from_utf8(matching_names));
	}
	case SUBSETOF:
		if (par2.empty() && !onoff_) {
			if (!hint_.empty())
				return hint_;
			else
				return bformat(_("Please specify one or more of '%1$s'."),
							   from_utf8(info_));
		}
		for (size_t i = 0; i < par2.size(); ++i)
			if (info_.find(par2[i], 0) == string::npos)
				return bformat(_("Should be composed of one or more of %1$s."),
						from_utf8(info_));
		if (unclosed)
			return _("Unbalanced braces!");
		return docstring();
	}
	return docstring();
}


/// languages and language/dialect combinations
char const * allowed_languages =
	"no language\nABAP\n[R/2 4.3]ABAP\n[R/2 5.0]ABAP\n[R/3 3.1]ABAP\n"
	"[R/3 4.6C]ABAP\n[R/3 6.10]ABAP\nACM\nACMscript\nACSL\nAda\n[2005]Ada\n[83]Ada\n"
	"[95]Ada\nALGOL\n[60]ALGOL\n[68]ALGOL\nAnt\nAssembler\n"
	"[Motorola68k]Assembler\n[x86masm]Assembler\nAwk\n[gnu]Awk\n[POSIX]Awk\n"
	"bash\nBasic\n[Visual]Basic\nC\n[ANSI]C\n[Handel]C\n[Objective]C\n"
	"[Sharp]C\nC++\n[ANSI]C++\n[GNU]C++\n[ISO]C++\n[Visual]C++\nCaml\n"
	"[light]Caml\n[Objective]Caml\nCIL\nClean\nCobol\n[1974]Cobol\n[1985]Cobol\n"
	"[ibm]Cobol\nComal 80\ncommand.com\n[WinXP]command.com\nComsol\ncsh\n"
	"Delphi\nEiffel\nElan\nelisp\nerlang\nEuphoria\nFortran\n[77]Fortran\n[90]Fortran\n"
	"[95]Fortran\n[03]Fortran\n[08]Fortran\nGAP\nGCL\nGnuplot\nGo\nhansl\nHaskell\nHTML\nIDL\n[CORBA]IDL\ninform\n"
	"Java\n[AspectJ]Java\nJVMIS\nksh\nLingo\nLisp\n[Auto]Lisp\nLLVM\nLogo\n"
	"Lua\n[5.0]Lua\n[5.1]Lua\n[5.2]Lua\n[5.3]Lua\n"
	"make\n[gnu]make\nMathematica\n[1.0]Mathematica\n[3.0]Mathematica\n[11.0]Mathematica\n"
	"[5.2]Mathematica\nMatlab\nMercury\nMetaPost\nMiranda\nMizar\nML\n"
	"Modula-2\nMuPAD\nNASTRAN\nOberon-2\nOCL\n[decorative]OCL\n[OMG]OCL\n"
	"Octave\nOORexx\nOz\nPascal\n[Borland6]Pascal\n[Standard]Pascal\n[XSC]Pascal\n"
	"Perl\nPHP\nPL/I\nPlasm\nPostScript\nPOV\nProlog\nPromela\nPSTricks\n"
	"Python\nR\nReduce\nRexx\n[VM/XA]Rexx\nRSL\nRuby\nS\n[PLUS]S\nSAS\nScala\nScilab\nsh\n"
	"SHELXL\nSimula\n[67]Simula\n[CII]Simula\n[DEC]Simula\n[IBM]Simula\n"
	"SPARQL\nSQL\ntcl\n[tk]tcl\nTeX\n[AlLaTeX]TeX\n[common]TeX\n[LaTeX]TeX\n"
	"[plain]TeX\n[primitive]TeX\nVBScript\nVerilog\nVHDL\n[AMS]VHDL\nVRML\n"
	"[97]VRML\nXML\nXSLT";


/// Return language allowed in the GUI without dialect and proper casing
string const languageonly(string const & lang)
{
	string const locase = ascii_lowercase(trim(lang, "{}"));
	string const all_languages = ascii_lowercase(allowed_languages) + "\n";
	string language = (lang.at(0) == '[') ? locase + "\n"
	                                      : string("]") + locase + "\n";
	size_t i = all_languages.find(language);
	if (i == string::npos && lang.at(0) != '[') {
		language[0] = '\n';
		i = all_languages.find(language);
	}
	if (i == string::npos)
		return lang;
	if (all_languages.at(i) == '[')
		i = all_languages.find(']', i);
	if (i == string::npos)
		return lang;
	size_t j = all_languages.find('\n', i + 1);
	if (j == string::npos)
		return lang;
	return string(allowed_languages).substr(i + 1, j - i - 1);
}


/// ListingsParam Validator.
/// This class is aimed to be a singleton which is instantiated in
/// \c InsetListingsParams::addParam().
// FIXME: transfer this validator to the frontend.
// FIXME: avoid the use of exception.
class ParValidator
{
public:
	ParValidator();

	/// validate a parameter for a given name.
	/// return an error message if \c par is an invalid parameter.
	docstring validate(string const & name, string const & par) const;

	/// return the onoff status of a parameter \c key, if \c key is not found
	/// return false
	bool onoff(string const & key) const;

private:
	/// key is the name of the parameter
	typedef	map<string, ListingsParam> ListingsParams;
	ListingsParams all_params_[2];
};


ParValidator::ParValidator()
{
	docstring const empty_hint;
	docstring const style_hint = _("Use \\footnotesize, \\small, \\itshape, "
		"\\ttfamily or something like that");
	docstring const frame_hint_mint =
		_("none, leftline, topline, bottomline, lines, single");
	docstring const frame_hint_lst =
		_("none, leftline, topline, bottomline, lines, "
		"single, shadowbox or subset of trblTRBL");
	docstring const frameround_hint = _("Enter four letters (either t = round "
		"or f = square) for top right, bottom "
		"right, bottom left and top left corner.");
	docstring const color_hint_mint =
			_("Previously defined color name as a string");
	docstring const color_hint_lst =
			_("Enter something like \\color{white}");

	// Listings package

	/// options copied from page 26 of listings manual
	// FIXME: add default parameters ... (which is not used now)
	all_params_[0]["float"] =
		ListingsParam("false", true, SUBSETOF, "*tbph", empty_hint);
	all_params_[0]["floatplacement"] =
		ListingsParam("tbp", false, SUBSETOF, "tbp", empty_hint);
	all_params_[0]["aboveskip"] =
		ListingsParam("\\medskipamount", false, SKIP, "", empty_hint);
	all_params_[0]["belowskip"] =
		ListingsParam("\\medskipamount", false, SKIP, "", empty_hint);
	all_params_[0]["lineskip"] =
		ListingsParam("", false, SKIP, "", empty_hint);
	all_params_[0]["boxpos"] =
		ListingsParam("", false, SUBSETOF, "bct", empty_hint);
	all_params_[0]["print"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["firstline"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_[0]["lastline"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_[0]["linerange"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["showlines"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["emptylines"] =
		ListingsParam("", false, ALL, "", _(
		"Expect a number with an optional * before it"));
	all_params_[0]["gobble"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_[0]["style"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["language"] =
		ListingsParam("", false, ONEOF, allowed_languages, empty_hint);
	all_params_[0]["alsolanguage"] =
		ListingsParam("", false, ONEOF, allowed_languages, empty_hint);
	all_params_[0]["defaultdialect"] =
		ListingsParam("", false, ONEOF, allowed_languages, empty_hint);
	all_params_[0]["printpod"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["usekeywordsintag"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["tagstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_[0]["markfirstintag"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_[0]["makemacrouse"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["basicstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_[0]["identifierstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_[0]["commentstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_[0]["stringstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_[0]["keywordstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_[0]["ndkeywordstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_[0]["classoffset"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_[0]["texcsstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_[0]["directivestyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_[0]["emph"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["moreemph"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["deleteemph"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["emphstyle"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["delim"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["moredelim"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["deletedelim"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["extendedchars"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["inputencoding"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["upquote"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["tabsize"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_[0]["showtabs"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["tab"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["showspaces"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["showstringspaces"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["formfeed"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["numbers"] =
		ListingsParam("", false, ONEOF, "none\nleft\nright", empty_hint);
	all_params_[0]["stepnumber"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_[0]["numberfirstline"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["numberstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_[0]["numbersep"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[0]["numberblanklines"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["firstnumber"] =
		ListingsParam("", false, ALL, "", _("auto, last or a number"));
	all_params_[0]["name"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["thelstnumber"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["title"] =
		ListingsParam("", false, ALL, "", empty_hint);
	// this option is not handled in the parameter box
	all_params_[0]["caption"] =
		ListingsParam("", false, ALL, "", _(
		"This parameter should not be entered here. Please use the caption "
		"edit box (when using the child document dialog) or "
		"menu Insert->Caption (when defining a listing inset)"));
	// this option is not handled in the parameter box
	all_params_[0]["label"] =
		ListingsParam("", false, ALL, "",_(
		"This parameter should not be entered here. Please use the label "
		"edit box (when using the child document dialog) or "
		"menu Insert->Label (when defining a listing inset)"));
	all_params_[0]["nolol"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["captionpos"] =
		ListingsParam("", false, SUBSETOF, "tb", empty_hint);
	all_params_[0]["abovecaptionskip"] =
		ListingsParam("", false, SKIP, "", empty_hint);
	all_params_[0]["belowcaptionskip"] =
		ListingsParam("", false, SKIP, "", empty_hint);
	all_params_[0]["linewidth"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[0]["xleftmargin"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[0]["xrightmargin"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[0]["resetmargins"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["breaklines"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["breakatwhitespace"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["prebreak"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["postbreak"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["breakindent"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[0]["breakautoindent"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["frame"] =
		ListingsParam("", false, ALL, "", frame_hint_lst);
	all_params_[0]["frameround"] =
		ListingsParam("", false, SUBSETOF, "tf", frameround_hint);
	all_params_[0]["framesep"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[0]["rulesep"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[0]["framerule"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[0]["framexleftmargin"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[0]["framexrightmargin"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[0]["framextopmargin"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[0]["framexbottommargin"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[0]["backgroundcolor"] =
		ListingsParam("", false, ALL, "", color_hint_lst);
	all_params_[0]["rulecolor"] =
		ListingsParam("", false, ALL, "", color_hint_lst);
	all_params_[0]["fillcolor"] =
		ListingsParam("", false, ALL, "", color_hint_lst);
	all_params_[0]["rulesepcolor"] =
		ListingsParam("", false, ALL, "", color_hint_lst);
	all_params_[0]["frameshape"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["index"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["moreindex"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["deleteindex"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["indexstyle"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["columns"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["flexiblecolumns"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["keepspaces"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["basewidth"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[0]["fontadjust"] =
		ListingsParam("", true, TRUEFALSE, "", empty_hint);
	all_params_[0]["texcl"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["mathescape"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["escapechar"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["escapeinside"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["escapebegin"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["escapeend"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["fancyvrb"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["fvcmdparams"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["morefvcmdparams"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["keywordsprefix"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["keywords"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["morekeywords"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["deletekeywords"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["ndkeywords"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["morendkeywords"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["deletendkeywords"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["texcs"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["moretexcs"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["deletetexcs"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["directives"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["moredirectives"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["deletedirectives"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["sensitive"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["alsoletter"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["alsodigit"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["alsoother"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["otherkeywords"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["tag"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["string"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["morestring"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["deletestring"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["comment"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["morecomment"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["deletecomment"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["keywordcomment"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["morekeywordcomment"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["deletekeywordcomment"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["keywordcommentsemicolon"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["podcomment"] =
		ListingsParam("", false, ALL, "", empty_hint);
	// the following are experimental listings features
	all_params_[0]["procnamekeys"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["moreprocnamekeys"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["deleteprocnamekeys"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["procnamestyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_[0]["indexprocnames"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["hyperref"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["morehyperref"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["deletehyperref"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["hyperanchor"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["hyperlink"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["literate"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["lgrindef"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["rangebeginprefix"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["rangebeginsuffix"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["rangeendprefix"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["rangeendsuffix"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["rangeprefix"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["rangesuffix"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[0]["includerangemarker"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[0]["multicols"] =
		ListingsParam("", false, INTEGER, "", empty_hint);

	// Minted package

	// This is not a real minted option and its only purpose
	// is to get a caption for a floating listing.
	all_params_[1]["caption"] =
		ListingsParam("", false, ALL, "", _(
		"This parameter should not be entered here. Please use the caption "
		"edit box (when using the child document dialog) or "
		"menu Insert->Caption (when defining a listing inset)"));
	// The "label" minted option is being subverted here for the
	// sake of getting a label for a floating listing.
	all_params_[1]["label"] =
		ListingsParam("", false, ALL, "",_(
		"This parameter should not be entered here. Please use the label "
		"edit box (when using the child document dialog) or "
		"menu Insert->Label (when defining a listing inset)"));
	// This is not a real minted option and its only purpose
	// is to signal that this is a floating listing.
	all_params_[1]["float"] =
		ListingsParam("false", true, SUBSETOF, "*tbph", empty_hint);
	all_params_[1]["cache"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["cachedir"] =
		ListingsParam("", false, ALL, "", _(
					"default: _minted-<jobname>"));
	all_params_[1]["finalizecache"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["frozencache"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["draft"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["final"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["kpsewhich"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["langlinenos"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["newfloat"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["outputdir"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["autogobble"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["baselinestretch"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["breakafter"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["breakaftergroup"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["breakaftersymbolpre"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["breakaftersymbolpost"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["breakanywhere"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["breakanywheresymbolpre"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["breakanywheresymbolpost"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["breakautoindent"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["breakbefore"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["breakbeforegroup"] =
		ListingsParam("", true, ALL, "", empty_hint);
	all_params_[1]["breakbeforesymbolpre"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["breakbeforesymbolpost"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["breakbytoken"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["breakbytokenanywhere"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["breakindent"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[1]["breaklines"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["breaksymbol"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["breaksymbolleft"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["breaksymbolright"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["breaksymbolindent"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[1]["breaksymbolindentleft"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[1]["breaksymbolindentright"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[1]["breaksymbolsep"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[1]["breaksymbolsepleft"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[1]["breaksymbolsepright"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[1]["bgcolor"] =
		ListingsParam("", false, ALL, "", color_hint_mint);
	all_params_[1]["codetagify"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["curlyquotes"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["encoding"] =
		ListingsParam("", false, ALL, "", _(
				"Sets encoding expected by Pygments"));
	all_params_[1]["escapeinside"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["firstline"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_[1]["firstnumber"] =
		ListingsParam("", false, ALL, "", _(
					"auto, last or a number"));
	all_params_[1]["fontfamily"] =
		ListingsParam("", false, ALL, "", _(
				"A latex family such as tt, sf, rm"));
	all_params_[1]["fontseries"] =
		ListingsParam("", false, ALL, "", _(
				"A latex series such as m, b, c, bx, sb"));
	all_params_[1]["fontsize"] =
		ListingsParam("", false, ALL, "", _(
				"A latex name such as \\small"));
	all_params_[1]["fontshape"] =
		ListingsParam("", false, ALL, "", _(
				"A latex shape such as n, it, sl, sc"));
	all_params_[1]["formatcom"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_[1]["frame"] =
		ListingsParam("", false, ONEOF,
		  "none\nleftline\ntopline\nbottomline\nlines\nsingle",
		  frame_hint_mint);
	all_params_[1]["framerule"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[1]["framesep"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[1]["funcnamehighlighting"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["gobble"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_[1]["highlightcolor"] =
		ListingsParam("", false, ALL, "", color_hint_mint);
	all_params_[1]["highlightlines"] =
		ListingsParam("", false, ALL, "", _(
				"A range of lines such as {1,3-4}"));
	all_params_[1]["keywordcase"] =
		ListingsParam("", false, ONEOF,
				"lower\nupper\ncapitalize", empty_hint);
	all_params_[1]["labelposition"] =
		ListingsParam("", false, ONEOF,
			"none\ntopline\nbottomline\nall", empty_hint);
	all_params_[1]["language"] =
		ListingsParam("", false, ALL, "", _(
		"Enter one of the supported languages. However, if you "
		"are defining a listing inset, it is better using the  "
		"language combo box, unless you need to enter a language not "
		"offered there, otherwise the combo box will be disabled."));
	all_params_[1]["lastline"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_[1]["linenos"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["numberfirstline"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["numbers"] =
		ListingsParam("", false, ONEOF,
				"left\nright\nboth\nnone", empty_hint);
	all_params_[1]["mathescape"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["numberblanklines"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["numbersep"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[1]["obeytabs"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["outencoding"] =
		ListingsParam("", false, ALL, "", _(
		  "File encoding used by Pygments for highlighting"));
	all_params_[1]["python3"] =
		ListingsParam("", false, TRUEFALSE, "", _(
					"Apply Python 3 highlighting"));
	all_params_[1]["resetmargins"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["rulecolor"] =
		ListingsParam("", false, ALL, "", color_hint_mint);
	all_params_[1]["samepage"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["showspaces"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["showtabs"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["space"] =
		ListingsParam("", false, ALL, "", _(
				"A macro. Default: \\textvisiblespace"));
	all_params_[1]["spacecolor"] =
		ListingsParam("", false, ALL, "", color_hint_mint);
	all_params_[1]["startinline"] =
		ListingsParam("", false, TRUEFALSE, "", _("For PHP only"));
	all_params_[1]["style"] =
		ListingsParam("", false, ALL, "", _(
					"The style used by Pygments"));
	all_params_[1]["stepnumber"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_[1]["stepnumberfromfirst"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["stepnumberoffsetvalues"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["stripall"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["stripnl"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_[1]["tab"] =
		ListingsParam("", false, ALL, "", _(
				"A macro to redefine visible tabs"));
	all_params_[1]["tabcolor"] =
		ListingsParam("", false, ALL, "", color_hint_mint);
	all_params_[1]["tabsize"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_[1]["texcl"] =
		ListingsParam("", false, TRUEFALSE, "", _(
				"Enables latex code in comments"));
	all_params_[1]["texcomments"] =
		ListingsParam("", false, TRUEFALSE, "", _(
				"Enables latex code in comments"));
	all_params_[1]["xleftmargin"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_[1]["xrightmargin"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
}


docstring ParValidator::validate(string const & name,
		string const & par) const
{
	int p = InsetListingsParams::package();

	if (name.empty())
		return _("Invalid (empty) listing parameter name.");

	if (name[0] == '?') {
		string suffix = trim(string(name, 1));
		string param_names;
		ListingsParams::const_iterator it = all_params_[p].begin();
		ListingsParams::const_iterator end = all_params_[p].end();
		for (; it != end; ++it) {
			if (suffix.empty() || contains(it->first, suffix)) {
				if (!param_names.empty())
					param_names += ", ";
				param_names += it->first;
			}
		}
		if (suffix.empty())
			return bformat(
					_("Available listing parameters are %1$s"), from_ascii(param_names));
		else
			return bformat(
					_("Available listings parameters containing string \"%1$s\" are %2$s"),
						from_utf8(suffix), from_utf8(param_names));
	}

	// locate name in parameter table
	ListingsParams::const_iterator it = all_params_[p].find(name);
	if (it != all_params_[p].end()) {
		docstring msg = it->second.validate(par);
		if (msg.empty())
			return msg;
		else
			return bformat(_("Parameter %1$s: "), from_utf8(name)) + msg;
	} else {
		// otherwise, produce a meaningful error message.
		string matching_names;
		ListingsParams::const_iterator end = all_params_[p].end();
		for (it = all_params_[p].begin(); it != end; ++it) {
			if (prefixIs(it->first, name)) {
				if (!matching_names.empty())
					matching_names += ", ";
				matching_names += it->first;
			}
		}
		if (matching_names.empty())
			return bformat(_("Unknown listing parameter name: %1$s"),
								from_utf8(name));
		else
			return bformat(_("Parameters starting with '%1$s': %2$s"),
								from_utf8(name), from_utf8(matching_names));
	}
}


bool ParValidator::onoff(string const & name) const
{
	int p = InsetListingsParams::package();

	// locate name in parameter table
	ListingsParams::const_iterator it = all_params_[p].find(name);
	if (it != all_params_[p].end())
		return it->second.onoff_;
	else
		return false;
}

} // namespace

// define a global ParValidator
ParValidator * par_validator = 0;

// The package to be used by the global ParValidator
// (0 for listings, 1 for minted)
int InsetListingsParams::package_ = 0;

InsetListingsParams::InsetListingsParams()
	: inline_(false), params_(), status_(InsetCollapsible::Open)
{
}


InsetListingsParams::InsetListingsParams(string const & par, bool in,
		InsetCollapsible::CollapseStatus s)
	: inline_(in), params_(), status_(s)
{
	// this will activate parameter validation.
	fromEncodedString(par);
}


void InsetListingsParams::write(ostream & os) const
{
	if (inline_)
		os << "true ";
	else
		os << "false ";
	os << status_ << " \""	<< encodedString() << "\"";
}


void InsetListingsParams::read(Lexer & lex)
{
	lex >> inline_;
	int s = InsetCollapsible::Collapsed;
	lex >> s;
	status_ = static_cast<InsetCollapsible::CollapseStatus>(s);
	string par;
	lex >> par;
	fromEncodedString(par);
}


string InsetListingsParams::params(string const & sep) const
{
	string par;
	keyValuePair::const_iterator it = params_.begin();
	for (; it != params_.end(); ++it) {
		if (!par.empty())
			par += sep;
		// key=value,key=value1 is stored in params_ as key=value,key_=value1.
		if (it->second.empty())
			par += rtrim(it->first, "_");
		else
			par += rtrim(it->first, "_") + '=' + it->second;
	}
	return par;
}


bool InsetListingsParams::hasParam(string const & key) const
{
	keyValuePair::const_iterator it = params_.begin();
	for (; it != params_.end(); ++it) {
		if (it->first == key)
			return true;
	}
	return false;
}


string InsetListingsParams::getValue(string const & key) const
{
	keyValuePair::const_iterator it = params_.begin();
	for (; it != params_.end(); ++it) {
		if (it->first == key)
			return it->second;
	}
	return string();
}


void InsetListingsParams::addParam(string const & key,
		string const & val, bool replace)
{
	if (key.empty())
		return;

	bool const is_minted_language = minted() && key == "language";
	string const value = (is_minted_language && !val.empty())
				? languageonly(val) : val;
	// duplicate parameters!
	string keyname = key;
	if (!replace && hasParam(key))
		// key=value,key=value1 is allowed in listings
		// use key_, key__, key___ etc to avoid name conflict
		while (hasParam(keyname += '_')) { }
	// check onoff flag
	// onoff parameter with value false
	if (!par_validator)
		par_validator = new ParValidator;
	if (par_validator->onoff(key) && (value == "false" || value == "{false}"))
		params_.push_back(make_pair(keyname, string()));
	// if the parameter is surrounded with {}, good
	else if (prefixIs(value, "{") && suffixIs(value, "}"))
		params_.push_back(make_pair(keyname, value));
	// otherwise, check if {} is needed. Add {} to all values with
	// non-ascii/number characters, just to be safe
	else {
		bool has_special_char = false;
		for (size_t i = 0; i < value.size(); ++i)
			if (!isAlnumASCII(value[i])) {
				has_special_char = true;
				break;
			}
		if (has_special_char && !is_minted_language)
			params_.push_back(make_pair(keyname, "{" + value + "}"));
		else
			params_.push_back(make_pair(keyname, value));
	}
}


void InsetListingsParams::addParams(string const & par)
{
	string key;
	string value;
	bool isValue = false;
	int braces = 0;
	for (size_t i = 0; i < par.size(); ++i) {
		// end of par
		if (par[i] == '\n') {
			addParam(trim(key), trim(value));
			key = string();
			value = string();
			isValue = false;
			continue;
		} else if (par[i] == ',' && braces == 0) {
			addParam(trim(key), trim(value));
			key = string();
			value = string();
			isValue = false;
			continue;
		} else if (par[i] == '=' && braces == 0) {
			isValue = true;
			continue;
		} else if (par[i] == '{' && i > 0 && par[i-1] != '\\')
			// don't count a brace in first position
			++braces;
		else if (par[i] == '}' && i != par.size() - 1
		         && (i == 0 || (i > 0 && par[i-1] != '\\')))
			--braces;

		if (isValue)
			value += par[i];
		else
			key += par[i];
	}
	if (!trim(key).empty())
		addParam(trim(key), trim(value));
}


void InsetListingsParams::setParams(string const & par)
{
	params_.clear();
	addParams(par);
}


string InsetListingsParams::encodedString() const
{
	// Encode string!
	// '"' is handled differently because it will
	// terminate a lyx token.
	string par = params();
	// '"' is now &quot;  ==> '"' is now &amp;quot;
	par = subst(par, "&", "&amp;");
	// '"' is now &amp;quot; ==> '&quot;' is now &amp;quot;
	par = subst(par, "\"", "&quot;");
	return par;
}


string InsetListingsParams::separatedParams(bool keepComma) const
{
	if (keepComma)
		return params(",\n");
	else
		return params("\n");
}


void InsetListingsParams::fromEncodedString(string const & in)
{
	// Decode string! Reversal of encodedString
	string par = in;
	// '&quot;' is now &amp;quot; ==> '"' is now &amp;quot;
	par = subst(par, "&quot;", "\"");
	//  '"' is now &amp;quot; ==> '"' is now &quot;
	par = subst(par, "&amp;", "&");
	setParams(par);
}


bool InsetListingsParams::isFloat() const
{
	return hasParam("float");
}


string InsetListingsParams::getParamValue(string const & param) const
{
	string par = getValue(param);
	if (prefixIs(par, "{") && suffixIs(par, "}"))
		return par.substr(1, par.size() - 2);
	else
		return par;
}


docstring InsetListingsParams::validate() const
{
	docstring msg;
	if (!par_validator)
		par_validator = new ParValidator;
	// return msg for first key=value pair which is incomplete or has an error
	keyValuePair::const_iterator it = params_.begin();
	for (; it != params_.end(); ++it) {
		// key trimmed
		msg = par_validator->validate(rtrim(it->first, "_"), it->second);
		if (!msg.empty())
			return msg;
	}
	return msg;
}

} // namespace lyx
