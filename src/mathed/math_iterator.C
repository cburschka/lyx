
#include <config.h>

#include "math_iterator.h"
#include "debug.h"
#include "support/LAssert.h"


//MathIterator::MathIterator()
//{}


MathIterator::MathIterator(MathAtom & t)
{
	push(t);
}


MathIterator::MathIterator(MathCursor::cursor_type const & c)
	: cursor_(c)
{}


MathCursorPos const & MathIterator::position() const
{
	lyx::Assert(cursor_.size());
	return cursor_.back();
}


MathCursorPos & MathIterator::position()
{
	lyx::Assert(cursor_.size());
	return cursor_.back();
}


MathCursor::cursor_type const & MathIterator::cursor() const
{
	return cursor_;
}


MathAtom const & MathIterator::par() const
{
	return *(position().par_);
}


MathAtom & MathIterator::par()
{
	return *(position().par_);
}


MathXArray const & MathIterator::xcell() const
{
	return par()->xcell(position().idx_);
}


MathAtom * MathIterator::nextInset() const
{
	if (position().pos_ == xcell().data_.size())
		return 0;
	return const_cast<MathAtom *>(&*(xcell().begin() + position().pos_));
}


void MathIterator::push(MathAtom & t)
{
	//lyxerr << "push: " << p << endl;
	cursor_.push_back(MathCursorPos(t));
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
	if (nextInset() && nextInset()->nucleus()->isActive()) {
		push(*nextInset());
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

	// otherwise leave array, move on one position
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



MathIterator ibegin(MathAtom & t)
{
	return MathIterator(t);
}


MathIterator iend(MathAtom & t)
{
	MathIterator it(t);
	it.goEnd();
	return it;
}
