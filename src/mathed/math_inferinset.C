#ifdef __GNUG__
#pragma implementation
#endif

#include "math_inferinset.h"
#include "math_support.h"
#include "frontends/Painter.h"
#include "math_mathmlstream.h"
#include "textpainter.h"


using std::max;


MathInferInset::MathInferInset()
	: MathGridInset(1, 1)
{}


MathInset * MathInferInset::clone() const
{
	return new MathInferInset(*this);
}


void MathInferInset::metrics(MathMetricsInfo &) const
{
}


void MathInferInset::draw(MathPainterInfo &, int, int) const
{
}


void MathInferInset::write(WriteStream & os) const
{
	os << "\\infer";
	if (opt_.size())
		os << "[" << opt_ << "]";
	MathGridInset::write(os);
}


