/**
 * \file qfont_metrics.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "frontends/font_metrics.h"
#include "frontends/lyx_gui.h"

#include "qfont_loader.h"

#include "language.h"

#include "support/unicode.h"

using lyx::char_type;
using lyx::docstring;

using std::string;


namespace {

int smallcapswidth(unsigned short const * s, size_t ls, LyXFont const & f)
{
	if (!lyx_gui::use_gui)
		return 1;
	// handle small caps ourselves ...

	LyXFont smallfont = f;
	smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);

	QFontMetrics const & qm = fontloader.metrics(f);
	QFontMetrics const & qsmallm = fontloader.metrics(smallfont);

	int w = 0;

	for (size_t i = 0; i < ls; ++i) {
		QChar const c = s[i];
		QChar const uc = c.upper();
		if (c != uc)
			w += qsmallm.width(uc);
		else
			w += qm.width(c);
	}
	return w;
}


} // anon namespace


int font_metrics::maxAscent(LyXFont const & f)
{
	if (!lyx_gui::use_gui)
		return 1;
	return fontloader.metrics(f).ascent();
}


int font_metrics::maxDescent(LyXFont const & f)
{
	if (!lyx_gui::use_gui)
		return 1;
	// We add 1 as the value returned by QT is different than X
	// See http://doc.trolltech.com/2.3/qfontmetrics.html#200b74
	return fontloader.metrics(f).descent() + 1;
}


int font_metrics::ascent(char_type c, LyXFont const & f)
{
	if (!lyx_gui::use_gui)
		return 1;

	QRect const & r = fontloader.metrics(f).boundingRect(ucs4_to_ucs2(c));
	// Qt/Win 3.2.1nc (at least) corrects the GetGlyphOutlineA|W y
	// value by the height: (x, -y-height, width, height).
	// Other versions return: (x, -y, width, height)
#if defined(Q_WS_WIN) && (QT_VERSION == 0x030201)
	return -r.top() - r.height();
#else
	return -r.top();
#endif
}


int font_metrics::descent(char_type c, LyXFont const & f)
{
	if (!lyx_gui::use_gui)
		return 1;

	QRect const & r = fontloader.metrics(f).boundingRect(ucs4_to_ucs2(c));
	// Qt/Win 3.2.1nc (at least) corrects the GetGlyphOutlineA|W y
	// value by the height: (x, -y-height, width, height).
	// Other versions return: (x, -y, width, height)
#if defined(Q_WS_WIN) && (QT_VERSION == 0x030201)
	return r.bottom() + r.height() + 1;
#else
	return r.bottom() + 1;
#endif
}


int font_metrics::lbearing(char_type c, LyXFont const & f)
{
	if (!lyx_gui::use_gui)
		return 1;

	return fontloader.metrics(f).leftBearing(ucs4_to_ucs2(c));
}


int font_metrics::rbearing(char_type c, LyXFont const & f)
{
	if (!lyx_gui::use_gui)
		return 1;

	QFontMetrics const & m = fontloader.metrics(f);

	// Qt rbearing is from the right edge of the char's width().
	unsigned short sc = ucs4_to_ucs2(c);
	return m.width(sc) - m.rightBearing(sc);
}


int font_metrics::width(lyx::char_type const * str, size_t const ls, LyXFont const & f)
{
	if (!lyx_gui::use_gui)
		return ls;

	std::vector<unsigned short> ucs2 = ucs4_to_ucs2(str, ls);
	ucs2.push_back(0);

	if (f.realShape() == LyXFont::SMALLCAPS_SHAPE)
		return smallcapswidth(&ucs2[0], ls, f);

	QLFontInfo & fi = fontloader.fontinfo(f);

	if (ls == 1)
		return fi.width(ucs2[0]);

	int w = 0;
	for (size_t i = 0; i < ls; ++i)
		w += fi.width(ucs2[i]);

	return w;
}


void font_metrics::rectText(docstring const & str, LyXFont const & f,
	int & w, int & ascent, int & descent)
{
	QFontMetrics const & m = fontloader.metrics(f);
	static int const d = 2;
	w = font_metrics::width(str, f) + d * 2 + 2;
	ascent = m.ascent() + d;
	descent = m.descent() + d;
}


void font_metrics::buttonText(docstring const & str, LyXFont const & f,
	int & w, int & ascent, int & descent)
{
	QFontMetrics const & m = fontloader.metrics(f);
	static int const d = 3;
	w = font_metrics::width(str, f) + d * 2 + 2;
	ascent = m.ascent() + d;
	descent = m.descent() + d;
}


int font_metrics::signedWidth(docstring const & s, LyXFont const & f)
{
	if (s[0] == '-')
		return -font_metrics::width(s.substr(1, s.length() - 1), f);
	else
		return font_metrics::width(s, f);
}
