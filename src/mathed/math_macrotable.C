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
#include "math_macroarg.h"
#include "math_support.h"
#include "math_sqrtinset.h"

#include "debug.h"
#include "dociterator.h"

#include "support/std_sstream.h"

#include <boost/assert.hpp>

using std::endl;
using std::istringstream;
using std::map;
using std::pair;
using std::string;
using std::vector;


MacroData::MacroData() 
	: numargs_(0)
{}


MacroData::MacroData(string const & def, int numargs, string const & disp)
	: def_(def), numargs_(numargs), disp_(disp)
{}


void MacroData::expand(vector<MathArray> const & args, MathArray & to) const
{
	MathSqrtInset inset; // Hack. Any inset with a cell would do.
	asArray(disp_.empty() ? def_ : disp_, inset.cell(0));
	//lyxerr << "MathData::expand: args: " << args << endl;
	//lyxerr << "MathData::expand: ar: " << inset.cell(0) << endl;
	for (DocIterator it = doc_iterator_begin(inset); it; it.forwardChar()) {
		if (!it.nextInset())
			continue;
		if (it.nextInset()->lyxCode() != InsetBase::MATHMACROARG_CODE)
			continue;
		//it.cell().erase(it.pos());
		//it.cell().insert(it.pos(), it.nextInset()->asMathInset()
		int n = static_cast<MathMacroArgument*>(it.nextInset())->number();
		if (n <= args.size()) {
			it.cell().erase(it.pos());
			it.cell().insert(it.pos(), args[n - 1]);
		}
	}
	//lyxerr << "MathData::expand: res: " << inset.cell(0) << endl;
	to = inset.cell(0);
}



// The global table.
MacroTable & MacroTable::globalMacros()
{
	static MacroTable theGlobalMacros;
	return theGlobalMacros;
}


// The local table.
//MacroTable & MacroTable::localMacros()
//{
//	static MacroTable theLocalMacros;
//	return theLocalMacros;
//}


bool MacroTable::has(string const & name) const
{
	return find(name) != end();
}


MacroData const & MacroTable::get(string const & name) const
{
	const_iterator it = find(name);
	BOOST_ASSERT(it != end());
	return it->second;
}


void MacroTable::insert(string const & name, MacroData const & data)
{
	//lyxerr << "MacroTable::insert: " << name << endl;
	operator[](name) = data;
}


void MacroTable::insert(string const & def)
{
	//lyxerr << "MacroTable::insert, def: " << def << endl;
	istringstream is(def);
	MathMacroTemplate mac(is);
	insert(mac.name(), mac.asMacroData());
}


void MacroTable::dump()
{
	lyxerr << "\n------------------------------------------" << endl;
	for (const_iterator it = begin(); it != end(); ++it)
		lyxerr << it->first
			<< " [" << it->second.def() << "] : "
			<< " [" << it->second.disp() << "] : "
			<< endl;
	lyxerr << "------------------------------------------" << endl;
}
