/**
 * \file math_braceinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_braceinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_support.h"
#include "LColor.h"
#include "support/std_ostream.h"
#include "frontends/Painter.h"

using std::max;
using std::auto_ptr;


MathBraceInset::MathBraceInset()
	: MathNestInset(1)
{}


MathBraceInset::MathBraceInset(MathArray const & ar)
	: MathNestInset(1)
{
	cell(0) = ar;
}


auto_ptr<InsetBase> MathBraceInset::doClone() const
{
	return auto_ptr<InsetBase>(new MathBraceInset(*this));
}


void MathBraceInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi);
	Dimension t;
	mathed_char_dim(mi.base.font, '{', t);
	dim.asc = max(cell(0).ascent(), t.asc);
	dim.des = max(cell(0).descent(), t.des);
	dim.wid = cell(0).width() + 2 * t.wid;
	metricsMarkers(dim);
	dim_ = dim;
}


void MathBraceInset::draw(PainterInfo & pi, int x, int y) const
{
	LyXFont font = pi.base.font;
	font.setColor(LColor::latex);
	Dimension t;
	mathed_char_dim(font, '{', t);
	pi.pain.text(x, y, '{', font);
	cell(0).draw(pi, x + t.wid, y);
	pi.pain.text(x + t.wid + cell(0).width(), y, '}', font);
	drawMarkers(pi, x, y);
}


void MathBraceInset::write(WriteStream & os) const
{
	os << '{' << cell(0) << '}';
}


void MathBraceInset::normalize(NormalStream & os) const
{
	os << "[block " << cell(0) << ']';
}


void MathBraceInset::maple(MapleStream & os) const
{
	os << cell(0);
}


void MathBraceInset::octave(OctaveStream & os) const
{
	os << cell(0);
}


void MathBraceInset::mathmlize(MathMLStream & os) const
{
	os << MTag("mrow") << cell(0) << ETag("mrow");
}


void MathBraceInset::mathematica(MathematicaStream & os) const
{
	os << cell(0);
}


void MathBraceInset::infoize(std::ostream & os) const
{
	os << "Nested Block: ";
}
