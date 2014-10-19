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

#include "Text.h"
#include "Paragraph.h"

#include "support/debug.h"

#include "insets/Inset.h"

#include "mathed/InsetMath.h"
#include "mathed/MathMacro.h"

#include "support/ExceptionMessage.h"
#include "support/gettext.h"
#include "support/lassert.h"

#include <ostream>

using namespace std;

namespace lyx {


CursorSlice::CursorSlice()
	: inset_(0), idx_(0), pit_(0), pos_(0)
{}


CursorSlice::CursorSlice(Inset & p)
	: inset_(&p), idx_(0), pit_(0), pos_(0)
{
	LBUFERR(inset_);
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
	LBUFERR(inset_);
	InsetMath const * math = inset_->asInsetMath();
	bool paramless_macro = math && math->asMacro() && !math->asMacro()->nargs();
	return math ? (paramless_macro ? 0 : cell().size()) 
		    : (text()->empty() ? 0 : paragraph().size());
}


pit_type CursorSlice::lastpit() const
{
	if (inset_->inMathed())
		return 0;
	return text()->paragraphs().size() - 1;
}


CursorSlice::row_type CursorSlice::row() const
{
	// LASSERT: This should only ever be called from an InsetMath.
	// Should we crash in release mode, though, or try to continue?
	LASSERT(asInsetMath(), /**/);
	return asInsetMath()->row(idx_);
}


CursorSlice::col_type CursorSlice::col() const
{
	// LASSERT: This should only ever be called from an InsetMath.
	// Should we crash in release mode, though, or try to continue?
	LASSERT(asInsetMath(), /**/);
	return asInsetMath()->col(idx_);
}


void CursorSlice::forwardPos()
{
	//  move on one position if possible
	if (pos_ < lastpos()) {
		//lyxerr << "... next pos" << endl;
		++pos_;
		return;
	}

	// otherwise move on one paragraph if possible
	if (pit_ < lastpit()) {
		//lyxerr << "... next par" << endl;
		++pit_;
		pos_ = 0;
		return;
	}

	// otherwise move on one cell
	//lyxerr << "... next idx" << endl;

	LASSERT(idx_ < nargs(), return);

	++idx_;
	pit_ = 0;
	pos_ = 0;
}


void CursorSlice::forwardIdx()
{
	LASSERT(idx_ < nargs(), return);

	++idx_;
	pit_ = 0;
	pos_ = 0;
}


void CursorSlice::backwardPos()
{
	if (pos_ != 0) {
		--pos_;
		return;
	}

	if (pit_ != 0) {
		--pit_;
		pos_ = lastpos();
		return;
	}

	if (idx_ != 0) {
		--idx_;
		pit_ = lastpit();
		pos_ = lastpos();
		return;
	}

	LATTEST(false);
}


bool CursorSlice::at_cell_end() const
{
	return pit_ == lastpit() && pos_ == lastpos();
}


bool CursorSlice::at_cell_begin() const
{
	return pit_ == 0 && pos_ == 0;
}


bool CursorSlice::at_end() const
{
	return idx_ == lastidx() && at_cell_end();
}


bool CursorSlice::at_begin() const
{
	return idx_ == 0 && at_cell_begin();
}


bool operator==(CursorSlice const & p, CursorSlice const & q)
{
	return p.inset_ == q.inset_
	       && p.idx_ == q.idx_
	       && p.pit_ == q.pit_
	       && p.pos_ == q.pos_;
}


bool operator!=(CursorSlice const & p, CursorSlice const & q)
{
	return p.inset_ != q.inset_
	       || p.idx_ != q.idx_
	       || p.pit_ != q.pit_
	       || p.pos_ != q.pos_;
}


bool operator<(CursorSlice const & p, CursorSlice const & q)
{
	if (p.inset_ != q.inset_) {
		LYXERR0("can't compare cursor and anchor in different insets\n"
		       << "p: " << p << '\n' << "q: " << q);
		// It should be safe to continue, just registering the error.
		LASSERT(false, return false);
	}
	if (p.idx_ != q.idx_)
		return p.idx_ < q.idx_;
	if (p.pit_ != q.pit_)
		return p.pit_ < q.pit_;
	return p.pos_ < q.pos_;
}


bool operator>(CursorSlice const & p, CursorSlice const & q)
{
	return q < p;
}


bool operator<=(CursorSlice const & p, CursorSlice const & q)
{
	return !(q < p);
}


ostream & operator<<(ostream & os, CursorSlice const & item)
{
	return os
	   << "inset: " << (void *)item.inset_
//	   << " text: " << item.text()
	   << " idx: " << item.idx_
	   << " par: " << item.pit_
	   << " pos: " << item.pos_
//	   << " x: " << item.inset_->x()
//	   << " y: " << item.inset_->y()
;
}


} // namespace lyx
