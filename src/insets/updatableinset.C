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
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "lyxtext.h"

#include "support/lstrings.h"

#include <boost/assert.hpp>

using lyx::support::strToDbl;
using lyx::support::strToInt;



// An updatable inset is highly editable by definition
InsetOld::EDITABLE UpdatableInset::editable() const
{
	return HIGHLY_EDITABLE;
}


void UpdatableInset::scroll(BufferView & bv, float s) const
{
	if (!s) {
		scx = 0;
		return;
	}

	int const workW = bv.workWidth();
	int const tmp_xo_ = xo_ - scx;

	if (tmp_xo_ > 0 && tmp_xo_ + width() < workW)
		return;
	if (s > 0 && xo_ > 0)
		return;

	scx = int(s * workW / 2);

#ifdef WITH_WARNINGS
#warning metrics?
#endif
	if (tmp_xo_ + scx + width() < workW / 2)
		scx = workW / 2 - tmp_xo_ - width();
}


void UpdatableInset::scroll(BufferView & bv, int offset) const
{
	if (offset > 0) {
		if (!scx && xo_ >= 20)
			return;
		if (xo_ + offset > 20)
			scx = 0;
		// scx = - xo_;
		else
			scx += offset;
	} else {
#warning metrics?
		if (!scx && xo_ + width() < bv.workWidth() - 20)
			return;
		if (xo_ - scx + offset + width() < bv.workWidth() - 20) {
			scx += bv.workWidth() - width() - xo_ - 20;
		} else {
			scx += offset;
		}
	}
}


void UpdatableInset::priv_dispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	//case LFUN_MOUSE_RELEASE:
	//	return DispatchResult(editable() == IS_EDITABLE);

	case LFUN_SCROLL_INSET:
		if (cmd.argument.empty()) {
			if (cmd.argument.find('.') != cmd.argument.npos)
				scroll(cur.bv(), static_cast<float>(strToDbl(cmd.argument)));
			else
				scroll(cur.bv(), strToInt(cmd.argument));
			cur.bv().update();
		}
		break;

	default:
		InsetOld::dispatch(cur, cmd);
	}
}


void UpdatableInset::getCursorDim(int &, int &) const
{
	BOOST_ASSERT(false);
}
