/**
 * \file math_macrotable.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "debug.h"

#include <boost/assert.hpp>


using std::string;
using std::endl;


MathMacroTable::table_type MathMacroTable::macro_table;


void MathMacroTable::dump()
{
/*
	lyxerr << "\n------------------------------------------" << endl;
	table_type::const_iterator it;
	for (it = macro_table.begin(); it != macro_table.end(); ++it)
		lyxerr << it->first
			<< " [" << it->second->asMacroTemplate()->nargs() << "] : "
			<< it->second->cell(0) << endl;
	lyxerr << "------------------------------------------" << endl;
*/
}


MathAtom & MathMacroTable::provide(string const & name)
{
	table_type::iterator pos = macro_table.find(name);
	if (pos == macro_table.end()) {
		lyxerr << "MathMacroTable::provideTemplate: no template with name '"
		       << name << "' available." << endl;
		BOOST_ASSERT(false);
	}
	return pos->second;
}


void MathMacroTable::create(MathAtom const & at)
{
	//lyxerr << "MathMacroTable::create: '"
	//	<< at->asMacroTemplate()->name() << "'" << endl;
	macro_table[at->asMacroTemplate()->name()] = at;
}


bool MathMacroTable::has(string const & name)
{
	return macro_table.find(name) != macro_table.end();
}
