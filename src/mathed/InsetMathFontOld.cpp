/**
 * \file InsetMathFontOld.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathFontOld.h"

#include "MathData.h"
#include "MathParser.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "MetricsInfo.h"

#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include <ostream>

using namespace lyx::support;

namespace lyx {

InsetMathFontOld::InsetMathFontOld(Buffer * buf, latexkeys const * key)
	: InsetMathNest(buf, 1), key_(key), current_mode_(TEXT_MODE)
{
	//lock(true);
}


std::string InsetMathFontOld::font() const
{
	LASSERT(isAscii(key_->name), return "mathnormal");
	return to_ascii(key_->name);
}


Inset * InsetMathFontOld::clone() const
{
	return new InsetMathFontOld(*this);
}


void InsetMathFontOld::metrics(MetricsInfo & mi, Dimension & dim) const
{
	current_mode_ = isTextFont(mi.base.fontname)
				? TEXT_MODE : MATH_MODE;

	std::string const fontname = current_mode_ == MATH_MODE
		? "math" + font() : "text" + font();// I doubt that this still works

	// When \cal is used in text mode, the font is not changed
	bool really_change_font = fontname != "textcal";

	Changer dummy = mi.base.changeFontSet(fontname, really_change_font);
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
}


void InsetMathFontOld::draw(PainterInfo & pi, int x, int y) const
{
	current_mode_ = isTextFont(pi.base.fontname)
				? TEXT_MODE : MATH_MODE;

	std::string const fontname = current_mode_ == MATH_MODE
		? "math" + font() : "text" + font();// I doubt that this still works

	// When \cal is used in text mode, the font is not changed
	bool really_change_font = fontname != "textcal";

	Changer dummy = pi.base.changeFontSet(fontname, really_change_font);
	cell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void InsetMathFontOld::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	cell(0).metricsT(mi, dim);
}


void InsetMathFontOld::drawT(TextPainter & pain, int x, int y) const
{
	cell(0).drawT(pain, x, y);
}


void InsetMathFontOld::write(WriteStream & os) const
{
	os << "{\\" << key_->name << ' ' << cell(0) << '}';
}


void InsetMathFontOld::normalize(NormalStream & os) const
{
	os << "[font " << key_->name << ' ' << cell(0) << ']';
}


void InsetMathFontOld::infoize(odocstream & os) const
{
	os << bformat(_("Font: %1$s"), key_->name);
}


} // namespace lyx
