#include "math_inferinset.h"
#include "math_support.h"
#include "frontends/Painter.h"
#include "math_mathmlstream.h"
#include "textpainter.h"

using std::auto_ptr;


MathInferInset::MathInferInset()
	: MathGridInset(1, 1)
{}


auto_ptr<InsetBase> MathInferInset::clone() const
{
	return auto_ptr<InsetBase>(new MathInferInset(*this));
}


void MathInferInset::metrics(MetricsInfo &, Dimension &) const
{
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
