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
#include "iterators.h"
#include "lyxtext.h"
#include "paragraph.h"

#include "insets/updatableinset.h"

using std::vector;


DispatchResult Cursor::dispatch(FuncRequest const &)
{
	for (int i = data_.size() - 1; i >= 0; --i) {
		lyxerr << "trying to dispatch to " << data_[i].text_ << std::endl;
	}
	return UNDISPATCHED;
}


void buildCursor(Cursor & cursor, BufferView & bv)
{
	UpdatableInset * inset = bv.theLockingInset();
	lyxerr << "\nbuildCursor: " << inset << std::endl;
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
		lyxerr << " tli not found! inset: " << inset << std::endl;
		return;
	}

	vector<ParagraphList::iterator> pits; 
	vector<ParagraphList const *>   plists;
	vector<LyXText *>               texts;
/*
	pit.getPits(pits, plists, texts);

	cursor.data_.resize(pits.size());
	for (size_t i = 0, n = pits.size(); i != n; ++i) {
		cursor.data_[i].text_ = texts[i];
		cursor.data_[i].pit_  = pits[i];
		//cursor.data_[i].pos_ = texts[i]->cursor.pos();
		cursor.data_[i].pos_ = 0;
		lyxerr << " text: " << cursor.data_[i].text_
		       << " pit: " << cursor.data_[i].pit_->id()
		       << " pos: " << cursor.data_[i].pos_
		       << std::endl;
	}
*/
}
