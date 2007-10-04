/**
 * \file ControlMath.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlMath.h"
#include "debug.h"
#include "FuncRequest.h"

using std::string;
using std::map;

namespace lyx {
namespace frontend {

ControlMath::ControlMath(Dialog & dialog)
	: Controller(dialog)
{
	// FIXME: Ideally, those unicode codepoints would be defined
	// in "lib/symbols". Unfortunately, some of those are already
	// defined with non-unicode ids for use within mathed.
	// FIXME 2: We should fill-in this map with the parsed "symbols"
	// file done in MathFactory.cpp.
	math_symbols_["("] = MathSymbol('(');
	math_symbols_[")"] = MathSymbol(')');
	math_symbols_["{"] = MathSymbol('{');
	math_symbols_["}"] = MathSymbol('}');
	math_symbols_["["] = MathSymbol('[');
	math_symbols_["]"] = MathSymbol(']');
	math_symbols_["|"] = MathSymbol('|');
	math_symbols_["/"] = MathSymbol('/', 54, Font::CMSY_FAMILY);
	math_symbols_["backslash"] = MathSymbol('\\', 110, Font::CMSY_FAMILY);
	math_symbols_["lceil"] = MathSymbol(0x2308, 100, Font::CMSY_FAMILY);
	math_symbols_["rceil"] = MathSymbol(0x2309, 101, Font::CMSY_FAMILY);
	math_symbols_["lfloor"] = MathSymbol(0x230A, 98, Font::CMSY_FAMILY);
	math_symbols_["rfloor"] = MathSymbol(0x230B, 99, Font::CMSY_FAMILY);
	math_symbols_["langle"] = MathSymbol(0x2329, 104, Font::CMSY_FAMILY);
	math_symbols_["rangle"] = MathSymbol(0x232A, 105, Font::CMSY_FAMILY);
	math_symbols_["uparrow"] = MathSymbol(0x2191, 34, Font::CMSY_FAMILY);
	math_symbols_["Uparrow"] = MathSymbol(0x21D1, 42, Font::CMSY_FAMILY);
	math_symbols_["updownarrow"] = MathSymbol(0x2195, 108, Font::CMSY_FAMILY);
	math_symbols_["Updownarrow"] = MathSymbol(0x21D5, 109, Font::CMSY_FAMILY);
	math_symbols_["downarrow"] = MathSymbol(0x2193, 35, Font::CMSY_FAMILY);
	math_symbols_["Downarrow"] = MathSymbol(0x21D3, 43, Font::CMSY_FAMILY);
	math_symbols_["downdownarrows"] = MathSymbol(0x21CA, 184, Font::MSA_FAMILY);
	math_symbols_["downharpoonleft"] = MathSymbol(0x21C3, 188, Font::MSA_FAMILY);
	math_symbols_["downharpoonright"] = MathSymbol(0x21C2, 186, Font::MSA_FAMILY);
	math_symbols_["vert"] = MathSymbol(0x007C, 106, Font::CMSY_FAMILY);
	math_symbols_["Vert"] = MathSymbol(0x2016, 107, Font::CMSY_FAMILY);

	std::map<string, MathSymbol>::const_iterator it = math_symbols_.begin();
	std::map<string, MathSymbol>::const_iterator end = math_symbols_.end();
	for (; it != end; ++it)
		tex_names_[it->second.unicode] = it->first;
}


void ControlMath::dispatchFunc(kb_action action, string const & arg) const
{
	dispatch(FuncRequest(action, arg));
}


void ControlMath::dispatchInsert(string const & name) const
{
	dispatchFunc(LFUN_MATH_INSERT, '\\' + name);
}


void ControlMath::dispatchSubscript() const
{
	dispatchFunc(LFUN_MATH_INSERT, "_");
}


void ControlMath::dispatchSuperscript() const
{
	dispatchFunc(LFUN_MATH_INSERT, "^");
}


void ControlMath::dispatchCubeRoot() const
{
	dispatchFunc(LFUN_MATH_INSERT, "\\root");
	dispatchFunc(LFUN_SELF_INSERT, "3");
	dispatchFunc(LFUN_CHAR_FORWARD);
}


void ControlMath::dispatchMatrix(string const & str) const
{
	dispatchFunc(LFUN_MATH_MATRIX, str);
}


void ControlMath::dispatchDelim(string const & str) const
{
	dispatchFunc(LFUN_MATH_DELIM, str);
}


void ControlMath::dispatchBigDelim(string const & str) const
{
	dispatchFunc(LFUN_MATH_BIGDELIM, str);
}


void ControlMath::dispatchToggleDisplay() const
{
	dispatchFunc(LFUN_MATH_DISPLAY);
}


void ControlMath::showDialog(string const & name) const
{
	dispatchFunc(LFUN_DIALOG_SHOW, name);
}


MathSymbol const & ControlMath::mathSymbol(string tex_name) const
{
	map<string, MathSymbol>::const_iterator it =
		math_symbols_.find(tex_name);

	static MathSymbol unknown_symbol;
	if (it == math_symbols_.end())
		return unknown_symbol;

	return it->second;
}


std::string const & ControlMath::texName(char_type math_symbol) const
{
	map<char_type, string>::const_iterator it =
		tex_names_.find(math_symbol);

	static string empty_string;
	if (it == tex_names_.end())
		return empty_string;

	return it->second;
}


char const * function_names[] = {
	"arccos", "arcsin", "arctan", "arg", "bmod",
	"cos", "cosh", "cot", "coth", "csc", "deg",
	"det", "dim", "exp", "gcd", "hom", "inf", "ker",
	"lg", "lim", "liminf", "limsup", "ln", "log",
	"max", "min", "sec", "sin", "sinh", "sup",
	"tan", "tanh", "Pr", ""
};

int const nr_function_names = sizeof(function_names) / sizeof(char const *) - 1;

char const *  latex_delimiters[] = {
	"(", ")", "{", "}", "[", "]",
	"lceil", "rceil", "lfloor", "rfloor", "langle", "rangle",
	"uparrow", "updownarrow", "Uparrow", "Updownarrow", "downarrow", "Downarrow",
	"|", "Vert", "/", "backslash", ""
};


int const nr_latex_delimiters = sizeof(latex_delimiters) / sizeof(char const *);

} // namespace frontend
} // namespace lyx
