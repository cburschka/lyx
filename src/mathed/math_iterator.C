
#include <config.h>

#include "math_iterator.h"
#include "debug.h"
#include "support/LAssert.h"


//MathIterator::MathIterator()
//{}


MathIterator::MathIterator(MathInset * p)
{
	push(p);
}


//MathIterator::MathIterator(MathCursor::cursor_type const & c)
//	: cursor_(c)
//{}


MathCursor::cursor_type const & MathIterator::cursor() const
{
	return cursor_;
}


MathInset const * MathIterator::par() const
{
	return position().par_;
}


MathInset * MathIterator::par()
{
	return position().par_;
}


MathXArray const & MathIterator::xcell() const
{
	return par()->xcell(position().idx_);
}


MathArray const & MathIterator::cell() const
{
	return par()->xcell(position().idx_).data_;
}


MathInset * MathIterator::nextInset() const
{
	if (position().pos_ == xcell().data_.size())
		return 0;
	return (xcell().begin() + position().pos_)->nucleus();
}


void MathIterator::push(MathInset * p)
{
	//lyxerr << "push: " << p << endl;
	cursor_.push_back(MathCursorPos(p));
}


void MathIterator::pop()
{
	//lyxerr << "pop: " << endl;
	lyx::Assert(cursor_.size());
	cursor_.pop_back();
}


MathCursorPos const & MathIterator::operator*() const
{
	return position();
}


MathCursorPos const & MathIterator::operator->() const
{
	return position();
}


void MathIterator::goEnd()
{
	position().idx_ = par()->nargs() - 1;
	position().pos_ = xcell().data_.size();
}


void MathIterator::operator++()
{
	// move into the current inset if possible
	// it is impossible for pos() == size()!
	MathInset * n = nextInset();
	if (n && n->isActive()) {
		push(n);
		return;
	}

	// otherwise move on one cell position if possible
	MathCursorPos & top = position();
	if (top.pos_ < top.par_->cell(top.idx_).size()) {
		// pos() == size() is valid!
		++top.pos_;
		return;
	}

	// otherwise move on one cell if possible
	if (top.idx_ + 1 < top.par_->nargs()) {
		// idx() == nargs() is _not_ valid!
		++top.idx_;
		top.pos_ = 0;
		return;
	}

	// otherwise leave array, move on one position
	// this might yield pos() == size(), but that's a ok.
	// it certainly invalidates top
	pop(); 
	++position().pos_;
}


void MathIterator::jump(MathInset::difference_type i)
{
	position().pos_ += i;
	//lyx::Assert(position().pos_ >= 0);
	lyx::Assert(position().pos_ <= cell().size());
}


bool operator==(MathIterator const & it, MathIterator const & jt)
{
	//lyxerr << "==: " << it.cursor().size() << " " << jt.cursor().size() << endl;
	if (it.cursor().size() != jt.cursor().size())
		return false;
	return it.cursor() == jt.cursor();	
}


bool operator!=(MathIterator const & it, MathIterator const & jt)
{
	//lyxerr << "!=: " << it.cursor().size() << " " << jt.cursor().size() << endl;
	if (it.cursor().size() != jt.cursor().size())
		return true;
	return it.cursor() != jt.cursor();	
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
