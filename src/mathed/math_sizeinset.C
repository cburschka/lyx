#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_sizeinset.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "support/LOstream.h"


MathSizeInset::MathSizeInset(latexkeys const * l)
	: MathNestInset(1), key_(l)
{}


MathInset * MathSizeInset::clone() const
{
	return new MathSizeInset(*this);
}


void MathSizeInset::metrics(MathMetricsInfo & mi) const
{
	//MathStyleChanger dummy(mi.base, MathStyles(key_->id));
	xcell(0).metrics(mi);
	ascent_   = xcell(0).ascent() + 1;
	descent_  = xcell(0).descent() + 1;
	width_    = xcell(0).width() + 2;
}


void MathSizeInset::draw(MathPainterInfo & pi, int x, int y) const
{
	//MathStyleChanger dummy(pi.base, MathStyles(key_->id));
	xcell(0).draw(pi, x + 1, y);
	drawMarkers2(pi, x, y);
}


void MathSizeInset::write(WriteStream & os) const
{
	os << "{\\" << key_->name << ' ' << cell(0) << '}';
}


void MathSizeInset::normalize(NormalStream & os) const
{
	os << "[" << key_->name << ' ' << cell(0) << ']';
}


void MathSizeInset::infoize(std::ostream & os) const
{
	os << "Size: " << key_->name;
}
