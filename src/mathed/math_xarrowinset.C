#include <config.h>


#include "math_xarrowinset.h"
#include "math_support.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


MathXArrowInset::MathXArrowInset(string const & name)
	: MathFracbaseInset(), name_(name)
{}


MathInset * MathXArrowInset::clone() const
{
	return new MathXArrowInset(*this);
}


void MathXArrowInset::metrics(MathMetricsInfo & mi) const
{
	MathScriptChanger dummy(mi.base);
	cell(0).metrics(mi);
	cell(1).metrics(mi);
	dim_.w = std::max(cell(0).width(), cell(1).width()) + 10;
	dim_.a = cell(0).height() + 10;
	dim_.d = cell(1).height();
}


void MathXArrowInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathScriptChanger dummy(pi.base);
	cell(0).draw(pi, x + 5, y - 10);
	cell(1).draw(pi, x + 5, y + cell(1).height());
	mathed_draw_deco(pi, x + 1, y - 7, width() - 2, 5, name_);
}


void MathXArrowInset::write(WriteStream & os) const
{
	os << '\\' << name_;
	if (cell(1).size())
		os << '[' << cell(1) << ']';
	os << '{' << cell(0) << '}';
}


void MathXArrowInset::normalize(NormalStream & os) const
{
	os << "[xarrow " << name_ << ' ' <<  cell(0) << ' ' << cell(1) << ']';
}
