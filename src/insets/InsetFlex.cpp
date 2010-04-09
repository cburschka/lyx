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
#include "Lexer.h"

#include "support/gettext.h"

#include <ostream>

using namespace std;

namespace lyx {


InsetFlex::InsetFlex(Buffer * buf, string const & layoutName)
	: InsetCollapsable(buf), name_(layoutName)
{
	status_= Collapsed;
}


InsetFlex::InsetFlex(InsetFlex const & in)
	: InsetCollapsable(in), name_(in.name_)
{}


InsetLayout::InsetDecoration InsetFlex::decoration() const
{
	InsetLayout::InsetDecoration const dec = getLayout().decoration();
	return dec == InsetLayout::DEFAULT ? InsetLayout::CONGLOMERATE : dec;
}


void InsetFlex::write(ostream & os) const
{
	os << "Flex " <<
		(name_.empty() ? "undefined" : name_) << "\n";
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


} // namespace lyx
