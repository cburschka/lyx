
#include "math_diminset.h"
#include "Lsstream.h"
#include "textpainter.h"


void MathDimInset::metricsT(TextMetricsInfo const &) const
{
#ifndef WITH_WARNINGS
#warning temporarily disabled
#endif
/*
	std::ostringstream os;
	os << MathAtom(this);
	dim_.wid = int(os.str().size());
	dim_.asc = 1;
	dim_.des = 0;
*/
}


void MathDimInset::drawT(TextPainter &, int, int) const
{
/*
	std::ostringstream os;
	os << MathAtom(this);
	pain.draw(x, y, STRCONV(os.str()));
*/
}
