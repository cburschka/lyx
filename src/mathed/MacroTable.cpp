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

#include "InsetMathSqrt.h"
#include "MacroTable.h"
#include "MathMacroTemplate.h"
#include "MathMacroArgument.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "InsetMathNest.h"

#include "Buffer.h"
#include "DocIterator.h"
#include "InsetList.h"
#include "Text.h"

#include "support/debug.h"

#include "support/lassert.h"

#include <sstream>

using namespace std;

namespace lyx {


/////////////////////////////////////////////////////////////////////
//
// MacroData
//
/////////////////////////////////////////////////////////////////////

MacroData::MacroData()
	: queried_(true), numargs_(0), optionals_(0), lockCount_(0),
	  redefinition_(false), type_(MacroTypeNewcommand)
{}

	
	
MacroData::MacroData(Buffer const & buf, DocIterator const & pos)
	: buffer_(&buf), pos_(pos), queried_(false), numargs_(0),
	  optionals_(0), lockCount_(0), redefinition_(false),
	  type_(MacroTypeNewcommand)
{
}
	
	
MacroData::MacroData(MathMacroTemplate const & macro)
	: queried_(false), numargs_(0), optionals_(0), lockCount_(0),
	  redefinition_(false), type_(MacroTypeNewcommand)
{
	queryData(macro);
}


void MacroData::expand(vector<MathData> const & args, MathData & to) const
{
	updateData();

	// Hack. Any inset with a cell would do.
	static Buffer buffer("");
	static InsetMathSqrt inset;

	// FIXME UNICODE
	asArray(display_.empty() ? definition_ : display_, inset.cell(0));
	//lyxerr << "MathData::expand: args: " << args << endl;
	//lyxerr << "MathData::expand: ar: " << inset.cell(0) << endl;
	for (DocIterator it = doc_iterator_begin(&buffer, &inset); it; it.forwardChar()) {
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
	updateData();
	return optionals_;
}


vector<docstring> const &  MacroData::defaults() const
{
	updateData();
	return defaults_;
}


void MacroData::unlock() const
{
	--lockCount_;
	LASSERT(lockCount_ >= 0, /**/);
}


void MacroData::queryData(MathMacroTemplate const & macro) const
{
	if (queried_)
		return;

	queried_ = true;
	definition_ = macro.definition();
	numargs_ = macro.numArgs();
	display_ = macro.displayDefinition();
	redefinition_ = macro.redefinition();
	type_ = macro.type();
	optionals_ = macro.numOptionals();
	
	macro.getDefaults(defaults_);
}


void MacroData::updateData() const
{
	if (queried_)
		return;

	LASSERT(buffer_ != 0, /**/);
	
	// Try to fix position DocIterator. Should not do anything in theory.
	pos_.fixIfBroken();
	
	// find macro template
	Inset * inset = pos_.nextInset();
	if (inset == 0 || inset->lyxCode() != MATHMACRO_CODE) {
		lyxerr << "BUG: No macro template found by MacroData" << endl;
		return;
	}
	
	// query the data from the macro template
	queryData(static_cast<MathMacroTemplate const &>(*inset));	
}
	

void MacroData::write(odocstream & os, bool overwriteRedefinition) const
{
	updateData();

	// find macro template
	Inset * inset = pos_.nextInset();
	if (inset == 0 || inset->lyxCode() != MATHMACRO_CODE) {
		lyxerr << "BUG: No macro template found by MacroData" << endl;
		return;
	}
		
	// output template
	MathMacroTemplate const & tmpl =
		static_cast<MathMacroTemplate const &>(*inset);
	WriteStream wi(os, false, true, false);
	tmpl.write(wi, overwriteRedefinition);
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


MacroData const * MacroTable::get(docstring const & name) const
{
	const_iterator it = find(name);
	return it == end() ? 0 : &it->second;
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
	MacroData data(mac);
	data.requires() = requires;
	insert(mac.name(), data);
}


void MacroTable::getMacroNames(std::set<docstring> & names) const
{
	for (const_iterator it = begin(); it != end(); ++it)
		names.insert(it->first);
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


/////////////////////////////////////////////////////////////////////
//
// MacroContext
//
/////////////////////////////////////////////////////////////////////

MacroContext::MacroContext(Buffer const & buf, DocIterator const & pos)
	: buf_(buf), pos_(pos)
{
}


MacroData const * MacroContext::get(docstring const & name) const
{
	return buf_.getMacro(name, pos_);
}

} // namespace lyx
