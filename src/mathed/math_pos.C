#include <config.h>

#include "math_pos.h"
#include "math_inset.h"
#include "debug.h"
#include "support/LAssert.h"
#include "support/LOstream.h"

using namespace lyx::support;

using std::endl;


CursorPos::CursorPos()
	: inset_(0), idx_(0), pos_(0)
{}


CursorPos::CursorPos(MathInset * p)
	: inset_(p), idx_(0), pos_(0)
{
	Assert(inset_);
}



MathArray & CursorPos::cell(MathArray::idx_type idx) const
{
	Assert(inset_);
	return inset_->cell(idx);
}


MathArray & CursorPos::cell() const
{
	Assert(inset_);
	return inset_->cell(idx_);
}


void CursorPos::getPos(int & x, int & y) const
{
	inset_->getPos(idx_, pos_, x, y);
}


void CursorPos::setPos(MathArray::pos_type pos)
{
	pos_ = pos;
}


std::ostream & operator<<(std::ostream & os, CursorPos const & p)
{
	os << "(par: " << p.inset_ << " idx: " << p.idx_ << " pos: " << p.pos_ << ')';
	return os;
}


bool operator==(CursorPos const & p, CursorPos const & q)
{
	return p.inset_ == q.inset_ && p.idx_ == q.idx_ && p.pos_ == q.pos_;
}


bool operator!=(CursorPos const & p, CursorPos const & q)
{
	return p.inset_ != q.inset_ || p.idx_ != q.idx_ || p.pos_ != q.pos_;
}


bool operator<(CursorPos const & p, CursorPos const & q)
{
	if (p.inset_ != q.inset_) {
		lyxerr << "can't compare cursor and anchor in different insets"
		       << endl;
		return true;
	}
	if (p.idx_ != q.idx_)
		return p.idx_ < q.idx_;
	return p.pos_ < q.pos_;
}
