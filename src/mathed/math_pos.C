
#ifdef __GNUG__
#pragma implementation 
#endif

#include "config.h"
#include "math_pos.h"
#include "math_inset.h"
#include "debug.h"
#include "support/LAssert.h"
#include "support/LOstream.h"


MathCursorPos::MathCursorPos()
	: par_(0), idx_(0), pos_(0)
{}


MathCursorPos::MathCursorPos(MathInset * p)
	: par_(p), idx_(0), pos_(0)
{
	lyx::Assert(par_);
}



MathArray & MathCursorPos::cell(MathArray::idx_type idx) const
{
	lyx::Assert(par_);
	return par_->cell(idx);
}


MathArray & MathCursorPos::cell() const
{
	lyx::Assert(par_);
	return par_->cell(idx_);
}


void MathCursorPos::getPos(int & x, int & y) const
{
	par_->getPos(idx_, pos_, x, y);
}


std::ostream & operator<<(std::ostream & os, MathCursorPos const & p)
{
	os << "(par: " << p.par_ << " idx: " << p.idx_ << " pos: " << p.pos_ << ")";
	return os;
}


bool operator==(MathCursorPos const & p, MathCursorPos const & q)
{
	return p.par_ == q.par_ && p.idx_ == q.idx_ && p.pos_ == q.pos_;
}


bool operator!=(MathCursorPos const & p, MathCursorPos const & q)
{
	return p.par_ != q.par_ || p.idx_ != q.idx_ || p.pos_ != q.pos_;
}


bool operator<(MathCursorPos const & p, MathCursorPos const & q)
{
	if (p.par_ != q.par_) {
		lyxerr << "can't compare cursor and anchor in different insets\n";
		return true;
	}
	if (p.idx_ != q.idx_)
		return p.idx_ < q.idx_;
	return p.pos_ < q.pos_;
}
