/**
 * \file math_xarrowinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_xarrowinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"

using std::auto_ptr;


MathXArrowInset::MathXArrowInset(string const & name)
	: MathFracbaseInset(), name_(name)
{}


auto_ptr<InsetBase> MathXArrowInset::clone() const
{
	return auto_ptr<InsetBase>(new MathXArrowInset(*this));
}


void MathXArrowInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	ScriptChanger dummy(mi.base);
	cell(0).metrics(mi);
	cell(1).metrics(mi);
	dim_.wid = std::max(cell(0).width(), cell(1).width()) + 10;
	dim_.asc = cell(0).height() + 10;
	dim_.des = cell(1).height();
	dim = dim_;
}


void MathXArrowInset::draw(PainterInfo & pi, int x, int y) const
{
	ScriptChanger dummy(pi.base);
	cell(0).draw(pi, x + 5, y - 10);
	cell(1).draw(pi, x + 5, y + cell(1).height());
	mathed_draw_deco(pi, x + 1, y - 7, pi.width - 2, 5, name_);
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
