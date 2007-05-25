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

#include "Lexer.h"
#include "InsetListingsParams.h"

#include "gettext.h"
#include "Length.h"

#include <sstream>
#include <boost/assert.hpp>

#include "support/lstrings.h"
#include "support/textutils.h"
#include "support/convert.h"

using std::map;
using std::vector;
using std::ostream;
using std::string;
using std::exception;
using lyx::support::bformat;
using lyx::support::trim;
using lyx::support::subst;
using lyx::support::isStrInt;
using lyx::support::prefixIs;
using lyx::support::suffixIs;
using lyx::support::getVectorFromString;
using lyx::isAlphaASCII;
using lyx::isDigit;

namespace lyx
{

enum param_type {
	ALL,  // accept all
	TRUEFALSE, // accept 'true' or 'false'
	INTEGER, // accept an integer
	LENGTH,  // accept a latex length
	ONEOF,  // accept one of a few values
	SUBSETOF, // accept a string composed of given characters
};


/** Information about each parameter
 */
struct listings_param_info {
	/// name of the parameter
	char const * name;
	/// default value
	char const * value;
	// for option with value "true", "false",
	// if onoff is true,
	//   "true":  option
	//   "false": 
	//   "other": option="other"
	// onoff is false,
	//   "true":  option=true
	//   "false": option=false
	bool onoff;
	/// validator type
	// ALL:
	// TRUEFALSE:
	// INTEGER:
	// LENGTH:
	//     info is ignored.
	// ONEOF
	//     info is a \n separated string with allowed values
	// SUBSETOF
	//     info is a string from which par is composed of
	//     (e.g. floatplacement can be one or more of tbph)
	param_type type;
	/// parameter info, meaning depending on parameter type
	char const * info;
	/// a help message that is displayed in the gui
	char const * hint;
};



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
	"Delphi\nEiffel\nElan\nEuphoria\nFortran\n[77]Fortran\n[90]Fortran\n"
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

char const * style_hint = N_("Use \\footnotesize, \\small, \\itshape, \\ttfamily or something like that");
char const * frame_hint = N_("none, leftline, topline, bottomline, lines, single, shadowbox or subset of trblTRBL");
char const * frameround_hint =
	N_("Enter four letters (either t = round or f = square) for top right, bottom right, bottom left and top left corner.");
char const * color_hint = N_("Enter something like \\color{white}");

/// options copied from page 26 of listings manual
// FIXME: add default parameters ... (which is not used now)
listings_param_info const listings_param_table[] = {
	{ "float", "false", true,  SUBSETOF, "tbph", "" },
	{ "floatplacement", "tbp", false, SUBSETOF, "tbph", "" },
	{ "aboveskip", "\\medskipamount", false, LENGTH, "", "" },
	{ "belowskip", "\\medskipamount", false, LENGTH, "", "" },
	{ "lineskip", "", false, LENGTH, "", "" },
	{ "boxpos", "", false, SUBSETOF, "bct", "" },
	{ "print", "", false, TRUEFALSE, "", "" },
	{ "firstline", "", false, INTEGER, "", "" },
	{ "lastline", "", false, INTEGER, "", "" },
	{ "showlines", "", false, TRUEFALSE, "", "" },
	{ "emptylines", "", false, ALL, "", N_("Expect a number with an optional * before it") },
	{ "gobble", "", false, INTEGER, "", "" },
	{ "style", "", false, ALL, "", "" },
	{ "language", "", false, ONEOF, allowed_languages, "" },
	{ "alsolanguage", "", false, ONEOF, allowed_languages, "" },
	{ "defaultdialect", "", false, ONEOF, allowed_languages, "" },
	{ "printpod", "", false, TRUEFALSE, "", "" },
	{ "usekeywordsintag", "", false, TRUEFALSE, "", "" },
	{ "tagstyle", "", false, ALL, "", style_hint },
	{ "markfirstintag", "", false, ALL, "", style_hint },
	{ "makemacrouse", "", false, TRUEFALSE, "", "" },
	{ "basicstyle", "", false, ALL, "", style_hint },
	{ "identifierstyle", "", false, ALL, "", style_hint },
	{ "commentstyle", "", false, ALL, "", style_hint },
	{ "stringstyle", "", false, ALL, "", style_hint },
	{ "keywordstyle", "", false, ALL, "", style_hint },
	{ "ndkeywordstyle", "", false, ALL, "", style_hint },
	{ "classoffset", "", false, INTEGER, "", "" },
	{ "texcsstyle", "", false, ALL, "", style_hint },
	{ "directivestyle", "", false, ALL, "", style_hint },
	{ "emph", "", false, ALL, "", "" },
	{ "moreemph", "", false, ALL, "", "" },
	{ "deleteemph", "", false, ALL, "", "" },
	{ "emphstyle", "", false, ALL, "", "" },
	{ "delim", "", false, ALL, "", "" },
	{ "moredelim", "", false, ALL, "", "" },
	{ "deletedelim", "", false, ALL, "", "" },
	{ "extendedchars", "", false, TRUEFALSE, "", "" },
	{ "inputencoding", "", false, ALL, "", "" },
	{ "upquote", "", false, TRUEFALSE, "", "" },
	{ "tabsize", "", false, INTEGER, "", "" },
	{ "showtabs", "", false, ALL, "", "" },
	{ "tab", "", false, ALL, "", "" },
	{ "showspaces", "", false, TRUEFALSE, "", "" },
	{ "showstringspaces", "", false, TRUEFALSE, "", "" },
	{ "formfeed", "", false, ALL, "", "" },
	{ "numbers", "", false, ONEOF, "none\nleft\nright", "" },
	{ "stepnumber", "", false, INTEGER, "", "" },
	{ "numberfirstline", "", false, TRUEFALSE, "", "" },
	{ "numberstyle", "", false, ALL, "", style_hint },
	{ "numbersep", "", false, LENGTH, "", "" },
	{ "numberblanklines", "", false, ALL, "", "" },
	{ "firstnumber", "", false, ALL, "", N_("auto, last or a number") },
	{ "name", "", false, ALL, "", "" },
	{ "thelstnumber", "", false, ALL, "", "" },
	{ "title", "", false, ALL, "", "" },
	// this option is not handled in the parameter box
	{ "caption", "", false, ALL, "", N_("This parameter should not be entered here. "
		"Please use caption editbox (Include dialog) or insert->caption (listings inset)") },
	// this option is not handled in the parameter box
	{ "label", "", false, ALL, "", N_("This parameter should not be entered here. "
		"Please use label editbox (Include dialog) or insert->caption (listings inset)") },
	{ "nolol", "", false, TRUEFALSE, "", "" },
	{ "captionpos", "", false, SUBSETOF, "tb", "" },
	{ "abovecaptionskip", "", false, LENGTH, "", "" },
	{ "belowcaptionskip", "", false, LENGTH, "", "" },
	{ "linewidth", "", false, LENGTH, "", "" },
	{ "xleftmargin", "", false, LENGTH, "", "" },
	{ "xrightmargin", "", false, LENGTH, "", "" },
	{ "resetmargins", "", false, TRUEFALSE, "", "" },
	{ "breaklines", "", false, TRUEFALSE, "", "" },
	{ "prebreak", "", false, ALL, "", "" },
	{ "postbreak", "", false, ALL, "", "" },
	{ "breakindent", "", false, LENGTH, "", "" },
	{ "breakautoindent", "", false, TRUEFALSE, "", "" },
	{ "frame", "", false, ALL, "", frame_hint },
	{ "frameround", "", false, SUBSETOF, "tf", frameround_hint },
	{ "framesep", "", false, LENGTH, "", "" },
	{ "rulesep", "", false, LENGTH, "", "" },
	{ "framerule", "", false, LENGTH, "", "" },
	{ "framexleftmargin", "", false, LENGTH, "", "" },
	{ "framexrightmargin", "", false, LENGTH, "", "" },
	{ "framextopmargin", "", false, LENGTH, "", "" },
	{ "framexbottommargin", "", false, LENGTH, "", "" },
	{ "backgroundcolor", "", false, ALL, "", color_hint },
	{ "rulecolor", "", false, ALL, "", color_hint },
	{ "fillcolor", "", false, ALL, "", color_hint },
	{ "rulesepcolor", "", false, ALL, "", color_hint },
	{ "frameshape", "", false, ALL, "", "" },
	{ "index", "", false, ALL, "", "" },
	{ "moreindex", "", false, ALL, "", "" },
	{ "deleteindex", "", false, ALL, "", "" },
	{ "indexstyle", "", false, ALL, "", "" },
	{ "columns", "", false, ALL, "", "" },
	{ "flexiblecolumns", "", false, ALL, "", "" },
	{ "keepspaces", "", false, TRUEFALSE, "", "" },
	{ "basewidth", "", false, LENGTH, "", "" },
	{ "fontadjust", "", true, TRUEFALSE, "", "" },
	{ "texcl", "", false, TRUEFALSE, "", "" },
	{ "mathescape", "", false, TRUEFALSE, "", "" },
	{ "escapechar", "", false, ALL, "", "" },
	{ "escapeinside", "", false, ALL, "", "" },
	{ "escepeinside", "", false, ALL, "", "" },
	{ "escepebegin", "", false, ALL, "", "" },
	{ "escepeend", "", false, ALL, "", "" },
	{ "fancyvrb", "", false, TRUEFALSE, "", "" },
	{ "fvcmdparams", "", false, ALL, "", "" },
	{ "morefvcmdparams", "", false, ALL, "", "" },
	{ "keywordsprefix", "", false, ALL, "", "" },
	{ "keywords", "", false, ALL, "", "" },
	{ "morekeywords", "", false, ALL, "", "" },
	{ "deletekeywords", "", false, ALL, "", "" },
	{ "ndkeywords", "", false, ALL, "", "" },
	{ "morendkeywords", "", false, ALL, "", "" },
	{ "deletendkeywords", "", false, ALL, "", "" },
	{ "texcs", "", false, ALL, "", "" },
	{ "moretexcs", "", false, ALL, "", "" },
	{ "deletetexcs", "", false, ALL, "", "" },
	{ "directives", "", false, ALL, "", "" },
	{ "moredirectives", "", false, ALL, "", "" },
	{ "deletedirectives", "", false, ALL, "", "" },
	{ "sensitive", "", false, ALL, "", "" },
	{ "alsoletter", "", false, ALL, "", "" },
	{ "alsodigit", "", false, ALL, "", "" },
	{ "alsoother", "", false, ALL, "", "" },
	{ "otherkeywords", "", false, ALL, "", "" },
	{ "tag", "", false, ALL, "", "" },
	{ "string", "", false, ALL, "", "" },
	{ "morestring", "", false, ALL, "", "" },
	{ "deletestring", "", false, ALL, "", "" },
	{ "comment", "", false, ALL, "", "" },
	{ "morecomment", "", false, ALL, "", "" },
	{ "deletecomment", "", false, ALL, "", "" },
	{ "keywordcomment", "", false, ALL, "", "" },
	{ "morekeywordcomment", "", false, ALL, "", "" },
	{ "deletekeywordcomment", "", false, ALL, "", "" },
	{ "keywordcommentsemicolon", "", false, ALL, "", "" },
	{ "podcomment", "", false, ALL, "", "" },
	{ "", "", false, ALL, "", ""}
};


class parValidator
{
public:
	parValidator(string const & name);

	/// validate given parameter
	/// invalidParam will be thrown if invalid 
	/// parameter is found.
	void validate(std::string const & par) const;

private:
	/// parameter name
	string const & name;
	///
	listings_param_info const * info;
};


parValidator::parValidator(string const & n)
	: name(n), info(0)
{
	if (name.empty())
		throw invalidParam(_("Invalid (empty) listings param name."));
	else if (name == "?") {
		string pars;
		size_t idx = 0;
		while (listings_param_table[idx].name != string()) {
			if (!pars.empty())
				pars += ", ";
			pars += listings_param_table[idx].name;
			++idx;
		}
		throw invalidParam(bformat(
			_("Available listings parameters are %1$s"), from_utf8(pars)));
	}
	// locate name in parameter table
	size_t idx = 0;
	while (listings_param_table[idx].name != name && listings_param_table[idx].name != string())
		++idx;
	// found the name
	if (listings_param_table[idx].name != "") {
		info = &listings_param_table[idx];
		return;
	}
	// otherwise, produce a meaningful error message.
	string matching_names;
	for (size_t i = 0; i < idx; ++i) {
		string n(listings_param_table[i].name);
		if (prefixIs(n, name)) {
			if (matching_names.empty())
				matching_names += n;
			else
				matching_names += ", " + n;
		}
	}
	if (matching_names.empty())
		throw invalidParam(bformat(_("Unknown listings param name: %1$s"),
						    from_utf8(name)));
	else
		throw invalidParam(bformat(_("Parameters starting with '%1$s': %2$s"),
						    from_utf8(name), from_utf8(matching_names)));
}


void parValidator::validate(std::string const & par) const
{
	bool unclosed = false;
	string par2 = par;
	// braces are allowed
	if (prefixIs(par, "{") && suffixIs(par, "}"))
		par2 = par.substr(1, par.size() - 2);
	else if (prefixIs(par, "{")) {
		par2 = par.substr(1);
		unclosed = true;
	}
	
		
	switch (info->type) {
	case ALL:
		if (par2.empty() && !info->onoff) {
			if (info->hint != "")
				throw invalidParam(from_utf8(info->hint));
			else
				throw invalidParam(_("A value is expected"));
		}
		if (unclosed)
				throw invalidParam(_("Unbalanced braces!"));
		return;
	case TRUEFALSE: {
		if (par2.empty() && !info->onoff) {
			if (info->hint != "")
				throw invalidParam(from_utf8(info->hint));
			else
				throw invalidParam(_("Please specify true or false"));
		}
		if (par2 != "true" && par2 != "false")
			throw invalidParam(bformat(_("Only true or false is allowed for parameter %1$s"),
						   from_utf8(name)));
		if (unclosed)
				throw invalidParam(_("Unbalanced braces!"));
		return;
	}
	case INTEGER: {
		if (!isStrInt(par2)) {
			if (info->hint != "")
				throw invalidParam(from_utf8(info->hint));
			else
				throw invalidParam(_("Please specify an integer value"));
		}
		if (convert<int>(par2) == 0 && par2[0] != '0')
			throw invalidParam(bformat(_("An integer is expected for parameter %1$s"),
						   from_utf8(name)));
		if (unclosed)
				throw invalidParam(_("Unbalanced braces!"));
		return;
	}
	case LENGTH: {
		if (par2.empty() && !info->onoff) {
			if (info->hint != "")
				throw invalidParam(from_utf8(info->hint));
			else
				throw invalidParam(_("Please specify a latex length expression"));
		}
		if (!isValidLength(par2))
			throw invalidParam(bformat(_("Invalid latex length expression for parameter %1$s"),
						   from_utf8(name)));
		if (unclosed)
				throw invalidParam(_("Unbalanced braces!"));
		return;
	}
	case ONEOF: {
		if (par2.empty() && !info->onoff) {
			if (info->hint != "")
				throw invalidParam(from_utf8(info->hint));
			else
				throw invalidParam(bformat(_("Please specify one of %1$s"),
							   from_utf8(string(info->info))));
		}
		// break value to allowed strings
		vector<string> lists;
		string v;
		for (size_t i = 0; info->info[i] != '\0'; ++i) {
			if (info->info[i] == '\n') {
				lists.push_back(v);
				v = string();
			} else
				v += info->info[i];
		}
		if (!v.empty())
			lists.push_back(v);

		// good, find the string
		if (std::find(lists.begin(), lists.end(), par2) != lists.end()) {
			if (unclosed)
				throw invalidParam(_("Unbalanced braces!"));
			return;
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
			throw invalidParam(bformat(_("Try one of %1$s"),
						   from_utf8(string(info->info))));
		else
			throw invalidParam(bformat(_("I guess you mean %1$s"),
						   from_utf8(matching_names)));
		return;
	}
	case SUBSETOF: {
		if (par2.empty() && !info->onoff) {
			if (info->hint != "")
				throw invalidParam(from_utf8(info->hint));
			else
				throw invalidParam(bformat(_("Please specify one or more of '%1$s'"),
							   from_utf8(string(info->info))));
		}
		for (size_t i = 0; i < par2.size(); ++i)
			if (string(info->info).find(par2[i], 0) == string::npos)
				throw invalidParam(
					bformat(_("Parameter %1$s should be composed of one or more of %2$s"),
						from_utf8(name), from_utf8(info->info)));
		if (unclosed)
				throw invalidParam(_("Unbalanced braces!"));
		return;
	}
	}
}


InsetListingsParams::InsetListingsParams() :
	inline_(false), params_(), status_(InsetCollapsable::Open)
{
}


InsetListingsParams::InsetListingsParams(string const & par, bool in, InsetCollapsable::CollapseStatus s)
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
	int s;
	lex >> s;
	if (lex)
		status_ = static_cast<InsetCollapsable::CollapseStatus>(s);
	string par;
	lex >> par;
	fromEncodedString(par);
}


string InsetListingsParams::params(string const & sep) const
{
	string par;
	for (map<string, string>::const_iterator it = params_.begin();
		it != params_.end(); ++it) {
		if (!par.empty())
			par += sep;
		if (it->second.empty())
			par += it->first;
		else
			par += it->first + '=' + it->second;
	}
	return par;
}


void InsetListingsParams::addParam(string const & key, string const & value)
{	
	if (key.empty())
		return;
	// exception may be thown.
	parValidator(key).validate(value);
	// duplicate parameters!
	if (params_.find(key) != params_.end())
		throw invalidParam(bformat(_("Parameter %1$s has already been defined"),
					  from_utf8(key)));
	// check onoff flag
	size_t idx = 0;
	while (listings_param_table[idx].name != key)
		++idx;
	BOOST_ASSERT(listings_param_table[idx].name == key);
	// onoff parameter with value false
	if (listings_param_table[idx].onoff && (value == "false" || value == "{false}"))
		params_[key] = string();
	// if the parameter is surrounded with {}, good
	else if (prefixIs(value, "{") && suffixIs(value, "}"))
		params_[key] = value;
	// otherwise, check if {} is needed. Add {} to all values with 
	// non-ascii/number characters, just to be safe
	else {
		bool has_special_char = false;
		for (size_t i = 0; i < value.size(); ++i)
			if (!isAlphaASCII(value[i]) && !isDigit(value[i])) {
				has_special_char = true;
				break;
			}
		if (has_special_char)
			params_[key] = "{" + value + "}";
		else
			params_[key] = value;
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
		} else if (par[i] == '{' && par[i - 1] == '=')
			braces ++;
		else if (par[i] == '}' && (i == par.size() - 1 || par[i + 1] == ',' || par[i + 1] == '\n'))
			braces --;
		
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
	return params_.find("float") != params_.end();
}


string InsetListingsParams::getParamValue(string const & param) const
{
	// is this parameter defined?
	map<string, string>::const_iterator it = params_.find(param);
	return (it == params_.end()) ? string() : it->second;
}


} // namespace lyx
