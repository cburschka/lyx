/**
 * \file MacroTable.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MacroTable.h"
#include "MathMacroTemplate.h"
#include "MathMacroArgument.h"
#include "MathSupport.h"
#include "InsetMathSqrt.h"

#include "debug.h"
#include "DocIterator.h"

#include <boost/assert.hpp>

#include <sstream>


namespace lyx {

using std::endl;
using std::istringstream;
using std::map;
using std::pair;
using std::string;
using std::vector;
using std::size_t;


MacroData::MacroData()
	: numargs_(0), lockCount_(0)
{}


MacroData::MacroData(docstring const & def, int numargs, docstring const & disp, string const & requires)
	: def_(def), numargs_(numargs), disp_(disp), requires_(requires), lockCount_(0)
{}


void MacroData::expand(vector<MathData> const & args, MathData & to) const
{
	InsetMathSqrt inset; // Hack. Any inset with a cell would do.
	// FIXME UNICODE
	asArray(disp_.empty() ? def_ : disp_, inset.cell(0));
	//lyxerr << "MathData::expand: args: " << args << endl;
	//lyxerr << "MathData::expand: ar: " << inset.cell(0) << endl;
	for (DocIterator it = doc_iterator_begin(inset); it; it.forwardChar()) {
		if (!it.nextInset())
			continue;
		if (it.nextInset()->lyxCode() != MATHMACROARG_CODE)
			continue;
		//it.cell().erase(it.pos());
		//it.cell().insert(it.pos(), it.nextInset()->asInsetMath()
		size_t n = static_cast<MathMacroArgument*>(it.nextInset())->number();
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


bool MacroTable::has(docstring const & name) const
{
	return find(name) != end();
}


MacroData const & MacroTable::get(docstring const & name) const
{
	const_iterator it = find(name);
	BOOST_ASSERT(it != end());
	return it->second;
}


void MacroTable::insert(docstring const & name, MacroData const & data)
{
	//lyxerr << "MacroTable::insert: " << to_utf8(name) << endl;
	operator[](name) = data;
}


void MacroTable::insert(docstring const & def, string const & requires)
{
	//lyxerr << "MacroTable::insert, def: " << to_utf8(def) << endl;
	MathMacroTemplate mac(def);
	MacroData data = mac.asMacroData();
	data.requires() = requires;
	insert(mac.name(), data);
}


void MacroTable::dump()
{
	lyxerr << "\n------------------------------------------" << endl;
	for (const_iterator it = begin(); it != end(); ++it)
		lyxerr << to_utf8(it->first)
			<< " [" << to_utf8(it->second.def()) << "] : "
			<< " [" << to_utf8(it->second.disp()) << "] : "
			<< endl;
	lyxerr << "------------------------------------------" << endl;
}


} // namespace lyx
