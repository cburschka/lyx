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
#include "WordLangTuple.h"

#include "support/lstrings.h"


using lyx::support::strToDbl;
using lyx::support::strToInt;

using std::string;


// some stuff for inset locking

void UpdatableInset::insetUnlock(BufferView *)
{
	lyxerr[Debug::INFO] << "Inset Unlock" << std::endl;
}


// An updatable inset is highly editable by definition
InsetOld::EDITABLE UpdatableInset::editable() const
{
	return HIGHLY_EDITABLE;
}


void UpdatableInset::fitInsetCursor(BufferView *) const
{}


void UpdatableInset::draw(PainterInfo &, int, int) const
{
	// ATTENTION: don't do the following here!!!
	//    top_x = x;
	//    top_baseline = y;
}


void UpdatableInset::scroll(BufferView * bv, float s) const
{
	//LyXFont font;

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
	// if (!display())
	// scx += 20;

#warning metrics?
	if (tmp_top_x + scx + width() < workW / 2) {
		scx += workW / 2 - (tmp_top_x + scx + width());
	}
}

void UpdatableInset::scroll(BufferView * bv, int offset) const
{
	if (offset > 0) {
		if (!scx && top_x >= 20)
			return;
		if ((top_x + offset) > 20)
			scx = 0;
		// scx += offset - (top_x - scx + offset - 20);
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
UpdatableInset::priv_dispatch(FuncRequest const & ev, idx_type &, pos_type &)
{
	if (ev.action == LFUN_MOUSE_RELEASE)
		return DispatchResult(editable() == IS_EDITABLE);

	if (!ev.argument.empty() && ev.action == LFUN_SCROLL_INSET) {
		if (ev.argument.find('.') != ev.argument.npos) {
			float const xx = static_cast<float>(strToDbl(ev.argument));
			scroll(ev.view(), xx);
		} else {
			int const xx = strToInt(ev.argument);
			scroll(ev.view(), xx);
		}
		ev.view()->updateInset(this);

		return DispatchResult(true);
	}
	return DispatchResult(false);
}


LyXCursor const & InsetOld::cursor(BufferView * bv) const
{
	if (owner())
		return owner()->getLyXText(bv, false)->cursor;
	return bv->text->cursor;
}


WordLangTuple const
UpdatableInset::selectNextWordToSpellcheck(BufferView *bv, float & value) const
{
	// we have to unlock ourself in this function by default!
	bv->unlockInset(const_cast<UpdatableInset *>(this));
	value = 0;
	return WordLangTuple();
}


bool UpdatableInset::nextChange(BufferView * bv, lyx::pos_type &)
{
	// we have to unlock ourself in this function by default!
	bv->unlockInset(const_cast<UpdatableInset *>(this));
	return false;
}


bool UpdatableInset::searchForward(BufferView * bv, string const &,
				   bool, bool)
{
	// we have to unlock ourself in this function by default!
	bv->unlockInset(const_cast<UpdatableInset *>(this));
	return false;
}


bool UpdatableInset::searchBackward(BufferView * bv, string const &,
				    bool, bool)
{
	// we have to unlock ourself in this function by default!
	bv->unlockInset(const_cast<UpdatableInset *>(this));
	return false;
}
