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
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "updatableinset.h"
#include "BufferView.h"
#include "funcrequest.h"
#include "debug.h"
#include "lyxfont.h"
#include "WordLangTuple.h"
#include "lyxtext.h"
#include "support/lstrings.h"


// some stuff for inset locking

UpdatableInset::UpdatableInset()
	: Inset(), block_drawing_(false)
{}


UpdatableInset::UpdatableInset(UpdatableInset const & in, bool same_id)
	: Inset(in, same_id), block_drawing_(false)
{}


void UpdatableInset::insetUnlock(BufferView *)
{
	lyxerr[Debug::INFO] << "Inset Unlock" << std::endl;
}


// An updatable inset is highly editable by definition
Inset::EDITABLE UpdatableInset::editable() const
{
	return HIGHLY_EDITABLE;
}


void UpdatableInset::fitInsetCursor(BufferView *) const
{}


void UpdatableInset::draw(BufferView *, LyXFont const &,
			  int /* baseline */, float & x) const
{
	x += float(scx);
	// ATTENTION: don't do the following here!!!
	//    top_x = int(x);
	//    top_baseline = baseline;
}


void UpdatableInset::scroll(BufferView * bv, float s) const
{
	LyXFont font;

	if (!s) {
		scx = 0;
		return;
	}

	int const workW = bv->workWidth();
	int const tmp_top_x = top_x - scx;

	if (tmp_top_x > 0 &&
	    (tmp_top_x + width(bv, font)) < workW)
		return;
	if (s > 0 && top_x > 0)
		return;

	// int mx_scx=abs((width(bv,font) - bv->workWidth())/2);
	//int const save_scx = scx;

	scx = int(s * workW / 2);
	// if (!display())
	// scx += 20;

	if ((tmp_top_x + scx + width(bv, font)) < (workW / 2)) {
		scx += (workW / 2) - (tmp_top_x + scx + width(bv, font));
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
		LyXFont const font;
		if (!scx && (top_x + width(bv, font)) < (bv->workWidth() - 20))
			return;
		if ((top_x - scx + offset + width(bv, font)) < (bv->workWidth() - 20)) {
			scx = bv->workWidth() - width(bv, font) - top_x + scx - 20;
		} else {
			scx += offset;
		}
	}
}


///  An updatable inset could handle lyx editing commands
Inset::RESULT UpdatableInset::localDispatch(FuncRequest const & ev)
{
	if (ev.action == LFUN_MOUSE_RELEASE)
		return (editable() == IS_EDITABLE) ? DISPATCHED : UNDISPATCHED;

	if (!ev.argument.empty() && ev.action == LFUN_SCROLL_INSET) {
		if (ev.argument.find('.') != ev.argument.npos) {
			float const xx = static_cast<float>(strToDbl(ev.argument));
			scroll(ev.view(), xx);
		} else {
			int const xx = strToInt(ev.argument);
			scroll(ev.view(), xx);
		}
		ev.view()->updateInset(this);

		return DISPATCHED;
	}
	return UNDISPATCHED;
}


int UpdatableInset::getMaxWidth(BufferView * bv, UpdatableInset const *) const
{
	int w;

	if (owner()) {
		w = static_cast<UpdatableInset*> (owner())->getMaxWidth(bv, this);
	} else {
		w = bv->text->workWidth(const_cast<UpdatableInset *>(this));
	}
	if (w < 0) {
		return -1;
	}
	// check for margins left/right and extra right margin "const 5"
	if ((w - ((2 * TEXT_TO_INSET_OFFSET) + 5)) >= 0)
		w -= (2 * TEXT_TO_INSET_OFFSET) + 5;

	// Deep magic. I don't understand this either.
	if (owner() && owner()->owner()) {
		// add the right paper margin
		w -= 20;
	}

	// FIXME: why ?
	if (w < 10) {
		w = 10;
	}
	return w;
}


LyXCursor const & Inset::cursor(BufferView * bv) const
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
