/**
 * \file updatableinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "updatableinset.h"

#include "BufferView.h"
#include "debug.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "lyxtext.h"

#include "support/lstrings.h"


using lyx::support::strToDbl;
using lyx::support::strToInt;


// An updatable inset is highly editable by definition
InsetOld::EDITABLE UpdatableInset::editable() const
{
	return HIGHLY_EDITABLE;
}


void UpdatableInset::fitInsetCursor(BufferView *) const
{}


void UpdatableInset::scroll(BufferView * bv, float s) const
{
	if (!s) {
		scx = 0;
		return;
	}

	int const workW = bv->workWidth();
	int const tmp_top_x = top_x - scx;

	if (tmp_top_x > 0 && tmp_top_x + width() < workW)
		return;
	if (s > 0 && top_x > 0)
		return;

	scx = int(s * workW / 2);

#warning metrics?
	if (tmp_top_x + scx + width() < workW / 2)
		scx = workW / 2 - tmp_top_x - width();
}


void UpdatableInset::scroll(BufferView * bv, int offset) const
{
	if (offset > 0) {
		if (!scx && top_x >= 20)
			return;
		if (top_x + offset > 20)
			scx = 0;
		// scx = - top_x;
		else
			scx += offset;
	} else {
#warning metrics?
		if (!scx && top_x + width() < bv->workWidth() - 20)
			return;
		if (top_x - scx + offset + width() < bv->workWidth() - 20) {
			scx += bv->workWidth() - width() - top_x - 20;
		} else {
			scx += offset;
		}
	}
}


///  An updatable inset could handle lyx editing commands
DispatchResult
UpdatableInset::priv_dispatch(FuncRequest const & cmd, idx_type &, pos_type &)
{
	switch (cmd.action) {
	case LFUN_MOUSE_RELEASE:
		return DispatchResult(editable() == IS_EDITABLE);

	case LFUN_SCROLL_INSET:
		if (!cmd.argument.empty()) {
			if (cmd.argument.find('.') != cmd.argument.npos)
				scroll(cmd.view(), static_cast<float>(strToDbl(cmd.argument)));
			else
				scroll(cmd.view(), strToInt(cmd.argument));
			cmd.view()->updateInset(this);
			return DispatchResult(true, true);
		}

	default:
		return DispatchResult(false);
	}
}
