/**
 * \file cursor.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
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
#include "insets/insettext.h"

#include <boost/assert.hpp>

using std::vector;
using std::endl;


std::ostream & operator<<(std::ostream & os, CursorItem const & item)
{
	os << " inset: " << item.inset_
	   << " idx: " << item.idx_
	   << " text: " << item.text_
	   << " par: " << item.par_
	   << " pos: " << item.pos_;
	return os;
}


std::ostream & operator<<(std::ostream & os, LCursor const & cursor)
{
	for (size_t i = 0, n = cursor.data_.size(); i != n; ++i)
		os << "   " << cursor.data_[i];
	return os;
}


DispatchResult LCursor::dispatch(FuncRequest const & cmd)
{
	for (int i = data_.size() - 1; i >= 0; --i) {
		CursorItem & citem = data_[i];

		lyxerr << "trying to dispatch to inset" << citem.inset_ << endl;
		DispatchResult res = citem.inset_->dispatch(cmd);
		lyxerr << "   result: " << res.val() << endl;

		switch (res.val()) {
			case FINISHED:
				pop();
				return DispatchResult(true, true);

			case FINISHED_RIGHT: {
				pop();
				//InsetText * inset = static_cast<InsetText *>(innerInset());
				//if (inset)
				//	inset->moveRightIntern(bv_, false, false);
				//else
				//	bv_->text->cursorRight(bv_);
				innerText()->cursorRight(bv_);
				return DispatchResult(true);
			}

			case FINISHED_UP: {
				pop();
				//InsetText * inset = static_cast<InsetText *>(inset());
				//if (inset)
				//	result = inset->moveUp(bv);
				return DispatchResult(true);
			}

			case FINISHED_DOWN: {
				pop();
				//InsetText * inset = static_cast<InsetText *>(inset());
				//if (inset)
				// 	result = inset->moveDown(bv);
				return DispatchResult(true);
			}

			default:
				break;
		}

		lyxerr << "# unhandled result: " << res.val() << endl;
	}

	lyxerr << "trying to dispatch to main text " << bv_->text << endl;
	DispatchResult res = bv_->text->dispatch(cmd);
	lyxerr << "   result: " << res.val() << endl;
	return res;
}


LCursor::LCursor(BufferView * bv)
	: bv_(bv)
{}


void LCursor::push(InsetOld * inset, LyXText * text)
{
	data_.push_back(CursorItem(inset, text));
}


void LCursor::pop()
{
	BOOST_ASSERT(!data_.empty());
	data_.pop_back();
}


InsetOld * LCursor::innerInset() const
{
	return data_.empty() ? 0 : data_.back().inset_;
}


LyXText * LCursor::innerText() const
{
	return data_.empty() ? bv_->text : data_.back().text_;
}
