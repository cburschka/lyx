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
#include "lyxtext.h"
#include "paragraph.h"

#include "mathed/math_inset.h"
#include "mathed/math_data.h"

#include "insets/updatableinset.h"


#include <boost/assert.hpp>

using std::endl;


CursorSlice::CursorSlice()
	: inset_(0), idx_(0), par_(0), pos_(0), boundary_(false)
{}


CursorSlice::CursorSlice(InsetBase & p)
	: inset_(&p), idx_(0), par_(0), pos_(0), boundary_(false)
{
	BOOST_ASSERT(inset_);
}


size_t CursorSlice::nargs() const
{
	BOOST_ASSERT(inset_);
	return inset_->nargs();
}


size_t CursorSlice::nrows() const
{
	BOOST_ASSERT(inset_);
	return inset_->nrows();
}


size_t CursorSlice::ncols() const
{
	BOOST_ASSERT(inset_);
	return inset_->ncols();
}


CursorSlice::idx_type CursorSlice::idx() const
{
	return idx_;
}


CursorSlice::idx_type & CursorSlice::idx()
{
	return idx_;
}


CursorSlice::par_type CursorSlice::par() const
{
	return par_;
}


CursorSlice::par_type & CursorSlice::par()
{
	return par_;
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
	return inset_->asMathInset() ? cell().size() : paragraph().size();
}


bool CursorSlice::boundary() const
{
	return boundary_;
}


bool & CursorSlice::boundary()
{
	return boundary_;
}


CursorSlice::row_type CursorSlice::row() const
{
	BOOST_ASSERT(asMathInset());
	return asMathInset()->row(idx_);
}


CursorSlice::col_type CursorSlice::col() const
{
	BOOST_ASSERT(asMathInset());
	return asMathInset()->col(idx_);
}


MathInset * CursorSlice::asMathInset() const
{
	BOOST_ASSERT(inset_);
	return inset_->asMathInset();
}


UpdatableInset * CursorSlice::asUpdatableInset() const
{
	BOOST_ASSERT(inset_);
	return inset_->asUpdatableInset();
}


MathArray & CursorSlice::cell() const
{
	BOOST_ASSERT(asMathInset());
	return asMathInset()->cell(idx_);
}


LyXText * CursorSlice::text() const
{
	BOOST_ASSERT(inset_);
	return inset_->getText(idx_);
}


Paragraph & CursorSlice::paragraph()
{
	// access to the main lyx text must be handled in the cursor
	BOOST_ASSERT(text());
	return *text()->getPar(par_);
}


Paragraph const & CursorSlice::paragraph() const
{
	// access to the main lyx text must be handled in the cursor
	BOOST_ASSERT(text());
	return *text()->getPar(par_);
}


bool operator==(CursorSlice const & p, CursorSlice const & q)
{
	return &p.inset() == &q.inset()
	       && p.idx() == q.idx()
	       && p.par() == q.par()
	       && p.pos() == q.pos();
}


bool operator!=(CursorSlice const & p, CursorSlice const & q)
{
	return &p.inset() != &q.inset()
	       || p.idx() != q.idx()
	       || p.par() != q.par()
	       || p.pos() != q.pos();
}


bool operator<(CursorSlice const & p, CursorSlice const & q)
{
	if (&p.inset() != &q.inset()) {
		lyxerr << "can't compare cursor and anchor in different insets\n"
		       << "p: " << p << '\n' << "q: " << q << endl;
		return true;
	}
	if (p.idx() != q.idx())
		return p.idx() < q.idx();
	if (p.par() != q.par())
		return p.par() < q.par();
	return p.pos() < q.pos();
}


bool operator>(CursorSlice const & p, CursorSlice const & q)
{
	return q < p;
}


std::ostream & operator<<(std::ostream & os, CursorSlice const & item)
{
	return os
	   << "inset: " << &item.inset()
//	   << " text: " << item.text()
	   << " idx: " << item.idx()
	   << " par: " << item.par()
	   << " pos: " << item.pos()
//	   << " x: " << item.inset().x()
//	   << " y: " << item.inset().y()
;
}
