/**
 * \file cursor.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "buffer.h"
#include "BufferView.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "iterators.h"
#include "lyxtext.h"
#include "paragraph.h"

#include "insets/updatableinset.h"

using std::vector;
using std::endl;


DispatchResult Cursor::dispatch(FuncRequest const & cmd)
{
	for (int i = data_.size() - 1; i >= 0; --i) {
		lyxerr << "trying to dispatch to inset" << data_[i].inset_ << endl;
		DispatchResult res = data_[i].inset_->dispatch(cmd);
		lyxerr << "   result: " << res.val() << endl;

		if (res == DISPATCHED) {
			//update();
			return DISPATCHED;
		}

		if (res == DISPATCHED_NOUPDATE)
			return DISPATCHED;

		lyxerr << "# unhandled result: " << res.val() << endl;
	}
	return UNDISPATCHED;
}


void buildCursor(Cursor & cursor, BufferView & bv)
{
	UpdatableInset * inset = bv.theLockingInset();
	lyxerr << "\nbuildCursor: " << inset << endl;
	if (!inset)
		return;

	inset = inset->getLockingInset();

	bool ok = false;
	ParIterator pit = bv.buffer()->par_iterator_begin();
	ParIterator end = bv.buffer()->par_iterator_end();
	for ( ; pit != end && !ok; ++pit) {
		InsetList::iterator	it = pit->insetlist.begin();
		InsetList::iterator	iend = pit->insetlist.end();
		for ( ; it != iend && !ok; ++it)
			if (it->inset == inset || it->inset == inset->owner())
				ok = true;
	}

	if (!ok) {
		lyxerr << " tli not found! inset: " << inset << endl;
		return;
	}

	pit.asCursor(cursor);
	for (size_t i = 0, n = cursor.data_.size(); i != n; ++i) {
		lyxerr << " inset: " << cursor.data_[i].inset_
		       << " idx: " << cursor.data_[i].idx_
		       << " text: " << cursor.data_[i].text_
		       << " par: " << cursor.data_[i].par_
		       << " pos: " << cursor.data_[i].pos_
		       << endl;
	}
}
