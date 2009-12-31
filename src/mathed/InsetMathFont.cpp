/**
 * \file InsetMathFont.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathFont.h"

#include "LaTeXFeatures.h"
#include "MathData.h"
#include "MathStream.h"
#include "MathParser.h"
#include "MetricsInfo.h"

#include <ostream>


namespace lyx {

InsetMathFont::InsetMathFont(Buffer * buf, latexkeys const * key)
	: InsetMathNest(buf, 1), key_(key)
{}


Inset * InsetMathFont::clone() const
{
	return new InsetMathFont(*this);
}


InsetMath::mode_type InsetMathFont::currentMode() const
{
	if (key_->extra == "mathmode")
		return MATH_MODE;
	if (key_->extra == "textmode")
		return TEXT_MODE;
	return UNDECIDED_MODE;
}


bool InsetMathFont::lockedMode() const
{
	if (key_->extra == "forcetext")
		return true;
	return false;
}


void InsetMathFont::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, key_->name);
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
}


void InsetMathFont::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, key_->name.c_str());
	cell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
	setPosCache(pi, x, y);
}


void InsetMathFont::metricsT(TextMetricsInfo const & mi, Dimension &) const
{
	// FIXME: BROKEN!
	Dimension dim;
	cell(0).metricsT(mi, dim);
}


void InsetMathFont::drawT(TextPainter & pain, int x, int y) const
{
	cell(0).drawT(pain, x, y);
}


docstring InsetMathFont::name() const
{
	return key_->name;
}


void InsetMathFont::validate(LaTeXFeatures & features) const
{
	InsetMathNest::validate(features);
	// Make sure amssymb is put in preamble if Blackboard Bold or
	// Fraktur used:
	if (key_->name == "mathfrak" || key_->name == "mathbb")
		features.require("amssymb");
	if (key_->name == "text" || key_->name == "textnormal"
	    || (key_->name.length() == 6 && key_->name.substr(0, 4) == "text"))
		features.require("amstext");
	if (key_->name == "textipa")
		features.require("tipa");
	if (key_->name == "ce" || key_->name == "cf")
		features.require("mhchem");
}


// The fonts we want to support are listed in lib/symbols
void InsetMathFont::mathmlize(MathStream & os) const
{
	// FIXME These are not quite right, because they do not nest
	// correctly. A proper fix would presumably involve tracking
	// the fonts already in effect.
	std::string variant;
	if (key_->name == "mathnormal" || key_->name == "mathrm"
	    || key_->name == "text" || key_->name == "textnormal"
	    || key_->name == "textrm" || key_->name == "textup"
	    || key_->name == "textmd")
		variant = "normal";
	else if (key_->name == "frak" || key_->name == "mathfrak")
		variant = "fraktur";
	else if (key_->name == "mathbb" || key_->name == "mathbf"
	         || key_->name == "textbf")
		variant = "bold";
	else if (key_->name == "mathcal")
		variant == "script";
	else if (key_->name == "mathit" || key_->name == "textsl"
	         || key_->name == "emph")
		variant = "italic";
	else if (key_->name == "mathsf" || key_->name == "textit"
	         || key_->name == "textsf")
		variant = "sans-serif";
	else if (key_->name == "mathtt" || key_->name == "texttt")
		variant = "monospace";
	// no support at present for textipa, textsc, noun
	
	if (!variant.empty())
		os << "<mstyle mathvariant='" << from_utf8(variant) << "'>"
		   << cell(0) << "</mstyle>";
}


void InsetMathFont::infoize(odocstream & os) const
{
	os << "Font: " << key_->name;
}


} // namespace lyx
