#include "math_diminset.h"
#include "Lsstream.h"
#include "textpainter.h"


void MathDimInset::metricsT(TextMetricsInfo const &) const
{
	std::ostringstream os;
	os << *this;
	dim_.w = int(os.str().size());
	dim_.a = 1;
	dim_.d = 0;
}


void MathDimInset::drawT(TextPainter & pain, int x, int y) const
{
	std::ostringstream os;
	os << *this;
	pain.draw(x, y, os.str().c_str());
}
