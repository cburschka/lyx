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
	: InsetCollapsible(buf), name_(layoutName)
{}


InsetFlex::InsetFlex(InsetFlex const & in)
	: InsetCollapsible(in), name_(in.name_)
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


InsetDecoration InsetFlex::decoration() const
{
	InsetDecoration const dec = getLayout().decoration();
	return dec == InsetDecoration::DEFAULT ? InsetDecoration::CONGLOMERATE : dec;
}


void InsetFlex::write(ostream & os) const
{
	os << "Flex ";
	string name;
	if (name_.empty())
		name = "undefined";
	else {
		InsetLayout const & il = getLayout();
		// use il.name(), since this resolves obsoleted InsetLayout names
		if (il.name() == "undefined")
			// This is the name of the plain_insetlayout_. We assume that the
			// name resolution has failed.
			name = name_;
		else {
			name = to_utf8(il.name());
			// Remove the "Flex:" prefix, if it is present
			if (support::prefixIs(name, "Flex:"))
				name = support::split(name, ':');
		}
	}
	os << name << "\n";
	InsetCollapsible::write(os);
}


bool InsetFlex::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action()) {
	case LFUN_INSET_SPLIT:
	case LFUN_INSET_DISSOLVE:
		if (!cmd.argument().empty()) {
			InsetLayout const & il = getLayout();
			InsetLyXType const type =
				translateLyXType(to_utf8(cmd.argument()));
			if (il.lyxtype() == type
			    || (il.name() == DocumentClass::plainInsetLayout().name()
				    && type == InsetLyXType::CHARSTYLE)) {
				FuncRequest temp_cmd(cmd.action());
				return InsetCollapsible::getStatus(cur, temp_cmd, flag);
			} else
				return false;
		}
		// fall-through
	default:
		return InsetCollapsible::getStatus(cur, cmd, flag);
	}
}


void InsetFlex::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {
	case LFUN_INSET_SPLIT:
	case LFUN_INSET_DISSOLVE:
		if (!cmd.argument().empty()) {
			InsetLayout const & il = getLayout();
			InsetLyXType const type =
				translateLyXType(to_utf8(cmd.argument()));

			if (il.lyxtype() == type
			    || (il.name() == DocumentClass::plainInsetLayout().name()
				    && type == InsetLyXType::CHARSTYLE)) {
				FuncRequest temp_cmd(cmd.action());
				InsetCollapsible::doDispatch(cur, temp_cmd);
			} else
				cur.undispatched();
			break;
		}
		// fall-through
	default:
		InsetCollapsible::doDispatch(cur, cmd);
		break;
	}
}


void InsetFlex::updateBuffer(ParIterator const & it, UpdateType utype, bool const deleted)
{
	BufferParams const & bp = buffer().masterBuffer()->params();
	InsetLayout const & il = getLayout();
	docstring custom_label = translateIfPossible(il.labelstring());

	Counters & cnts = bp.documentClass().counters();
	docstring const & count = il.counter();
	bool const have_counter = cnts.hasCounter(count);
	if (have_counter) {
		Paragraph const & par = it.paragraph();
		if (!par.isDeleted(it.pos())) {
			cnts.step(count, utype);
			custom_label += ' ' +
				cnts.theCounter(count, it.paragraph().getParLanguage(bp)->code());
		} else
			custom_label += ' ' + from_ascii("#");
	}
	setLabel(custom_label);

	bool const save_counter = have_counter && utype == OutputUpdate;
	if (save_counter) {
		// we assume the counter is local to this inset
		// if this turns out to be wrong in some case, we will
		// need a layout flag
		cnts.saveLastCounter();
	}
	InsetCollapsible::updateBuffer(it, utype, deleted);
	if (save_counter)
		cnts.restoreLastCounter();
}


} // namespace lyx
