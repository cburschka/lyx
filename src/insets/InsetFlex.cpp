/**
 * \file InsetFlex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetFlex.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Language.h"
#include "Lexer.h"
#include "ParIterator.h"
#include "TextClass.h"

#include "support/gettext.h"
#include "support/lstrings.h"

#include <ostream>

using namespace std;

namespace lyx {


InsetFlex::InsetFlex(Buffer * buf, string const & layoutName)
	: InsetCollapsable(buf), name_(layoutName)
{}


InsetFlex::InsetFlex(InsetFlex const & in)
	: InsetCollapsable(in), name_(in.name_)
{}


// special code for InsetFlex when there is not the explicit Flex:: prefix
InsetLayout const & InsetFlex::getLayout() const
{
	if (!buffer_)
		return DocumentClass::plainInsetLayout();

	DocumentClass const & dc = buffer().params().documentClass();
	docstring const dname = from_utf8(name_); 
	if (dc.hasInsetLayout(dname))
		return dc.insetLayout(dname);
	return dc.insetLayout(from_utf8("Flex:" + name_));
}


InsetLayout::InsetDecoration InsetFlex::decoration() const
{
	InsetLayout::InsetDecoration const dec = getLayout().decoration();
	return dec == InsetLayout::DEFAULT ? InsetLayout::CONGLOMERATE : dec;
}


void InsetFlex::write(ostream & os) const
{
	os << "Flex ";
	InsetLayout const & il = getLayout();
	if (name_.empty())
		os << "undefined";
	else {
		// use il.name(), since this resolves obsoleted
		// InsetLayout names
		string name = to_utf8(il.name());
		// Remove the "Flex:" prefix, if it is present
		if (support::prefixIs(name, "Flex:"))
			name = support::token(name, ':', 1);
		os << name;
	}
	os << "\n";
	InsetCollapsable::write(os);
}


bool InsetFlex::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action()) {
	case LFUN_INSET_DISSOLVE:
		if (!cmd.argument().empty()) {
			InsetLayout const & il = getLayout();
			InsetLayout::InsetLyXType const type = 
				translateLyXType(to_utf8(cmd.argument()));
			if (il.lyxtype() == type) {
				FuncRequest temp_cmd(LFUN_INSET_DISSOLVE);
				return InsetCollapsable::getStatus(cur, temp_cmd, flag);
			} else
				return false;
		}
		// fall-through
	default:
		return InsetCollapsable::getStatus(cur, cmd, flag);
	}
}


void InsetFlex::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {
	case LFUN_INSET_DISSOLVE:
		if (!cmd.argument().empty()) {
			InsetLayout const & il = getLayout();
			InsetLayout::InsetLyXType const type = 
				translateLyXType(to_utf8(cmd.argument()));
			
			if (il.lyxtype() == type) {
				FuncRequest temp_cmd(LFUN_INSET_DISSOLVE);
				InsetCollapsable::doDispatch(cur, temp_cmd);
			} else
				cur.undispatched();
			break;
		}
		// fall-through
	default:
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}
}


void InsetFlex::updateBuffer(ParIterator const & it, UpdateType utype)
{
	BufferParams const & bp = buffer().masterBuffer()->params();
	InsetLayout const & il = getLayout();
	docstring custom_label = translateIfPossible(il.labelstring());

	Counters & cnts = bp.documentClass().counters();
	docstring const & count = il.counter();
	bool const have_counter = cnts.hasCounter(count);
	if (have_counter) {
		cnts.step(count, utype);
		custom_label += ' ' +
			cnts.theCounter(count, it.paragraph().getParLanguage(bp)->code());
	}
	setLabel(custom_label);
	
	bool const save_counter = have_counter && utype == OutputUpdate;
	if (save_counter) {
		// we assume the counter is local to this inset
		// if this turns out to be wrong in some case, we will
		// need a layout flag
		cnts.saveLastCounter();
	}
	InsetCollapsable::updateBuffer(it, utype);
	if (save_counter)
		cnts.restoreLastCounter();
}


} // namespace lyx
