/**
 * \file qfont_metrics.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#ifdef __GNUG__
#pragma implementation "frontends/font_metrics.h"
#endif

#include "support/lstrings.h"
#include "font_metrics.h"
#include "qfont_loader.h"
#include "debug.h"
#include "encoding.h"
#include "language.h"

#include <qfontmetrics.h>
#include <qfont.h>

namespace {
	QFontMetrics const & metrics(LyXFont const & f) {
		return fontloader.metrics(f);
	}
}


namespace font_metrics {

int maxAscent(LyXFont const & f)
{
	return metrics(f).ascent();
}


int maxDescent(LyXFont const & f)
{
	return metrics(f).descent();
}


int ascent(char c, LyXFont const & f)
{
	QRect r = metrics(f).boundingRect(c);
	return abs(r.top());
}


int descent(char c, LyXFont const & f)
{
	QRect r = metrics(f).boundingRect(c);
	return abs(r.bottom());
}


int lbearing(char c, LyXFont const & f)
{
	lyxerr << "lb of " << c << " is " << metrics(f).leftBearing(c)
	       << std::endl;
	return metrics(f).leftBearing(c);
}


int rbearing(char c, LyXFont const & f)
{
	QFontMetrics const & m(metrics(f));

	// Qt rbearing is from the right edge of the char's width().
	return (m.width(c) - m.rightBearing(c));
}


int width(char const * s, size_t ls, LyXFont const & f)
{
	Encoding const * encoding = f.language()->encoding();
	if (f.isSymbolFont())
		encoding = encodings.symbol_encoding();

	QString str;
	str.setLength(ls);
	for (size_t i = 0; i < ls; ++i)
		str[i] = QChar(encoding->ucs(s[i]));

	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		return metrics(f).width(str);
	}

	// handle small caps ourselves ...

	LyXFont smallfont(f);
	smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);

	QFontMetrics qm = fontloader.metrics(f);
	QFontMetrics qsmallm = fontloader.metrics(smallfont);

	int w = 0;

	for (size_t i = 0; i < ls; ++i) {
		QChar const c = str[i].upper();
		if (c != str[i])
			w += qsmallm.width(c);
		else
			w += qm.width(c);
	}
	return w;
}


int signedWidth(string const & s, LyXFont const & f)
{
	if (s[0] == '-')
		return -width(s.substr(1, s.length() - 1), f);
	else
		return width(s, f);
}


void rectText(string const & str, LyXFont const & f,
	int & w,
	int & ascent,
	int & descent)
{
	QFontMetrics const & m(metrics(f));

	static int const d = 2;

	w = width(str, f) + d * 2 + 2;
	ascent = m.ascent() + d;
	descent = m.descent() + d;
}



void buttonText(string const & str, LyXFont const & f,
	int & w,
	int & ascent,
	int & descent)
{
	QFontMetrics const & m(metrics(f));

	static int const d = 3;

	w = width(str, f) + d * 2 + 2;
	ascent = m.ascent() + d;
	descent = m.descent() + d;
}

} // namespace font_metrics
