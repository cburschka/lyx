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

#include <boost/assert.hpp>

using std::endl;


CursorSlice::CursorSlice()
	: inset_(0), idx_(0), pit_(0), pos_(0)
{}


CursorSlice::CursorSlice(InsetBase & p)
	: inset_(&p), idx_(0), pit_(0), pos_(0)
{
	BOOST_ASSERT(inset_);
}


MathArray & CursorSlice::cell() const
{
	return inset_->asMathInset()->cell(idx_);
}


Paragraph & CursorSlice::paragraph()
{
	return text()->getPar(pit_);
}


Paragraph const & CursorSlice::paragraph() const
{
	return text()->getPar(pit_);
}


CursorSlice::pos_type CursorSlice::lastpos() const
{
	BOOST_ASSERT(inset_);
	return inset_->asMathInset() ? cell().size() : paragraph().size();
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


bool operator==(CursorSlice const & p, CursorSlice const & q)
{
	return &p.inset() == &q.inset()
	       && p.idx() == q.idx()
	       && p.pit() == q.pit()
	       && p.pos() == q.pos();
}


bool operator!=(CursorSlice const & p, CursorSlice const & q)
{
	return &p.inset() != &q.inset()
	       || p.idx() != q.idx()
	       || p.pit() != q.pit()
	       || p.pos() != q.pos();
}


bool operator<(CursorSlice const & p, CursorSlice const & q)
{
	if (&p.inset() != &q.inset()) {
		lyxerr << "can't compare cursor and anchor in different insets\n"
		       << "p: " << p << '\n' << "q: " << q << endl;
		BOOST_ASSERT(false);
	}
	if (p.idx() != q.idx())
		return p.idx() < q.idx();
	if (p.pit() != q.pit())
		return p.pit() < q.pit();
	return p.pos() < q.pos();
}


bool operator>(CursorSlice const & p, CursorSlice const & q)
{
	return q < p;
}


bool operator<=(CursorSlice const & p, CursorSlice const & q)
{
	return !(q < p);
}


std::ostream & operator<<(std::ostream & os, CursorSlice const & item)
{
	return os
	   << "inset: " << &item.inset()
//	   << " text: " << item.text()
	   << " idx: " << item.idx()
	   << " par: " << item.pit()
	   << " pos: " << item.pos()
//	   << " x: " << item.inset().x()
//	   << " y: " << item.inset().y()
;
}
