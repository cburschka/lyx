/**
 * \file InsetMathSpecialChar.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathSpecialChar.h"

#include "MathSupport.h"
#include "MathStream.h"
#include "MetricsInfo.h"

#include "Dimension.h"
#include "LaTeXFeatures.h"
#include "TextPainter.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/lassert.h"


namespace lyx {


InsetMathSpecialChar::InsetMathSpecialChar(docstring name)
	: name_(name), kerning_(0)
{
	if (name.size() != 1) {
		if (name == "textasciicircum" || name == "mathcircumflex")
			char_ = '^';
		else if (name == "textasciitilde")
			char_ = '~';
		else if (name == "textbackslash")
			char_ = '\\';
		else
			LASSERT(false, /**/);
	} else
		char_ = name.at(0);
}



Inset * InsetMathSpecialChar::clone() const
{
	return new InsetMathSpecialChar(*this);
}


void InsetMathSpecialChar::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (char_ == ' ') {
		dim.asc = 4;
		dim.des = 0;
		dim.wid = 10;
	} else if (mi.base.fontname == "mathnormal") {
		ShapeChanger dummy(mi.base.font, UP_SHAPE);
		dim = theFontMetrics(mi.base.font).dimension(char_);
	} else {
		frontend::FontMetrics const & fm = theFontMetrics(mi.base.font);
		dim = fm.dimension(char_);
		kerning_ = fm.rbearing(char_) - dim.wid;
	}
}


void InsetMathSpecialChar::draw(PainterInfo & pi, int x, int y) const
{
	if (char_ == ' ') {
		int xp[4];
		int yp[4];
		int w = 10;

		xp[0] = ++x;        yp[0] = y - 3;
		xp[1] = x;          yp[1] = y;
		xp[2] = x + w - 2;  yp[2] = y;
		xp[3] = x + w - 2;  yp[3] = y - 3;

		pi.pain.lines(xp, yp, 4, Color_special);
	} else if (pi.base.fontname == "mathnormal") {
		ShapeChanger dummy(pi.base.font, UP_SHAPE);
		pi.draw(x, y, char_);
	} else {
		pi.draw(x, y, char_);
	}
}


void InsetMathSpecialChar::metricsT(TextMetricsInfo const &, Dimension & dim) const
{
	dim.wid = 1;
	dim.asc = 1;
	dim.des = 0;
}


void InsetMathSpecialChar::drawT(TextPainter & pain, int x, int y) const
{
	pain.draw(x, y, char_);
}


void InsetMathSpecialChar::write(WriteStream & os) const
{
	os << '\\' << name_;
	if (name_.size() != 1)
		os.pendingSpace(true);
}


void InsetMathSpecialChar::validate(LaTeXFeatures & features) const
{
	if (name_ == "mathcircumflex")
		features.require("mathcircumflex");
}


void InsetMathSpecialChar::normalize(NormalStream & os) const
{
	os << "[char ";
	os.os().put(char_);
	os << " mathalpha]";
}


void InsetMathSpecialChar::maple(MapleStream & os) const
{
	os.os().put(char_);
}


void InsetMathSpecialChar::mathematica(MathematicaStream & os) const
{
	os.os().put(char_);
}


void InsetMathSpecialChar::octave(OctaveStream & os) const
{
	os.os().put(char_);
}


void InsetMathSpecialChar::mathmlize(MathStream & ms) const
{
	switch (char_) {
	case '&':
		ms << "&amp;";
		break;
	default:
		ms.os().put(char_);
		break;
	}
}


} // namespace lyx
