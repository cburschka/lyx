#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_commentinset.h"
#include "math_mathmlstream.h"
#include "LaTeXFeatures.h"
#include "support/LOstream.h"
#include "textpainter.h"



MathCommentInset::MathCommentInset()
	: MathNestInset(1)
{}


MathCommentInset::MathCommentInset(string const & str)
	: MathNestInset(1)
{
	cell(0) = asArray(str);
}


MathInset * MathCommentInset::clone() const
{
	return new MathCommentInset(*this);
}


void MathCommentInset::metrics(MathMetricsInfo & mi) const
{
	dim_ = cell(0).metrics(mi);
	metricsMarkers();
}


void MathCommentInset::draw(MathPainterInfo & pi, int x, int y) const
{
	cell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void MathCommentInset::metricsT(TextMetricsInfo const & mi) const
{
	dim_ = cell(0).metricsT(mi);
}


void MathCommentInset::drawT(TextPainter & pain, int x, int y) const
{
	cell(0).drawT(pain, x, y);
}


void MathCommentInset::write(WriteStream & os) const
{
	os << '%' << cell(0) << "\n";
}


void MathCommentInset::maplize(MapleStream & os) const
{}


void MathCommentInset::mathematicize(MathematicaStream & os) const
{}


void MathCommentInset::octavize(OctaveStream & os) const
{}


void MathCommentInset::mathmlize(MathMLStream & os) const
{
	os << MTag("comment") << cell(0) << cell(1) << ETag("comment");
}


void MathCommentInset::infoize(std::ostream & os) const
{
	os << "Comment";
}
