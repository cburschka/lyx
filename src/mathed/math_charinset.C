/**
 * \file math_charinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_charinset.h"
#include "dimension.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "textpainter.h"

using std::auto_ptr;

#ifndef CXX_GLOBAL_CSTD
using std::strchr;
#endif


extern bool has_math_fonts;

namespace {

	bool isBinaryOp(char c)
	{
		return strchr("+-<>=/*", c);
	}


	bool slanted(char c)
	{
		//if (strchr("0123456789;:!|[]().,?+/-*<>=", c)
		return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
	}

}


MathCharInset::MathCharInset(char c)
	: char_(c)
{}



auto_ptr<InsetBase> MathCharInset::clone() const
{
	return auto_ptr<InsetBase>(new MathCharInset(*this));
}


void MathCharInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
#if 1
	if (char_ == '=' && has_math_fonts) {
		FontSetChanger dummy(mi.base, "cmr");
		mathed_char_dim(mi.base.font, char_, dim);
	} else if ((char_ == '>' || char_ == '<') && has_math_fonts) {
		FontSetChanger dummy(mi.base, "cmm");
		mathed_char_dim(mi.base.font, char_, dim);
	} else if (!slanted(char_) && mi.base.fontname == "mathnormal") {
		ShapeChanger dummy(mi.base.font, LyXFont::UP_SHAPE);
		mathed_char_dim(mi.base.font, char_, dim);
	} else {
		mathed_char_dim(mi.base.font, char_, dim);
	}
	int const em = mathed_char_width(mi.base.font, 'M');
	if (isBinaryOp(char_))
		dim.wid += static_cast<int>(0.5*em+0.5);
	else if (char_ == '\'')
		dim.wid += static_cast<int>(0.1667*em+0.5);
#else
	whichFont(font_, code_, mi);
	mathed_char_dim(font_, char_, dim_);
	if (isBinaryOp(char_, code_))
		width_ += 2 * font_metrics::width(' ', font_);
	lyxerr << "MathCharInset::metrics: " << dim << endl;
#endif
}


void MathCharInset::draw(PainterInfo & pi, int x, int y) const
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
		ShapeChanger dummy(pi.base.font, LyXFont::UP_SHAPE);
		pi.draw(x, y, char_);
	} else {
		pi.draw(x, y, char_);
	}
#else
	drawChar(pain, font_, x, y, char_);
#endif
}


void MathCharInset::metricsT(TextMetricsInfo const &, Dimension & dim) const
{
	dim.wid = 1;
	dim.asc = 1;
	dim.des = 0;
}


void MathCharInset::drawT(TextPainter & pain, int x, int y) const
{
	//lyxerr << "drawing text '" << char_ << "' code: " << code_ << endl;
	pain.draw(x, y, char_);
}


void MathCharInset::write(WriteStream & os) const
{
	os << char_;
}


void MathCharInset::normalize(NormalStream & os) const
{
	os << "[char " << char_ << ' ' << "mathalpha]";
}


void MathCharInset::octave(OctaveStream & os) const
{
	os << char_;
}


bool MathCharInset::isRelOp() const
{
	return char_ == '=' || char_ == '<' || char_ == '>';
}


bool MathCharInset::match(MathInset const * p) const
{
	MathCharInset const * q = p->asCharInset();
	return q && char_ == q->char_;
}
