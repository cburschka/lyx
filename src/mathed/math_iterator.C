#include <config.h>

#include "math_iterator.h"
#include "math_inset.h"
#include "debug.h"
#include "support/LAssert.h"


MathIterator::MathIterator()
{}


MathIterator::MathIterator(MathInset * p)
{
	push(p);
}


MathInset const * MathIterator::par() const
{
	return back().par_;
}


MathInset * MathIterator::par()
{
	return back().par_;
}


MathArray const & MathIterator::cell() const
{
	MathCursorPos const & top = back();
	return top.par_->cell(top.idx_);
}



void MathIterator::push(MathInset * p)
{
	//lyxerr << "push: " << p << endl;
	push_back(MathCursorPos(p));
}


void MathIterator::pop()
{
	//lyxerr << "pop: " << endl;
	lyx::Assert(size());
	pop_back();
}


MathCursorPos const & MathIterator::operator*() const
{
	return back();
}


MathCursorPos const & MathIterator::operator->() const
{
	return back();
}


void MathIterator::goEnd()
{
	MathCursorPos & top = back();
	top.idx_ = top.par_->nargs() - 1;
	top.pos_ = cell().size();
}


void MathIterator::operator++()
{
	MathCursorPos   & top = back();
	MathArray & ar  = top.par_->cell(top.idx_);

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
	while (top.idx_ + 1 < top.par_->nargs()) {
		// idx() == nargs() is _not_ valid!
		++top.idx_;
		if (top.par_->validCell(top.idx_)) {
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


void MathIterator::jump(difference_type i)
{
	back().pos_ += i;
	//lyx::Assert(back().pos_ >= 0);
	lyx::Assert(back().pos_ <= cell().size());
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
