/**
 * \file math_sizeinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_sizeinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_parser.h"
#include "math_streamstr.h"
#include "support/std_ostream.h"

using std::atoi;

using std::auto_ptr;


MathSizeInset::MathSizeInset(latexkeys const * l)
	: MathNestInset(1), key_(l), style_(Styles(atoi(l->extra.c_str())))
{}


auto_ptr<InsetBase> MathSizeInset::clone() const
{
	return auto_ptr<InsetBase>(new MathSizeInset(*this));
}


void MathSizeInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	StyleChanger dummy(mi.base, style_);
	cell(0).metrics(mi, dim_);
	metricsMarkers();
	dim = dim_;
}


void MathSizeInset::draw(PainterInfo & pi, int x, int y) const
{
	StyleChanger dummy(pi.base, style_);
	cell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void MathSizeInset::write(WriteStream & os) const
{
	os << "{\\" << key_->name << ' ' << cell(0) << '}';
}


void MathSizeInset::normalize(NormalStream & os) const
{
	os << '[' << key_->name << ' ' << cell(0) << ']';
}


void MathSizeInset::infoize(std::ostream & os) const
{
	os << "Size: " << key_->name;
}
