/**
 * \file math_iterator.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_iterator.h"
#include "math_inset.h"
#include "math_data.h"

#include <boost/assert.hpp>


MathIterator::MathIterator()
{}


MathIterator::MathIterator(MathInset * p)
{
	push(p);
}



MathArray const & MathIterator::cell() const
{
	CursorSlice const & top = back();
	return top.asMathInset()->cell(top.idx_);
}



void MathIterator::push(MathInset * p)
{
	//lyxerr << "push: " << p << endl;
	push_back(CursorSlice(p));
}


void MathIterator::pop()
{
	//lyxerr << "pop: " << endl;
	BOOST_ASSERT(size());
	pop_back();
}


void MathIterator::goEnd()
{
	CursorSlice & top = back();
	top.idx_ = top.asMathInset()->nargs() - 1;
	top.pos_ = cell().size();
}


void MathIterator::operator++()
{
	CursorSlice & top = back();
	MathArray   & ar  = top.asMathInset()->cell(top.idx_);

	// move into the current inset if possible
	// it is impossible for pos() == size()!
	MathInset * n = 0;
	if (top.pos_ != ar.size())
		n = (ar.begin() + top.pos_)->nucleus();
	if (n && n->isActive()) {
		push(n);
		return;
	}

	// otherwise move on one cell back if possible
	if (top.pos_ < ar.size()) {
		// pos() == size() is valid!
		++top.pos_;
		return;
	}

	// otherwise try to move on one cell if possible
	while (top.idx_ + 1 < top.asMathInset()->nargs()) {
		// idx() == nargs() is _not_ valid!
		++top.idx_;
		if (top.asMathInset()->validCell(top.idx_)) {
			top.pos_ = 0;
			return;
		}
	}

	// otherwise leave array, move on one back
	// this might yield pos() == size(), but that's a ok.
	pop();
	// it certainly invalidates top
	++back().pos_;
}


bool MathIterator::normal() const
{
	return back().pos_ < cell().size();
}


void MathIterator::shrink(size_type i)
{
	if (i < size())
		erase(begin() + i, end());
}


bool operator==(MathIterator const & it, MathIterator const & jt)
{
	return MathIterator::base_type(it) == MathIterator::base_type(jt);
}


bool operator!=(MathIterator const & it, MathIterator const & jt)
{
	return MathIterator::base_type(it) != MathIterator::base_type(jt);
}


MathIterator ibegin(MathInset * p)
{
	return MathIterator(p);
}


MathIterator iend(MathInset * p)
{
	MathIterator it(p);
	it.goEnd();
	return it;
}
