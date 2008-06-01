/**
 * \file InsetMathChar.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathChar.h"

#include "MathSupport.h"
#include "MathStream.h"
#include "MetricsInfo.h"

#include "Dimension.h"
#include "Encoding.h"
#include "LaTeXFeatures.h"
#include "TextPainter.h"

#include "frontends/FontMetrics.h"

#include "support/debug.h"
#include "support/lstrings.h"


namespace lyx {

extern bool has_math_fonts;


static bool isBinaryOp(char_type c)
{
	return support::contains("+-<>=/*", static_cast<char>(c));
}


static bool slanted(char_type c)
{
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}


InsetMathChar::InsetMathChar(char_type c)
	: char_(c), kerning_(0)
{}



Inset * InsetMathChar::clone() const
{
	return new InsetMathChar(*this);
}


void InsetMathChar::metrics(MetricsInfo & mi, Dimension & dim) const
{
#if 1
	if (char_ == '=' && has_math_fonts) {
		FontSetChanger dummy(mi.base, "cmr");
		dim = theFontMetrics(mi.base.font).dimension(char_);
	} else if ((char_ == '>' || char_ == '<') && has_math_fonts) {
		FontSetChanger dummy(mi.base, "cmm");
		dim = theFontMetrics(mi.base.font).dimension(char_);
	} else if (!slanted(char_) && mi.base.fontname == "mathnormal") {
		ShapeChanger dummy(mi.base.font, UP_SHAPE);
		dim = theFontMetrics(mi.base.font).dimension(char_);
	} else {
		frontend::FontMetrics const & fm = theFontMetrics(mi.base.font);
		dim = fm.dimension(char_);
		kerning_ = fm.rbearing(char_) - dim.wid;
	}
	int const em = mathed_char_width(mi.base.font, 'M');
	if (isBinaryOp(char_))
		dim.wid += static_cast<int>(0.5*em+0.5);
	else if (char_ == '\'')
		dim.wid += static_cast<int>(0.1667*em+0.5);
#else
	whichFont(font_, code_, mi);
	dim = theFontMetrics(font_).dimension(char_);
	if (isBinaryOp(char_, code_))
		dim.wid += 2 * theFontMetrics(font_).width(' ');
	lyxerr << "InsetMathChar::metrics: " << dim << endl;
#endif
}


void InsetMathChar::draw(PainterInfo & pi, int x, int y) const
{
	//lyxerr << "drawing '" << char_ << "' font: " << pi.base.fontname << endl;
	int const em = mathed_char_width(pi.base.font, 'M');
	if (isBinaryOp(char_))
		x += static_cast<int>(0.25*em+0.5);
	else if (char_ == '\'')
		x += static_cast<int>(0.0833*em+0.5);
#if 1
	if (char_ == '=' && has_math_fonts) {
		FontSetChanger dummy(pi.base, "cmr");
		pi.draw(x, y, char_);
	} else if ((char_ == '>' || char_ == '<') && has_math_fonts) {
		FontSetChanger dummy(pi.base, "cmm");
		pi.draw(x, y, char_);
	} else if (!slanted(char_) && pi.base.fontname == "mathnormal") {
		ShapeChanger dummy(pi.base.font, UP_SHAPE);
		pi.draw(x, y, char_);
	} else {
		pi.draw(x, y, char_);
	}
#else
	drawChar(pain, font_, x, y, char_);
#endif
}


void InsetMathChar::metricsT(TextMetricsInfo const &, Dimension & dim) const
{
	dim.wid = 1;
	dim.asc = 1;
	dim.des = 0;
}


void InsetMathChar::drawT(TextPainter & pain, int x, int y) const
{
	//lyxerr << "drawing text '" << char_ << "' code: " << code_ << endl;
	pain.draw(x, y, char_);
}


void InsetMathChar::write(WriteStream & os) const
{
	os.os().put(char_);
}


void InsetMathChar::validate(LaTeXFeatures & features) const
{
	if (char_ >= 0x80) {
		encodings.validate(char_, features);
		features.require("relsize");
		features.require("mathsym");
	}
}


void InsetMathChar::normalize(NormalStream & os) const
{
	os << "[char ";
	os.os().put(char_);
	os << " mathalpha]";
}


void InsetMathChar::octave(OctaveStream & os) const
{
	os.os().put(char_);
}


void InsetMathChar::mathmlize(MathStream & ms) const
{
	switch (char_) {
		case '<': ms << "&lt;"; break;
		case '>': ms << "&gt;"; break;
		case '&': ms << "&amp;"; break;
		default: ms.os().put(char_); break;
	}
}


bool InsetMathChar::isRelOp() const
{
	return char_ == '=' || char_ == '<' || char_ == '>';
}


} // namespace lyx
