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

using std::atoi;

MathSizeInset::MathSizeInset(latexkeys const * l)
	: MathNestInset(1), key_(l), style_(MathStyles(atoi(l->extra.c_str())))
{}


MathInset * MathSizeInset::clone() const
{
	return new MathSizeInset(*this);
}


void MathSizeInset::metrics(MathMetricsInfo & mi) const
{
	MathStyleChanger dummy(mi.base, style_);
	dim_ = xcell(0).metrics(mi);
	metricsMarkers2();
}


void MathSizeInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathStyleChanger dummy(pi.base, style_);
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
