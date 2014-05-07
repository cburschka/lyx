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
	/// Validate a paramater.
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
		return docstring();
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
	"[R/3 4.6C]ABAP\n[R/3 6.10]ABAP\nACSL\nAda\n[2005]Ada\n[83]Ada\n"
	"[95]Ada\nALGOL\n[60]ALGOL\n[68]ALGOL\nAssembler\n"
	"[Motorola68k]Assembler\n[x86masm]Assembler\nAwk\n[gnu]Awk\n[POSIX]Awk\n"
	"bash\nBasic\n[Visual]Basic\nC\n[ANSI]C\n[Handel]C\n[Objective]C\n"
	"[Sharp]C\nC++\n[ANSI]C++\n[GNU]C++\n[ISO]C++\n[Visual]C++\nCaml\n"
	"[light]Caml\n[Objective]Caml\nClean\nCobol\n[1974]Cobol\n[1985]Cobol\n"
	"[ibm]Cobol\nComal 80\ncommand.com\n[WinXP]command.com\nComsol\ncsh\n"
	"Delphi\nEiffel\nElan\nerlang\nEuphoria\nFortran\n[77]Fortran\n[90]Fortran\n"
	"[95]Fortran\nGCL\nGnuplot\nHaskell\nHTML\nIDL\n[CORBA]IDL\ninform\n"
	"Java\n[AspectJ]Java\nJVMIS\nksh\nLingo\nLisp\n[Auto]Lisp\nLogo\n"
	"make\n[gnu]make\nMathematica\n[1.0]Mathematica\n[3.0]Mathematica\n"
	"[5.2]Mathematica\nMatlab\nMercury\nMetaPost\nMiranda\nMizar\nML\n"
	"Modula-2\nMuPAD\nNASTRAN\nOberon-2\nOCL\n[decorative]OCL\n[OMG]OCL\n"
	"Octave\nOz\nPascal\n[Borland6]Pascal\n[Standard]Pascal\n[XSC]Pascal\n"
	"Perl\nPHP\nPL/I\nPlasm\nPostScript\nPOV\nProlog\nPromela\nPSTricks\n"
	"Python\nR\nReduce\nRexx\nRSL\nRuby\nS\n[PLUS]S\nSAS\nScilab\nsh\n"
	"SHELXL\nSimula\n[67]Simula\n[CII]Simula\n[DEC]Simula\n[IBM]Simula\n"
	"SPARQL\nSQL\ntcl\n[tk]tcl\nTeX\n[AlLaTeX]TeX\n[common]TeX\n[LaTeX]TeX\n"
	"[plain]TeX\n[primitive]TeX\nVBScript\nVerilog\nVHDL\n[AMS]VHDL\nVRML\n"
	"[97]VRML\nXML\nXSLT";


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
	ListingsParams all_params_;
};


ParValidator::ParValidator()
{
	docstring const empty_hint;
	docstring const style_hint = _("Use \\footnotesize, \\small, \\itshape, "
		"\\ttfamily or something like that");
	docstring const frame_hint = _("none, leftline, topline, bottomline, lines, "
		"single, shadowbox or subset of trblTRBL");
	docstring const frameround_hint = _("Enter four letters (either t = round "
		"or f = square) for top right, bottom "
		"right, bottom left and top left corner.");
	docstring const color_hint = _("Enter something like \\color{white}");

	/// options copied from page 26 of listings manual
	// FIXME: add default parameters ... (which is not used now)
	all_params_["float"] =
		ListingsParam("false", true, SUBSETOF, "*tbph", empty_hint);
	all_params_["floatplacement"] =
		ListingsParam("tbp", false, SUBSETOF, "tbp", empty_hint);
	all_params_["aboveskip"] =
		ListingsParam("\\medskipamount", false, LENGTH, "", empty_hint);
	all_params_["belowskip"] =
		ListingsParam("\\medskipamount", false, LENGTH, "", empty_hint);
	all_params_["lineskip"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["boxpos"] =
		ListingsParam("", false, SUBSETOF, "bct", empty_hint);
	all_params_["print"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["firstline"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_["lastline"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_["linerange"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["showlines"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["emptylines"] =
		ListingsParam("", false, ALL, "", _(
		"Expect a number with an optional * before it"));
	all_params_["gobble"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_["style"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["language"] =
		ListingsParam("", false, ONEOF, allowed_languages, empty_hint);
	all_params_["alsolanguage"] =
		ListingsParam("", false, ONEOF, allowed_languages, empty_hint);
	all_params_["defaultdialect"] =
		ListingsParam("", false, ONEOF, allowed_languages, empty_hint);
	all_params_["printpod"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["usekeywordsintag"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["tagstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_["markfirstintag"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_["makemacrouse"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["basicstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_["identifierstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_["commentstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_["stringstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_["keywordstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_["ndkeywordstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_["classoffset"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_["texcsstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_["directivestyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_["emph"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["moreemph"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["deleteemph"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["emphstyle"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["delim"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["moredelim"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["deletedelim"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["extendedchars"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["inputencoding"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["upquote"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["tabsize"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_["showtabs"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["tab"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["showspaces"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["showstringspaces"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["formfeed"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["numbers"] =
		ListingsParam("", false, ONEOF, "none\nleft\nright", empty_hint);
	all_params_["stepnumber"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
	all_params_["numberfirstline"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["numberstyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_["numbersep"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["numberblanklines"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["firstnumber"] =
		ListingsParam("", false, ALL, "", _("auto, last or a number"));
	all_params_["name"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["thelstnumber"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["title"] =
		ListingsParam("", false, ALL, "", empty_hint);
	// this option is not handled in the parameter box
	all_params_["caption"] =
		ListingsParam("", false, ALL, "", _(
		"This parameter should not be entered here. Please use the caption "
		"edit box (when using the child document dialog) or "
		"menu Insert->Caption (when defining a listing inset)"));
	// this option is not handled in the parameter box
	all_params_["label"] =
		ListingsParam("", false, ALL, "",_(
		"This parameter should not be entered here. Please use the label "
		"edit box (when using the child document dialog) or "
		"menu Insert->Label (when defining a listing inset)"));
	all_params_["nolol"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["captionpos"] =
		ListingsParam("", false, SUBSETOF, "tb", empty_hint);
	all_params_["abovecaptionskip"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["belowcaptionskip"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["linewidth"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["xleftmargin"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["xrightmargin"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["resetmargins"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["breaklines"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["breakatwhitespace"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["prebreak"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["postbreak"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["breakindent"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["breakautoindent"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["frame"] =
		ListingsParam("", false, ALL, "", frame_hint);
	all_params_["frameround"] =
		ListingsParam("", false, SUBSETOF, "tf", frameround_hint);
	all_params_["framesep"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["rulesep"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["framerule"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["framexleftmargin"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["framexrightmargin"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["framextopmargin"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["framexbottommargin"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["backgroundcolor"] =
		ListingsParam("", false, ALL, "", color_hint );
	all_params_["rulecolor"] =
		ListingsParam("", false, ALL, "", color_hint );
	all_params_["fillcolor"] =
		ListingsParam("", false, ALL, "", color_hint );
	all_params_["rulesepcolor"] =
		ListingsParam("", false, ALL, "", color_hint );
	all_params_["frameshape"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["index"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["moreindex"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["deleteindex"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["indexstyle"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["columns"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["flexiblecolumns"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["keepspaces"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["basewidth"] =
		ListingsParam("", false, LENGTH, "", empty_hint);
	all_params_["fontadjust"] =
		ListingsParam("", true, TRUEFALSE, "", empty_hint);
	all_params_["texcl"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["mathescape"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["escapechar"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["escapeinside"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["escapebegin"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["escapeend"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["fancyvrb"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["fvcmdparams"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["morefvcmdparams"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["keywordsprefix"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["keywords"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["morekeywords"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["deletekeywords"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["ndkeywords"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["morendkeywords"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["deletendkeywords"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["texcs"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["moretexcs"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["deletetexcs"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["directives"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["moredirectives"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["deletedirectives"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["sensitive"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["alsoletter"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["alsodigit"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["alsoother"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["otherkeywords"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["tag"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["string"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["morestring"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["deletestring"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["comment"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["morecomment"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["deletecomment"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["keywordcomment"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["morekeywordcomment"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["deletekeywordcomment"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["keywordcommentsemicolon"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["podcomment"] =
		ListingsParam("", false, ALL, "", empty_hint);
	// the following are experimental listings features
	all_params_["procnamekeys"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["moreprocnamekeys"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["deleteprocnamekeys"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["procnamestyle"] =
		ListingsParam("", false, ALL, "", style_hint);
	all_params_["indexprocnames"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["hyperref"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["morehyperref"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["deletehyperref"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["hyperanchor"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["hyperlink"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["literate"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["lgrindef"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["rangebeginprefix"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["rangebeginsuffix"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["rangeendprefix"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["rangeendsuffix"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["rangeprefix"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["rangesuffix"] =
		ListingsParam("", false, ALL, "", empty_hint);
	all_params_["includerangemarker"] =
		ListingsParam("", false, TRUEFALSE, "", empty_hint);
	all_params_["multicols"] =
		ListingsParam("", false, INTEGER, "", empty_hint);
}


docstring ParValidator::validate(string const & name,
		string const & par) const
{
	if (name.empty())
		return _("Invalid (empty) listing parameter name.");

	if (name[0] == '?') {
		string suffix = trim(string(name, 1));
		string param_names;
		ListingsParams::const_iterator it = all_params_.begin();
		ListingsParams::const_iterator end = all_params_.end();
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
	ListingsParams::const_iterator it = all_params_.find(name);
	if (it != all_params_.end()) {
		docstring msg = it->second.validate(par);
		if (msg.empty())
			return msg;
		else
			return bformat(_("Parameter %1$s: "), from_utf8(name)) + msg;
	} else {
		// otherwise, produce a meaningful error message.
		string matching_names;
		ListingsParams::const_iterator end = all_params_.end();
		for (it = all_params_.begin(); it != end; ++it) {
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
	return docstring();
}


bool ParValidator::onoff(string const & name) const
{
	// locate name in parameter table
	ListingsParams::const_iterator it = all_params_.find(name);
	if (it != all_params_.end())
		return it->second.onoff_;
	else
		return false;
}

} // namespace anon.

// define a global ParValidator
ParValidator * par_validator = 0;

InsetListingsParams::InsetListingsParams()
	: inline_(false), params_(), status_(InsetCollapsable::Open)
{
}


InsetListingsParams::InsetListingsParams(string const & par, bool in,
		InsetCollapsable::CollapseStatus s)
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
	int s = InsetCollapsable::Collapsed;
	lex >> s;
	status_ = static_cast<InsetCollapsable::CollapseStatus>(s);
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
		string const & value, bool replace)
{
	if (key.empty())
		return;

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
		if (has_special_char)
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
