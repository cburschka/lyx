/**
 * \file math_commentinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_commentinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_support.h"
#include "support/std_ostream.h"

using std::string;
using std::auto_ptr;


MathCommentInset::MathCommentInset()
	: MathNestInset(1)
{}


MathCommentInset::MathCommentInset(string const & str)
	: MathNestInset(1)
{
	asArray(str, cell(0));
}


auto_ptr<InsetBase> MathCommentInset::clone() const
{
	return auto_ptr<InsetBase>(new MathCommentInset(*this));
}


void MathCommentInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
	dim_ = dim;
}


void MathCommentInset::draw(PainterInfo & pi, int x, int y) const
{
	cell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void MathCommentInset::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	cell(0).metricsT(mi, dim);
}


void MathCommentInset::drawT(TextPainter & pain, int x, int y) const
{
	cell(0).drawT(pain, x, y);
}


void MathCommentInset::write(WriteStream & os) const
{
	os << '%' << cell(0) << "\n";
}


void MathCommentInset::maple(MapleStream & os) const
{
	os << '#' << cell(0) << "\n";
}


void MathCommentInset::mathematica(MathematicaStream &) const
{}


void MathCommentInset::octave(OctaveStream &) const
{}


void MathCommentInset::mathmlize(MathMLStream & os) const
{
	os << MTag("comment") << cell(0) << cell(1) << ETag("comment");
}


void MathCommentInset::infoize(std::ostream & os) const
{
	os << "Comment";
}
