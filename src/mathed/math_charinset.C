#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_charinset.h"
#include "LColor.h"
#include "frontends/Painter.h"
#include "frontends/font_metrics.h"
#include "support/LOstream.h"
#include "debug.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "LaTeXFeatures.h"
#include "textpainter.h"

#include <cctype>
#include <cstring>


using std::ostream;
using std::endl;

#ifndef CXX_GLOBAL_CSTD
using std::strchr;
using std::isalpha;
#endif


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



MathInset * MathCharInset::clone() const
{
	return new MathCharInset(*this);
}


void MathCharInset::metrics(MathMetricsInfo & mi) const
{
#if 1
	if (slanted(char_) && mi.base.fontname == "mathnormal") {
		MathShapeChanger dummy(mi.base.font, LyXFont::ITALIC_SHAPE);
		mathed_char_dim(mi.base.font, char_, dim_);
	} else {
		mathed_char_dim(mi.base.font, char_, dim_);
	}
	if (isBinaryOp(char_))
		dim_.w += 2 * font_metrics::width(' ', mi.base.font);
#else
	whichFont(font_, code_, mi);
	mathed_char_dim(font_, char_, dim_);
	if (isBinaryOp(char_, code_))
		width_ += 2 * font_metrics::width(' ', font_);
	lyxerr << "MathCharInset::metrics: " << dim_ << "\n";
#endif
}


void MathCharInset::draw(MathPainterInfo & pi, int x, int y) const
{
	//lyxerr << "drawing '" << char_ << "' font: " << pi.base.fontname << endl;
	if (isBinaryOp(char_))
		x += font_metrics::width(' ', pi.base.font);
#if 1
	if (slanted(char_) && pi.base.fontname == "mathnormal") {
		MathShapeChanger dummy(pi.base.font, LyXFont::ITALIC_SHAPE);
		pi.draw(x, y, char_);
	} else {
		pi.draw(x, y, char_);
	}
#else
	drawChar(pain, font_, x, y, char_);
#endif
}


void MathCharInset::metricsT(TextMetricsInfo const &) const
{
	dim_.w = 1;
	dim_.a = 1;
	dim_.d = 0;
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
	os << "[char " << char_ << " " << "mathalpha" << "]";
}


void MathCharInset::octavize(OctaveStream & os) const
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
