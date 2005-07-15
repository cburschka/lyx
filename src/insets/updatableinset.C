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
#include "coordcache.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "lyxtext.h"

#include "support/convert.h"

#include <boost/assert.hpp>


// An updatable inset is highly editable by definition
InsetBase::EDITABLE UpdatableInset::editable() const
{
	return HIGHLY_EDITABLE;
}


int UpdatableInset::scroll(bool) const
{
	return scx;
}


void UpdatableInset::setScroll(int maxwidth, double s) const
{
	lyxerr << "UpdatableInset::setScroll: (int) " << maxwidth << std::endl;
	if (!s) {
		scx = 0;
		return;
	}

	int xo_ = theCoords.getInsets().x(this);
	int const tmp_xo_ = xo_ - scx;

	if (tmp_xo_ > 0 && tmp_xo_ + width() < maxwidth)
		return;
	if (s > 0.0 && xo_ > 0)
		return;

	scx = int(s * maxwidth / 2);

#ifdef WITH_WARNINGS
#warning metrics?
#endif
	if (tmp_xo_ + scx + width() < maxwidth / 2)
		scx = maxwidth / 2 - tmp_xo_ - width();
}


void UpdatableInset::setScroll(int maxwidth, int offset) const
{
	lyxerr << "UpdatableInset::setScroll: (double) " << maxwidth << std::endl;
	int const xo_ = theCoords.getInsets().x(this);
	if (offset > 0) {
		if (!scx && xo_ >= 20)
			return;
		if (xo_ + offset > 20)
			scx = 0;
		// scx = - xo_;
		else
			scx += offset;
	} else {
#ifdef WITH_WARNINGS
#warning metrics?
#endif
		if (!scx && xo_ + width() < maxwidth - 20)
			return;

		if (xo_ - scx + offset + width() < maxwidth - 20)
			scx += maxwidth - width() - xo_ - 20;
		else
			scx += offset;
	}
}


void UpdatableInset::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_SCROLL_INSET:
		if (cmd.argument.empty()) {
			const int maxwidth = cur.bv().workWidth();
			if (cmd.argument.find('.') != cmd.argument.npos)
				setScroll(maxwidth, static_cast<float>(convert<double>(cmd.argument)));
			else
				setScroll(maxwidth, convert<int>(cmd.argument));
		} else
			cur.noUpdate();
		break;

	default:
		InsetBase::dispatch(cur, cmd);
	}
}


void UpdatableInset::getCursorDim(int &, int &) const
{
	BOOST_ASSERT(false);
}

