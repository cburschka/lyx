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


#include "support/lstrings.h"
#include "font_metrics.h"
#include "qfont_loader.h"
#include "debug.h"
#include "encoding.h"
#include "language.h"

#include <qfontmetrics.h>
#include <qfont.h>
 
namespace {

QFontMetrics const & metrics(LyXFont const & f)
{
	return fontloader.metrics(f);
}

 
int charwidth(Uchar val, LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return 1;
	return fontloader.charwidth(f, val); 
}

} // namespace anon


namespace font_metrics {

int maxAscent(LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return 1;
	return metrics(f).ascent();
}


int maxDescent(LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return 1;
	// We add 1 as the value returned by QT is different than X
	// See http://doc.trolltech.com/2.3/qfontmetrics.html#200b74
	return metrics(f).descent() + 1;
}


int ascent(char c, LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return 1;
	QRect const & r = metrics(f).boundingRect(c);
	return -r.top();
}


int descent(char c, LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return 1;
	QRect const & r = metrics(f).boundingRect(c);
	return r.bottom()+1;
}


int lbearing(char c, LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return 1;
	return metrics(f).leftBearing(c);
}


int rbearing(char c, LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return 1;
	QFontMetrics const & m(metrics(f));

	// Qt rbearing is from the right edge of the char's width().
	return (m.width(c) - m.rightBearing(c));
}


Encoding const * fontencoding(LyXFont const & f)
{
	Encoding const * encoding = f.language()->encoding();
	if (f.isSymbolFont())
		encoding = encodings.symbol_encoding();
	return encoding;
}
 

int smallcapswidth(char const * s, size_t ls, LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return 1;
	// handle small caps ourselves ...

	LyXFont smallfont(f);
	smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);

	QFontMetrics const & qm = fontloader.metrics(f);
	QFontMetrics const & qsmallm = fontloader.metrics(smallfont);

	Encoding const * encoding(fontencoding(f));
 
	int w = 0;

	for (size_t i = 0; i < ls; ++i) {
		QChar const c = QChar(encoding->ucs(s[i]));
		QChar const uc = c.upper();
		if (c != uc)
			w += qsmallm.width(uc);
		else
			w += qm.width(c);
	}
	return w;
}
 

int width(char const * s, size_t ls, LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return ls;

	if (f.realShape() == LyXFont::SMALLCAPS_SHAPE) {
		return smallcapswidth(s, ls, f);
	}
 
	Encoding const * encoding(fontencoding(f));

	if (ls == 1) {
		return charwidth(encoding->ucs(s[0]), f);
	}

	int w = 0;
 
	for (size_t i = 0; i < ls; ++i) {
		w += charwidth(encoding->ucs(s[i]), f);
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
