/**
 * \file math_macro.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_macro.h"
#include "math_support.h"
#include "math_extern.h"
#include "math_mathmlstream.h"

#include "buffer.h"
#include "cursor.h"
#include "debug.h"
#include "BufferView.h"
#include "LaTeXFeatures.h"

using std::string;
using std::max;
using std::auto_ptr;
using std::endl;



MathMacro::MathMacro(string const & name)
	: name_(name)
{}


auto_ptr<InsetBase> MathMacro::clone() const
{
	return auto_ptr<InsetBase>(new MathMacro(*this));
}


string MathMacro::name() const
{
	return name_;
}


void MathMacro::setExpansion(MathArray const & exp, MathArray const & arg) const
{
	expanded_ = exp;
	args_ = arg;
}


void MathMacro::metrics(MetricsInfo & mi, Dimension & dim) const
{
	LyXFont font = mi.base.font;
	augmentFont(font, "lyxtex");
	mathed_string_dim(font, "\\" + name(), dim);
	dim_ = dim;
}


void MathMacro::metricsExpanded(MetricsInfo & mi, Dimension & dim) const
{
	args_.metrics(mi);
	expanded_.metrics(mi, dim);
	dim.wid -= args_.size() ? args_.width() : 0;
	dim_ = dim;
}


void MathMacro::draw(PainterInfo & pi, int x, int y) const
{
	LyXFont font = pi.base.font;
	augmentFont(font, "lyxtex");
	drawStr(pi, font, x, y, "\\" + name());
	setPosCache(pi, x, y);
}


void MathMacro::drawExpanded(PainterInfo & pi, int x, int y) const
{
	expanded_.draw(pi, x, y);
	drawMarkers2(pi, x, y);
}


int MathMacro::widthExpanded() const
{
	return expanded_.width();
}


void MathMacro::validate(LaTeXFeatures & features) const
{
	if (name() == "binom" || name() == "mathcircumflex")
		features.require(name());
}


void MathMacro::maple(MapleStream & os) const
{
	updateExpansion();
	::maple(expanded_, os);
}


void MathMacro::mathmlize(MathMLStream & os) const
{
	updateExpansion();
	::mathmlize(expanded_, os);
}


void MathMacro::octave(OctaveStream & os) const
{
	updateExpansion();
	::octave(expanded_, os);
}


void MathMacro::updateExpansion() const
{
#warning FIXME
	//expand();
	//expanded_.substitute(*this);
}


void MathMacro::infoize(std::ostream & os) const
{
	os << "Macro: " << name();
}


void MathMacro::infoize2(std::ostream & os) const
{
	os << "Macro: " << name();

}
