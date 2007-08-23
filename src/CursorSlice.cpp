/**
 * \file CursorSlice.cpp
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

#include "CursorSlice.h"

#include "debug.h"
#include "Text.h"
#include "Paragraph.h"

#include "insets/Inset.h"

#include "mathed/InsetMath.h"
#include "mathed/MathData.h"

#include <boost/assert.hpp>

using std::endl;


namespace lyx {


CursorSlice::CursorSlice()
	: inset_(0), idx_(0), pit_(0), pos_(0)
{}


CursorSlice::CursorSlice(Inset & p)
	: inset_(&p), idx_(0), pit_(0), pos_(0)
{
	BOOST_ASSERT(inset_);
}


MathData & CursorSlice::cell() const
{
	return inset_->asInsetMath()->cell(idx_);
}


Paragraph & CursorSlice::paragraph() const
{
	return text()->getPar(pit_);
}


pos_type CursorSlice::lastpos() const
{
	BOOST_ASSERT(inset_);
	return inset_->asInsetMath() ? cell().size() : paragraph().size();
}


pit_type CursorSlice::lastpit() const
{
	if (inset().inMathed())
		return 0;
	return text()->paragraphs().size() - 1;
}


CursorSlice::row_type CursorSlice::row() const
{
	BOOST_ASSERT(asInsetMath());
	return asInsetMath()->row(idx_);
}


CursorSlice::col_type CursorSlice::col() const
{
	BOOST_ASSERT(asInsetMath());
	return asInsetMath()->col(idx_);
}


void CursorSlice::forwardPos()
{
	//  move on one position if possible
	if (pos() < lastpos()) {
		//lyxerr << "... next pos" << endl;
		++pos();
		return;
	}

	// otherwise move on one paragraph if possible
	if (pit() < lastpit()) {
		//lyxerr << "... next par" << endl;
		++pit();
		pos() = 0;
		return;
	}

	// otherwise move on one cell
	//lyxerr << "... next idx" << endl;

	BOOST_ASSERT(idx() < nargs());

	++idx();
	pit() = 0;
	pos() = 0;
}


void CursorSlice::forwardIdx()
{
	BOOST_ASSERT(idx() < nargs());

	++idx();
	pit() = 0;
	pos() = 0;
}


void CursorSlice::backwardPos()
{
	if (pos() != 0) {
		--pos();
		return;
	}

	if (pit() != 0) {
		--pit();
		pos() = lastpos();
		return;
	}

	if (idx() != 0) {
		--idx();
		pit() = lastpit();
		pos() = lastpos();
		return;
	}

	BOOST_ASSERT(false);
}


bool CursorSlice::at_end() const 
{
	return idx() == lastidx() && pit() == lastpit() && pos() == lastpos();
}


bool CursorSlice::at_begin() const
{
	return idx() == 0 && pit() == 0 && pos() == 0;
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


} // namespace lyx
