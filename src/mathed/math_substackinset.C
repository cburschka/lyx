#include <config.h>

#include "math_substackinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "support/LOstream.h"


MathSubstackInset::MathSubstackInset()
	: MathGridInset(1, 1)
{}


MathInset * MathSubstackInset::clone() const
{
	return new MathSubstackInset(*this);
}


void MathSubstackInset::metrics(MetricsInfo & mi) const
{
	if (mi.base.style == LM_ST_DISPLAY) {
		StyleChanger dummy(mi.base, LM_ST_TEXT);
		MathGridInset::metrics(mi);
	} else {
		MathGridInset::metrics(mi);
	}
	metricsMarkers2();
}


void MathSubstackInset::draw(PainterInfo & pi, int x, int y) const
{
	MathGridInset::draw(pi, x + 1, y);
	drawMarkers2(pi, x, y);
}


void MathSubstackInset::infoize(std::ostream & os) const
{
	os << "Substack ";
}


void MathSubstackInset::write(WriteStream & os) const
{
	os << "\\substack{";
	MathGridInset::write(os);
	os << "}\n";
}


void MathSubstackInset::normalize(NormalStream & os) const
{
	os << "[substack ";
	MathGridInset::normalize(os);
	os << ']';
}


void MathSubstackInset::maple(MapleStream & os) const
{
	os << "substack(";
	MathGridInset::maple(os);
	os << ')';
}
