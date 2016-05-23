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
#include "support/lstrings.h"

#include <ostream>

using namespace lyx::support;

namespace lyx {

InsetMathFontOld::InsetMathFontOld(Buffer * buf, latexkeys const * key)
	: InsetMathNest(buf, 1), key_(key), current_mode_(TEXT_MODE)
{
	//lock(true);
}


Inset * InsetMathFontOld::clone() const
{
	return new InsetMathFontOld(*this);
}


void InsetMathFontOld::metrics(MetricsInfo & mi, Dimension & dim) const
{
	current_mode_ = isTextFont(from_ascii(mi.base.fontname))
				? TEXT_MODE : MATH_MODE;

	docstring const font = current_mode_ == MATH_MODE
				? "math" + key_->name : "text" + key_->name;

	// When \cal is used in text mode, the font is not changed
	bool really_change_font = font != "textcal";

	Changer dummy = mi.base.changeFontSet(font, really_change_font);
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
}


void InsetMathFontOld::draw(PainterInfo & pi, int x, int y) const
{
	current_mode_ = isTextFont(from_ascii(pi.base.fontname))
				? TEXT_MODE : MATH_MODE;

	docstring const font = current_mode_ == MATH_MODE
				? "math" + key_->name : "text" + key_->name;

	// When \cal is used in text mode, the font is not changed
	bool really_change_font = font != "textcal";

	Changer dummy = pi.base.changeFontSet(font, really_change_font);
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
