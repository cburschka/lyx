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
#include "MathParser.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "InsetMathNest.h"

#include "Buffer.h"
#include "DocIterator.h"
#include "InsetList.h"
#include "Text.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lassert.h"

#include <sstream>

using namespace std;

namespace lyx {


/////////////////////////////////////////////////////////////////////
//
// MacroData
//
/////////////////////////////////////////////////////////////////////

MacroData::MacroData(Buffer * buf)
	: buffer_(buf), queried_(true), numargs_(0), sym_(0), optionals_(0),
	  lockCount_(0), redefinition_(false), type_(MacroTypeNewcommand)
{}


MacroData::MacroData(Buffer * buf, DocIterator const & pos)
	: buffer_(buf), pos_(pos), queried_(false), numargs_(0), sym_(0),
	  optionals_(0), lockCount_(0), redefinition_(false),
	  type_(MacroTypeNewcommand)
{
}


MacroData::MacroData(Buffer * buf, MathMacroTemplate const & macro)
	: buffer_(buf), queried_(false), numargs_(0), sym_(0), optionals_(0),
	  lockCount_(0), redefinition_(false), type_(MacroTypeNewcommand)
{
	queryData(macro);
}


bool MacroData::expand(vector<MathData> const & args, MathData & to) const
{
	updateData();

	// Hack. Any inset with a cell would do.
	static InsetMathSqrt inset(0);
	inset.setBuffer(const_cast<Buffer &>(*buffer_));

	docstring const & definition(display_.empty() ? definition_ : display_);
	asArray(definition, inset.cell(0));
	//lyxerr << "MathData::expand: args: " << args << endl;
	//LYXERR0("MathData::expand: ar: " << inset.cell(0));
	for (DocIterator it = doc_iterator_begin(buffer_, &inset); it; it.forwardChar()) {
		if (!it.nextInset())
			continue;
		if (it.nextInset()->lyxCode() != MATH_MACROARG_CODE)
			continue;
		//it.cell().erase(it.pos());
		//it.cell().insert(it.pos(), it.nextInset()->asInsetMath()
		size_t n = static_cast<MathMacroArgument*>(it.nextInset())->number();
		if (n <= args.size()) {
			it.cell().erase(it.pos());
			it.cell().insert(it.pos(), args[n - 1]);
		}
	}
	//LYXERR0("MathData::expand: res: " << inset.cell(0));
	to = inset.cell(0);
	// If the result is equal to the definition then we either have a
	// recursive loop, or the definition did not contain any macro in the
	// first place.
	return asString(to) != definition;
}


size_t MacroData::optionals() const
{
	updateData();
	return optionals_;
}


vector<docstring> const & MacroData::defaults() const
{
	updateData();
	return defaults_;
}


string const MacroData::requires() const
{
	if (sym_)
		return sym_->requires;
	return string();
}


bool MacroData::hidden() const
{
	if (sym_)
		return sym_->hidden;
	return false;
}


docstring const MacroData::xmlname() const
{
	if (sym_)
		return sym_->xmlname;
	return docstring();
}


char const * MacroData::MathMLtype() const
{
	return sym_ ? sym_->MathMLtype() : 0;
}


void MacroData::unlock() const
{
	--lockCount_;
	LASSERT(lockCount_ >= 0, lockCount_ = 0);
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

	LBUFERR(buffer_);

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


int MacroData::write(odocstream & os, bool overwriteRedefinition) const
{
	updateData();

	// find macro template
	Inset * inset = pos_.nextInset();
	if (inset == 0 || inset->lyxCode() != MATHMACRO_CODE) {
		lyxerr << "BUG: No macro template found by MacroData" << endl;
		return 0;
	}

	// output template
	MathMacroTemplate const & tmpl =
		static_cast<MathMacroTemplate const &>(*inset);
	WriteStream wi(os, false, true, WriteStream::wsDefault);
	return tmpl.write(wi, overwriteRedefinition);
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


MacroTable::iterator
MacroTable::insert(docstring const & name, MacroData const & data)
{
	//lyxerr << "MacroTable::insert: " << to_utf8(name) << endl;
	iterator it = find(name);
	if (it == end())
		it = map<docstring, MacroData>::insert(
				make_pair(name, data)).first;
	else
		it->second = data;
	return it;
}


MacroTable::iterator
MacroTable::insert(Buffer * buf, docstring const & def)
{
	//lyxerr << "MacroTable::insert, def: " << to_utf8(def) << endl;
	MathMacroTemplate mac(buf, def);
	MacroData data(buf, mac);
	return insert(mac.name(), data);
}


void MacroTable::getMacroNames(std::set<docstring> & names, bool gethidden) const
{
	for (const_iterator it = begin(); it != end(); ++it)
		if (gethidden || !it->second.hidden())
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

MacroContext::MacroContext(Buffer const * buf, DocIterator const & pos)
	: buf_(buf), pos_(pos)
{
}


MacroData const * MacroContext::get(docstring const & name) const
{
	return buf_->getMacro(name, pos_);
}

} // namespace lyx
