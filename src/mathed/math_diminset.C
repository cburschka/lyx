
#include "math_diminset.h"
#include "Lsstream.h"
#include "textpainter.h"


void MathDimInset::metrics(TextMetricsInfo const &) const
{
	std::ostringstream os;
	os << *this;
	width_   = int(os.str().size());
	ascent_  =	1;
	descent_ = 0;
}


void MathDimInset::draw(TextPainter & pain, int x, int y) const
{
	std::ostringstream os;
	os << *this;
	pain.draw(x, y, os.str().c_str());
}
