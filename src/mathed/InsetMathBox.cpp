/**
 * \file InsetMathBox.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathBox.h"
#include "MathData.h"
#include "MathStream.h"
#include "MathStream.h"
#include "LaTeXFeatures.h"
#include "support/std_ostream.h"


namespace lyx {

using std::string;
using std::auto_ptr;


InsetMathBox::InsetMathBox(docstring const & name)
	: InsetMathNest(1), name_(name)
{}


auto_ptr<Inset> InsetMathBox::doClone() const
{
	return auto_ptr<Inset>(new InsetMathBox(*this));
}


void InsetMathBox::write(WriteStream & os) const
{
	os << '\\' << name_ << '{' << cell(0) << '}';
}


void InsetMathBox::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ';
	//text_->write(buffer(), os);
	os << "] ";
}


bool InsetMathBox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
	if (dim_ == dim)
		return false;
	dim_ = dim;
	return true;
}


void InsetMathBox::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, "textnormal");
	cell(0).draw(pi, x, y);
	drawMarkers(pi, x, y);
}


void InsetMathBox::infoize(odocstream & os) const
{
	os << "Box: " << name_;
}


void InsetMathBox::validate(LaTeXFeatures & features) const
{
	if (name_ == "tag" || name_ == "tag*")
		features.require("amsmath");
	cell(0).validate(features);
}


} // namespace lyx
