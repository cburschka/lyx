#include <config.h>

#include <iostream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macrotable.h"
#include "math_macro.h"
#include "math_macrotemplate.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "debug.h"
#include "math_support.h" // math_font_available


MathMacroTable::table_type MathMacroTable::macro_table;


void MathMacroTable::dump()
{
/*
	lyxerr << "\n------------------------------------------\n";
	table_type::const_iterator it;
	for (it = macro_table.begin(); it != macro_table.end(); ++it)
		lyxerr << it->first
			<< " [" << it->second->asMacroTemplate()->nargs() << "] : "
			<< it->second->cell(0) << "\n";
	lyxerr << "------------------------------------------\n";
*/
}


MathAtom & MathMacroTable::provide(string const & name)
{
	table_type::iterator pos = macro_table.find(name);
	if (pos == macro_table.end()) {
		lyxerr << "MathMacroTable::provideTemplate: no template with name '"
		       << name << "' available.\n";
	}
	return pos->second;
}


void MathMacroTable::create(MathAtom const & at)
{
	macro_table[at->asMacroTemplate()->name()] = at;
}


bool MathMacroTable::has(string const & name)
{
	return macro_table.find(name) != macro_table.end();
}
