#include <config.h>


#include "math_fontoldinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "math_parser.h"
#include "textpainter.h"
#include "support/LOstream.h"
#include "frontends/Painter.h"



MathFontOldInset::MathFontOldInset(latexkeys const * key)
	: MathNestInset(1), key_(key)
{
	//lock(true);
}


MathInset * MathFontOldInset::clone() const
{
	return new MathFontOldInset(*this);
}


void MathFontOldInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, key_->name.c_str());
	cell(0).metrics(mi, dim_);
	metricsMarkers(1);
	dim = dim_;
}


void MathFontOldInset::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, key_->name.c_str());
	cell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void MathFontOldInset::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	cell(0).metricsT(mi, dim);
}


void MathFontOldInset::drawT(TextPainter & pain, int x, int y) const
{
	cell(0).drawT(pain, x, y);
}


void MathFontOldInset::write(WriteStream & os) const
{
	os << "{\\" << key_->name << ' ' << cell(0) << '}';
}


void MathFontOldInset::normalize(NormalStream & os) const
{
	os << "[font " << key_->name << ' ' << cell(0) << ']';
}


void MathFontOldInset::infoize(std::ostream & os) const
{
	os << "Font: " << key_->name;
}
