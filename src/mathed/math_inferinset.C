
#include "math_inferinset.h"
#include "math_support.h"
#include "frontends/Painter.h"
#include "math_mathmlstream.h"
#include "textpainter.h"


MathInferInset::MathInferInset()
	: MathGridInset(1, 1)
{}


MathInset * MathInferInset::clone() const
{
	return new MathInferInset(*this);
}


Dimension MathInferInset::metrics(MetricsInfo &) const
{
	return Dimension();
}


void MathInferInset::draw(PainterInfo &, int, int) const
{
}


void MathInferInset::write(WriteStream & os) const
{
	os << "\\infer";
	if (opt_.size())
		os << '[' << opt_ << ']';
	MathGridInset::write(os);
}
