/**
 * \file InsetMathDots.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathDots.h"

#include "Dimension.h"
#include "LaTeXFeatures.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "MathParser.h"
#include "MetricsInfo.h"

#include "frontends/FontMetrics.h"
#include "support/lassert.h"

namespace lyx {

InsetMathDots::InsetMathDots(latexkeys const * key)
	: dh_(0), key_(key)
{}


Inset * InsetMathDots::clone() const
{
	return new InsetMathDots(*this);
}


void InsetMathDots::metrics(MetricsInfo & mi, Dimension & dim) const
{
	dim = theFontMetrics(mi.base.font).dimension('X');
	dh_ = 0;
	if (key_->name == "cdots" || key_->name == "dotsb"
			|| key_->name == "dotsm" || key_->name == "dotsi")
		dh_ = dim.asc / 2;
	else if (key_->name == "dotsc" || key_->name == "ldots")
		dh_ = 2;
	else if (key_->name == "ddots" || key_->name == "adots"
			|| key_->name == "iddots" || key_->name == "vdots")
		dh_ = dim.asc;
}


void InsetMathDots::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	if (key_->name == "adots" || key_->name == "iddots")
		--y;
	else if (key_->name == "vdots")
		x += (dim.width() - 2) / 2;
	mathed_draw_deco(pi, x - 1, y - dh_, dim.width() - 2, dim.ascent(),
			key_->name);
}


docstring InsetMathDots::name() const
{
	return key_->name;
}


void InsetMathDots::validate(LaTeXFeatures & features) const
{
	if (!key_->required.empty())
		features.require(key_->required);
}


void InsetMathDots::mathmlize(MathMLStream & ms) const
{
	// which symbols we support is decided by what is listed in
	// lib/symbols as generating a dots inset
	docstring const & n = key_->name;
	std::string ent;
	if (ms.xmlMode()) {
		if (n == "dots" || n == "dotsc" || n == "dotso" || n == "ldots")
			ent = "&#x2026;";
		else if (n == "adots" || n == "iddots")
			ent = "&#x22F0;";
		else if (n == "cdots" || n == "dotsb" || n == "dotsi" || n == "dotsm")
			ent = "&#x22EF;";
		else if (n == "ddots")
			ent = "&#x22F1;";
		else if (n == "vdots")
			ent = "&#x22EE;";
		else LASSERT(false, ent = "&#x2026;");
	} else {
		if (n == "dots" || n == "dotsc" || n == "dotso" || n == "ldots")
			ent = "&hellip;";
		else if (n == "adots" || n == "iddots")
			ent = "&utdot;";
		else if (n == "cdots" || n == "dotsb" || n == "dotsi" || n == "dotsm")
			ent = "&ctdot;";
		else if (n == "ddots")
			ent = "&dtdot;";
		else if (n == "vdots")
			ent = "&vellip;";
		else LASSERT(false, ent = "&hellip;");
	}
	ms << MTag("mi") << from_ascii(ent) << ETag("mi");
}


void InsetMathDots::htmlize(HtmlStream & os) const
{
	// which symbols we support is decided by what is listed in
	// lib/symbols as generating a dots inset
	docstring const & n = key_->name;
	std::string ent;
	if (n == "dots" || n == "dotsc" || n == "dotso" || n == "ldots")
		ent = "&#x02026;";
	else if (n == "adots" || n == "iddots")
		ent = "&#x022F0;";
	else if (n == "cdots" || n == "dotsb" || n == "dotsi" || n == "dotsm")
		ent = "&#x022EF;";
	else if (n == "ddots")
		ent = "&#x022F1;";
	else if (n == "vdots")
		ent = "&#x022EE;";
	else
		LASSERT(false, ent = "#x02026;");
	os << from_ascii(ent);
}

} // namespace lyx
