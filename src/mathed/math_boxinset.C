#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_boxinset.h"
#include "support/LOstream.h"
#include "debug.h"
#include "Painter.h"
#include "math_cursor.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"

MathBoxInset::MathBoxInset(string const & name)
	: MathGridInset(1, 1), name_(name)
{}


MathInset * MathBoxInset::clone() const
{
	return new MathBoxInset(*this);
}


void MathBoxInset::write(WriteStream & os) const
{
	os << "\\" << name_ << "{" << cell(0) << "}";
}


void MathBoxInset::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ';
	//text_->write(buffer(), os);
	os << "] ";
}


void MathBoxInset::rebreak()
{
	//lyxerr << "trying to rebreak...\n";
}


void MathBoxInset::draw(Painter & pain, int x, int y) const
{
	MathGridInset::draw(pain, x, y);
	if (mathcursor && mathcursor->isInside(this)) {
		pain.rectangle(x - 1, y - ascent() - 1, width(), height(),
			LColor::mathframe);
	}
}
