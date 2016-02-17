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
#include "BufferEncodings.h"
#include "LaTeXFeatures.h"
#include "TextPainter.h"

#include "frontends/FontMetrics.h"

#include "support/debug.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/textutils.h"


namespace lyx {

extern bool has_math_fonts;


static bool isBinaryOp(char_type c)
{
	return support::contains("+-<>=/*", static_cast<char>(c));
}


static bool slanted(char_type c)
{
	return isAlphaASCII(c) || Encodings::isMathAlpha(c);
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
	int const em = mathed_font_em(mi.base.font);
	if (isBinaryOp(char_))
		dim.wid += support::iround(0.5 * em);
	else if (char_ == '\'')
		dim.wid += support::iround(0.1667 * em);
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
	//lyxerr << "drawing '" << char_ << "' font: " << pi.base.fontname << std::endl;
	int const em = mathed_font_em(pi.base.font);
	if (isBinaryOp(char_))
		x += support::iround(0.25 * em);
	else if (char_ == '\'')
		x += support::iround(0.0833 * em);
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
	if (!isASCII(char_))
		BufferEncodings::validate(char_, features, true);
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


// We have a bit of a problem here. MathML wants to know whether the
// character represents an "identifier" or an "operator", and we have
// no general way of telling. So we shall guess: If it's alpha or 
// mathalpha, then we'll treat it as an identifier, otherwise as an 
// operator.
// Worst case: We get bad spacing, or bad italics.
void InsetMathChar::mathmlize(MathStream & ms) const
{
	std::string entity;
	switch (char_) {
		case '<': entity = "&lt;"; break;
		case '>': entity = "&gt;"; break;
		case '&': entity = "&amp;"; break;
		case ' ': {
			ms << from_ascii("&nbsp;");
			return;
		}
		default: break;
	}
	
	if (ms.inText()) {
		if (entity.empty())
			ms.os().put(char_);
		else 
			ms << from_ascii(entity);
		return;
	}

	if (!entity.empty()) {
		ms << "<mo>" << from_ascii(entity) << "</mo>";
		return;
	}		

	char const * type = 
		(isAlphaASCII(char_) || Encodings::isMathAlpha(char_))
			? "mi" : "mo";
	// we don't use MTag and ETag because we do not want the spacing
	ms << "<" << type << ">" << char_type(char_) << "</" << type << ">";	
}


void InsetMathChar::htmlize(HtmlStream & ms) const
{
	std::string entity;
	switch (char_) {
		case '<': entity = "&lt;"; break;
		case '>': entity = "&gt;"; break;
		case '&': entity = "&amp;"; break;
		case ' ': entity = "&nbsp;"; break;
		default: break;
	}
	
	bool have_entity = !entity.empty();
	
	if (ms.inText()) {
		if (have_entity)
			ms << from_ascii(entity);
		else
			ms.os().put(char_);
		return;
	}
	
	if (have_entity) {
		// an operator, so give some space
		ms << ' ' << from_ascii(entity) << ' ';
		return;
	}		

	if (isAlphaASCII(char_) || Encodings::isMathAlpha(char_))
		// we don't use MTag and ETag because we do not want the spacing
		ms << MTag("i") << char_type(char_) << ETag("i");
	else
		// an operator, so give some space
		ms << " " << char_type(char_) << " ";
}


bool InsetMathChar::isRelOp() const
{
	return char_ == '=' || char_ == '<' || char_ == '>';
}


} // namespace lyx
