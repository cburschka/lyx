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


void CursorSlice::par(lyx::paroffset_type par)
{
	par_ = par;
}


lyx::paroffset_type CursorSlice::par() const
{
	return par_;
}


void CursorSlice::pos(lyx::pos_type pos)
{
	pos_ = pos;
}


lyx::pos_type CursorSlice::pos() const
{
	return pos_;
}


void CursorSlice::boundary(bool boundary)
{
	boundary_ = boundary;
}


bool CursorSlice::boundary() const
{
	return boundary_;
}


MathInset * CursorSlice::asMathInset() const
{
	return static_cast<MathInset *>(const_cast<InsetBase *>(inset_));
}


UpdatableInset * CursorSlice::asUpdatableInset() const
{
	return static_cast<UpdatableInset *>(const_cast<InsetBase *>(inset_));
}


MathArray & CursorSlice::cell(CursorSlice::idx_type idx) const
{
	BOOST_ASSERT(inset_);
	return asMathInset()->cell(idx);
}


MathArray & CursorSlice::cell() const
{
	BOOST_ASSERT(inset_);
	return asMathInset()->cell(idx_);
}


void CursorSlice::getPos(int & x, int & y) const
{
	asMathInset()->getPos(idx_, pos_, x, y);
}


void CursorSlice::setPos(int pos)
{
	pos_ = pos;
}


LyXText * CursorSlice::text() const
{
	return asUpdatableInset()->getText(idx_);
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


//std::ostream & operator<<(std::ostream & os, CursorSlice const & p)
//{
//	os << "(par: " << p.inset_ << " idx: " << p.idx_ << " pos: " << p.pos_ << ')';
//	return os;
//}


std::ostream & operator<<(std::ostream & os, CursorSlice const & item)
{
	os << " inset: " << item.inset_
//	   << " text: " << item.text()
	   << " idx: " << item.idx_
//	   << " par: " << item.par_
//	   << " pos: " << item.pos_
//	   << " x: " << item.inset_->x()
//	   << " y: " << item.inset_->y()
;
	return os;
}


