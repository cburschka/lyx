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

#include "InsetMathNest.h"
#include "Buffer.h"

#include "support/debug.h"
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


/////////////////////////////////////////////////////////////////////
//
// MacroData
//
/////////////////////////////////////////////////////////////////////

MacroData::MacroData()
	: numargs_(0), lockCount_(0), redefinition_(false)
{}


MacroData::MacroData(docstring const & definition,
		std::vector<docstring> const & defaults, 
		int numargs, int optionals, docstring const & display,
		string const & requires)
	: definition_(definition), numargs_(numargs), display_(display),
		requires_(requires), lockCount_(0), redefinition_(false),
		optionals_(optionals), defaults_(defaults)
{
	defaults_.resize(optionals);
}


void MacroData::expand(vector<MathData> const & args, MathData & to) const
{
	InsetMathSqrt inset; // Hack. Any inset with a cell would do.
	// FIXME UNICODE
	asArray(display_.empty() ? definition_ : display_, inset.cell(0));
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


size_t MacroData::optionals() const
{
	return optionals_;
}


std::vector<docstring> const &  MacroData::defaults() const
{
	return defaults_;
}


void MacroData::unlock() const
{
	--lockCount_;
	BOOST_ASSERT(lockCount_ >= 0);
}


/////////////////////////////////////////////////////////////////////
//
// The global table of macros
//
/////////////////////////////////////////////////////////////////////

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
			<< " [" << to_utf8(it->second.definition()) << "] : "
			<< " [" << to_utf8(it->second.display()) << "] : "
			<< endl;
	lyxerr << "------------------------------------------" << endl;
}


MacroContext::MacroContext(Buffer const & buf, Paragraph const & par)
	: buf_(buf), par_(par)
{
}


bool MacroContext::has(docstring const & name) const
{
	// check if it's a local macro
	if (macros_.has(name))
		return true;
	
	// otherwise ask the buffer
	return buf_.hasMacro(name, par_);
}


MacroData const & MacroContext::get(docstring const & name) const
{
	// check if it's a local macro
	if (macros_.has(name))
		return macros_.get(name);
	
	// ask the buffer for its macros
	return buf_.getMacro(name, par_);
}


void MacroContext::insert(docstring const & name, MacroData const & data)
{
	macros_.insert(name, data);
}


} // namespace lyx
