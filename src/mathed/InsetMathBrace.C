/**
 * \file InsetMathBrace.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathBrace.h"
#include "MathData.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "LColor.h"
#include "support/std_ostream.h"
#include "frontends/Painter.h"


namespace lyx {

using std::max;
using std::auto_ptr;


InsetMathBrace::InsetMathBrace()
	: InsetMathNest(1)
{}


InsetMathBrace::InsetMathBrace(MathArray const & ar)
	: InsetMathNest(1)
{
	cell(0) = ar;
}


auto_ptr<InsetBase> InsetMathBrace::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathBrace(*this));
}


void InsetMathBrace::metrics(MetricsInfo & mi, Dimension & dim) const
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


void InsetMathBrace::draw(PainterInfo & pi, int x, int y) const
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


void InsetMathBrace::write(WriteStream & os) const
{
	os << '{' << cell(0) << '}';
}


void InsetMathBrace::normalize(NormalStream & os) const
{
	os << "[block " << cell(0) << ']';
}


void InsetMathBrace::maple(MapleStream & os) const
{
	os << cell(0);
}


void InsetMathBrace::octave(OctaveStream & os) const
{
	os << cell(0);
}


void InsetMathBrace::mathmlize(MathStream & os) const
{
	os << MTag("mrow") << cell(0) << ETag("mrow");
}


void InsetMathBrace::mathematica(MathematicaStream & os) const
{
	os << cell(0);
}


void InsetMathBrace::infoize(odocstream & os) const
{
	os << "Nested Block: ";
}


} // namespace lyx
