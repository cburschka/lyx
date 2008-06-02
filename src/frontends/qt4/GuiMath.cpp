/**
 * \file GuiMath.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiMath.h"

#include "FuncRequest.h"

#include "support/debug.h"

using namespace std;

namespace lyx {
namespace frontend {

GuiMath::GuiMath(GuiView & lv, QString const & name, QString const & title)
	: GuiDialog(lv, name, title)
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
	math_symbols_["/"] = MathSymbol('/', 54, CMSY_FAMILY);
	math_symbols_["backslash"] = MathSymbol('\\', 110, CMSY_FAMILY);
	math_symbols_["lceil"] = MathSymbol(0x2308, 100, CMSY_FAMILY);
	math_symbols_["rceil"] = MathSymbol(0x2309, 101, CMSY_FAMILY);
	math_symbols_["lfloor"] = MathSymbol(0x230A, 98, CMSY_FAMILY);
	math_symbols_["rfloor"] = MathSymbol(0x230B, 99, CMSY_FAMILY);
	math_symbols_["langle"] = MathSymbol(0x2329, 104, CMSY_FAMILY);
	math_symbols_["rangle"] = MathSymbol(0x232A, 105, CMSY_FAMILY);
	math_symbols_["uparrow"] = MathSymbol(0x2191, 34, CMSY_FAMILY);
	math_symbols_["Uparrow"] = MathSymbol(0x21D1, 42, CMSY_FAMILY);
	math_symbols_["updownarrow"] = MathSymbol(0x2195, 108, CMSY_FAMILY);
	math_symbols_["Updownarrow"] = MathSymbol(0x21D5, 109, CMSY_FAMILY);
	math_symbols_["downarrow"] = MathSymbol(0x2193, 35, CMSY_FAMILY);
	math_symbols_["Downarrow"] = MathSymbol(0x21D3, 43, CMSY_FAMILY);
	math_symbols_["downdownarrows"] = MathSymbol(0x21CA, 184, MSA_FAMILY);
	math_symbols_["downharpoonleft"] = MathSymbol(0x21C3, 188, MSA_FAMILY);
	math_symbols_["downharpoonright"] = MathSymbol(0x21C2, 186, MSA_FAMILY);
	math_symbols_["vert"] = MathSymbol(0x007C, 106, CMSY_FAMILY);
	math_symbols_["Vert"] = MathSymbol(0x2016, 107, CMSY_FAMILY);

	map<string, MathSymbol>::const_iterator it = math_symbols_.begin();
	map<string, MathSymbol>::const_iterator end = math_symbols_.end();
	for (; it != end; ++it)
		tex_names_[it->second.unicode] = it->first;
}


void GuiMath::dispatchFunc(FuncCode action, string const & arg) const
{
	dispatch(FuncRequest(action, arg));
}


MathSymbol const & GuiMath::mathSymbol(string tex_name) const
{
	map<string, MathSymbol>::const_iterator it =
		math_symbols_.find(tex_name);

	static MathSymbol unknown_symbol;
	if (it == math_symbols_.end())
		return unknown_symbol;

	return it->second;
}


string const & GuiMath::texName(char_type math_symbol) const
{
	map<char_type, string>::const_iterator it =
		tex_names_.find(math_symbol);

	static string empty_string;
	if (it == tex_names_.end())
		return empty_string;

	return it->second;
}


} // namespace frontend
} // namespace lyx
