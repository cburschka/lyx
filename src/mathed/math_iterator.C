
#include <config.h>

#include "debug.h"
#include "math_iterator.h"


MathIterator::MathIterator()
{}


MathIterator::MathIterator(MathInset * p)
{
	push(p);
}


MathIterator::MathIterator(MathCursor::cursor_type const & c)
	: cursor_(c)
{}


MathCursorPos const & MathIterator::position() const
{
	return cursor_.back();
}


MathCursorPos & MathIterator::position()
{
	return cursor_.back();
}


MathCursor::cursor_type const & MathIterator::cursor() const
{
	return cursor_;
}


MathInset * MathIterator::par() const
{
	return cursor_.size() ? cursor_.back().par_ : 0;
}


MathXArray const & MathIterator::xcell() const
{
	if (!par())
		lyxerr << "MathIterator::xcell: no cell\n";
	return par()->xcell(position().idx_);
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


void MathIterator::operator++()
{
	// move into the current inset if possible
	// it is impossible for pos() == size()!
	if (nextInset() && nextInset()->isActive()) {
		push(nextInset());
		return;
	}

	// otherwise move on one cell position if possible
	if (position().pos_ < xcell().data_.size()) {
		// pos() == size() is valid!
		++position().pos_;
		return;
	}

	// otherwise move on one cell if possible
	if (position().idx_ + 1 < par()->nargs()) {
		// idx() == nargs() is _not_ valid!
		++position().idx_;
		position().pos_ = 0;
		return;
	}

	// otherwise leave array, move on one cell
	// this might yield pos() == size(), but that's a ok.
	pop();
	++position().pos_;
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

