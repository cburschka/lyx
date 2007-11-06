/**
 * \file InsetMathBox.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Ling Li (InsetMathMakebox)
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathBox.h"

#include "LaTeXFeatures.h"
#include "MathData.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "MetricsInfo.h"

#include "frontends/Painter.h"

#include <ostream>


namespace lyx {

/////////////////////////////////////////////////////////////////////
//
// InsetMathBox
//
/////////////////////////////////////////////////////////////////////

InsetMathBox::InsetMathBox(docstring const & name)
	: InsetMathNest(1), name_(name)
{}


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


void InsetMathBox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
	// Cache the inset dimension. 
	setDimCache(mi, dim);
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



/////////////////////////////////////////////////////////////////////
//
// InsetMathFBox
//
/////////////////////////////////////////////////////////////////////


InsetMathFBox::InsetMathFBox()
	: InsetMathNest(1)
{}


void InsetMathFBox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	cell(0).metrics(mi, dim);
	metricsMarkers(dim, 3); // 1 pixel space, 1 frame, 1 space
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetMathFBox::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	pi.pain.rectangle(x + 1, y - dim.ascent() + 1,
		dim.width() - 2, dim.height() - 2, Color_foreground);
	FontSetChanger dummy(pi.base, "textnormal");
	cell(0).draw(pi, x + 3, y);
	setPosCache(pi, x, y);
}


void InsetMathFBox::write(WriteStream & os) const
{
	os << "\\fbox{" << cell(0) << '}';
}


void InsetMathFBox::normalize(NormalStream & os) const
{
	os << "[fbox " << cell(0) << ']';
}


void InsetMathFBox::infoize(odocstream & os) const
{
	os << "FBox: ";
}


/////////////////////////////////////////////////////////////////////
//
// InsetMathFrameBox
//
/////////////////////////////////////////////////////////////////////


InsetMathFrameBox::InsetMathFrameBox()
	: InsetMathNest(3)
{}


void InsetMathFrameBox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	w_ = mathed_char_width(mi.base.font, '[');
	InsetMathNest::metrics(mi);
	dim  = cell(0).dimension(*mi.base.bv);
	dim += cell(1).dimension(*mi.base.bv);
	dim += cell(2).dimension(*mi.base.bv);
	metricsMarkers(dim);
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetMathFrameBox::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, "textnormal");
	Dimension const dim = dimension(*pi.base.bv);
	pi.pain.rectangle(x + 1, y - dim.ascent() + 1,
		dim.width() - 2, dim.height() - 2, Color_foreground);
	x += 5;
	BufferView const & bv = *pi.base.bv;

	drawStrBlack(pi, x, y, from_ascii("["));
	x += w_;
	cell(0).draw(pi, x, y);
	x += cell(0).dimension(bv).wid;
	drawStrBlack(pi, x, y, from_ascii("]"));
	x += w_ + 4;

	drawStrBlack(pi, x, y, from_ascii("["));
	x += w_;
	cell(1).draw(pi, x, y);
	x += cell(1).dimension(bv).wid;
	drawStrBlack(pi, x, y, from_ascii("]"));
	x += w_ + 4;

	cell(2).draw(pi, x, y);
	drawMarkers(pi, x, y);
}


void InsetMathFrameBox::write(WriteStream & os) const
{
	os << "\\framebox";
	os << '[' << cell(0) << ']';
	if (cell(1).size())
		os << '[' << cell(1) << ']';
	os << '{' << cell(2) << '}';
}


void InsetMathFrameBox::normalize(NormalStream & os) const
{
	os << "[framebox " << cell(0) << ' ' << cell(1) << ' ' << cell(2) << ']';
}



/////////////////////////////////////////////////////////////////////
//
// InsetMathBoxed
//
/////////////////////////////////////////////////////////////////////

InsetMathBoxed::InsetMathBoxed()
	: InsetMathNest(1)
{}


void InsetMathBoxed::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
	metricsMarkers2(dim, 3); // 1 pixel space, 1 frame, 1 space
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetMathBoxed::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	pi.pain.rectangle(x + 1, y - dim.ascent() + 1,
		dim.width() - 2, dim.height() - 2, Color_foreground);
	cell(0).draw(pi, x + 3, y);
	setPosCache(pi, x, y);
}


void InsetMathBoxed::write(WriteStream & os) const
{
	os << "\\boxed{" << cell(0) << '}';
}


void InsetMathBoxed::normalize(NormalStream & os) const
{
	os << "[boxed " << cell(0) << ']';
}


void InsetMathBoxed::infoize(odocstream & os) const
{
	os << "Boxed: ";
}


void InsetMathBoxed::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
}


/////////////////////////////////////////////////////////////////////
//
// InsetMathMakebox
//
/////////////////////////////////////////////////////////////////////


InsetMathMakebox::InsetMathMakebox()
	: InsetMathNest(3)
{}


void InsetMathMakebox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, from_ascii("textnormal"));
	w_ = mathed_char_width(mi.base.font, '[');
	InsetMathNest::metrics(mi);
	dim   = cell(0).dimension(*mi.base.bv);
	dim  += cell(1).dimension(*mi.base.bv);
	dim  += cell(2).dimension(*mi.base.bv);
	dim.wid += 4 * w_ + 4;
	metricsMarkers(dim);
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetMathMakebox::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, from_ascii("textnormal"));
	drawMarkers(pi, x, y);

	drawStrBlack(pi, x, y, from_ascii("["));
	x += w_;
	cell(0).draw(pi, x, y);
	x += cell(0).dimension(*pi.base.bv).width();
	drawStrBlack(pi, x, y, from_ascii("]"));
	x += w_ + 2;

	drawStrBlack(pi, x, y, from_ascii("["));
	x += w_;
	cell(1).draw(pi, x, y);
	x += cell(1).dimension(*pi.base.bv).wid;
	drawStrBlack(pi, x, y, from_ascii("]"));
	x += w_ + 2;

	cell(2).draw(pi, x, y);
	setPosCache(pi, x, y);
}


void InsetMathMakebox::write(WriteStream & os) const
{
	os << "\\makebox";
	os << '[' << cell(0) << ']';
	if (cell(1).size())
		os << '[' << cell(1) << ']';
	os << '{' << cell(2) << '}';
}


void InsetMathMakebox::normalize(NormalStream & os) const
{
	os << "[makebox " << cell(0) << ' ' << cell(1) << ' ' << cell(2) << ']';
}


void InsetMathMakebox::infoize(odocstream & os) const
{
	os << "Makebox (width: " << cell(0)
	    << " pos: " << cell(1) << ")";
}


} // namespace lyx
