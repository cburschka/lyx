/**
 * \file cursor_slice.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author André Pönitz
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "cursor_slice.h"
#include "debug.h"

#include "mathed/math_inset.h"
#include "mathed/math_data.h"

#include "insets/updatableinset.h"


#include <boost/assert.hpp>

using std::endl;


CursorSlice::CursorSlice()
	: inset_(0), idx_(0), par_(0), pos_(0), boundary_(false)
{}


CursorSlice::CursorSlice(InsetBase * p)
	: inset_(p), idx_(0), par_(0), pos_(0), boundary_(false)
{
	///BOOST_ASSERT(inset_);
}


void CursorSlice::idx(idx_type idx)
{
	idx_ = idx;
}


CursorSlice::idx_type CursorSlice::idx() const
{
	return idx_;
}


CursorSlice::idx_type & CursorSlice::idx()
{
	return idx_;
}


void CursorSlice::par(par_type par)
{
	par_ = par;
}


CursorSlice::par_type CursorSlice::par() const
{
	return par_;
}


CursorSlice::par_type & CursorSlice::par()
{
	return par_;
}


void CursorSlice::pos(pos_type pos)
{
	pos_ = pos;
}


CursorSlice::pos_type CursorSlice::pos() const
{
	return pos_;
}


CursorSlice::pos_type & CursorSlice::pos()
{
	return pos_;
}


CursorSlice::pos_type CursorSlice::lastpos() const
{
	BOOST_ASSERT(inset_);
#warning implement me for texted, too.
	return inset_->asMathInset() ? cell().size() : 0;
}


void CursorSlice::boundary(bool boundary)
{
	boundary_ = boundary;
}


bool CursorSlice::boundary() const
{
	return boundary_;
}


CursorSlice::row_type CursorSlice::row() const
{
	BOOST_ASSERT(asMathInset());
	asMathInset()->row(idx_);
}


CursorSlice::col_type CursorSlice::col() const
{
	BOOST_ASSERT(asMathInset());
	asMathInset()->col(idx_);
}


MathInset * CursorSlice::asMathInset() const
{
	return inset_ ? inset_->asMathInset() : 0;
}


UpdatableInset * CursorSlice::asUpdatableInset() const
{
	return inset_ ? inset_->asUpdatableInset() : 0;
}


MathArray & CursorSlice::cell() const
{
	BOOST_ASSERT(asMathInset());
	return asMathInset()->cell(idx_);
}


void CursorSlice::getScreenPos(int & x, int & y) const
{
	BOOST_ASSERT(asMathInset());
	asMathInset()->getScreenPos(idx_, pos_, x, y);
}


LyXText * CursorSlice::text() const
{
	return asUpdatableInset() ? asUpdatableInset()->getText(idx_) : 0;
}


bool operator==(CursorSlice const & p, CursorSlice const & q)
{
	return p.inset_ == q.inset_
	       && p.idx_ == q.idx_
	       && p.par_ == q.par_
	       && p.pos_ == q.pos_;
}


bool operator!=(CursorSlice const & p, CursorSlice const & q)
{
	return p.inset_ != q.inset_
	       || p.idx_ != q.idx_
	       || p.par_ != q.par_
	       || p.pos_ != q.pos_;
}


bool operator<(CursorSlice const & p, CursorSlice const & q)
{
	if (p.inset_ != q.inset_) {
		lyxerr << "can't compare cursor and anchor in different insets\n"
		       << "p: " << p << '\n' << "q: " << q << endl;
		return true;
	}
	if (p.idx_ != q.idx_)
		return p.idx_ < q.idx_;
	if (p.par_ != q.par_)
		return p.par_ < q.par_;
	return p.pos_ < q.pos_;
}


bool operator>(CursorSlice const & p, CursorSlice const & q)
{
	return q < p;
}


std::ostream & operator<<(std::ostream & os, CursorSlice const & item)
{
	os << " inset: " << item.inset_
	   << " text: " << item.text()
	   << " idx: " << item.idx_
	   << " par: " << item.par_
	   << " pos: " << item.pos_
//	   << " x: " << item.inset_->x()
//	   << " y: " << item.inset_->y()
;
	return os;
}




void increment(CursorBase & it)
{
	CursorSlice & top = it.back();
	MathArray   & ar  = top.asMathInset()->cell(top.idx_);

	// move into the current inset if possible
	// it is impossible for pos() == size()!
	MathInset * n = 0;
	if (top.pos_ != ar.size())
		n = (ar.begin() + top.pos_)->nucleus();
	if (n && n->isActive()) {
		it.push_back(CursorSlice(n));
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
	it.pop_back();
	// it certainly invalidates top
	++it.back().pos_;
}


CursorBase ibegin(InsetBase * p)
{
	CursorBase it;
	it.push_back(CursorSlice(p));
	return it;
}


CursorBase iend(InsetBase * p)
{
	CursorBase it;
	it.push_back(CursorSlice(p));
	CursorSlice & top = it.back();
	top.idx_ = top.asMathInset()->nargs() - 1;
	top.pos_ = top.asMathInset()->cell(top.idx_).size();
	return it;
}
