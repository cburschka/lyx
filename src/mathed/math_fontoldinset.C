#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

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


void MathFontOldInset::metrics(MathMetricsInfo & mi) const
{
	MathFontSetChanger dummy(mi.base, key_->name.c_str());
	dim_ = cell(0).metrics(mi);
	metricsMarkers();
}


void MathFontOldInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathFontSetChanger dummy(pi.base, key_->name.c_str());
	cell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void MathFontOldInset::metricsT(TextMetricsInfo const & mi) const
{
	dim_ = cell(0).metricsT(mi);
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
	os << "[font " << key_->name << " " << cell(0) << "]";
}


void MathFontOldInset::infoize(std::ostream & os) const
{
	os << "Font: " << key_->name;
}
