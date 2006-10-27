/**
 * \file GuiFontMetrics.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiFontMetrics.h"

#include "qt_helpers.h"

#include "language.h"

#include "support/unicode.h"

using lyx::char_type;
using lyx::docstring;

using std::string;

namespace lyx {
namespace frontend {


GuiFontMetrics::GuiFontMetrics(QFont const & font)
: metrics_(font), smallcaps_metrics_(font), smallcaps_shape_(false)
{
}


GuiFontMetrics::GuiFontMetrics(QFont const & font, QFont const & smallcaps_font)
: metrics_(font), smallcaps_metrics_(smallcaps_font), smallcaps_shape_(true)
{
}


int GuiFontMetrics::maxAscent() const
{
	return metrics_.ascent();
}


int GuiFontMetrics::maxDescent() const
{
	// We add 1 as the value returned by QT is different than X
	// See http://doc.trolltech.com/2.3/qfontmetrics.html#200b74
	return metrics_.descent() + 1;
}


int GuiFontMetrics::ascent(char_type c) const
{
	QRect const & r = metrics_.boundingRect(ucs4_to_qchar(c));
	// Qt/Win 3.2.1nc (at least) corrects the GetGlyphOutlineA|W y
	// value by the height: (x, -y-height, width, height).
	// Other versions return: (x, -y, width, height)
#if defined(Q_WS_WIN) && (QT_VERSION == 0x030201)
	return -r.top() - r.height();
#else
	return -r.top();
#endif
}


int GuiFontMetrics::descent(char_type c) const
{
	QRect const & r = metrics_.boundingRect(ucs4_to_qchar(c));
	// Qt/Win 3.2.1nc (at least) corrects the GetGlyphOutlineA|W y
	// value by the height: (x, -y-height, width, height).
	// Other versions return: (x, -y, width, height)
#if defined(Q_WS_WIN) && (QT_VERSION == 0x030201)
	return r.bottom() + r.height() + 1;
#else
	return r.bottom() + 1;
#endif
}


int GuiFontMetrics::lbearing(char_type c) const
{
	return metrics_.leftBearing(ucs4_to_qchar(c));
}


int GuiFontMetrics::rbearing(char_type c) const
{
	// Qt rbearing is from the right edge of the char's width().
	QChar sc = ucs4_to_qchar(c);
	return metrics_.width(sc) - metrics_.rightBearing(sc);
}


int GuiFontMetrics::smallcapsWidth(QString const & s) const
{
	int w = 0;
	int const ls = s.size();

	for (int i = 0; i < ls; ++i) {
		QChar const & c = s[i];
		QChar const uc = c.toUpper();
		if (c != uc)
			w += smallcaps_metrics_.width(uc);
		else
			w += metrics_.width(c);
	}
	return w;
}


int GuiFontMetrics::width(char_type const * s, size_t ls) const
{
	if (ls == 1 && !smallcaps_shape_) {
		QChar const c = ucs4_to_qchar(s[0]);
		return width(c.unicode());
	}

	QString ucs2;
	ucs4_to_qstring(s, ls, ucs2);

	if (smallcaps_shape_)
		return smallcapsWidth(ucs2);

	int w = 0;
	for (unsigned int i = 0; i < ls; ++i)
		w += width(ucs2[i].unicode());

	return w;
}


int GuiFontMetrics::width(QString const & ucs2) const
{
	int const ls = ucs2.size();
	if (ls == 1 && !smallcaps_shape_) {
		return width(ucs2[0].unicode());
	}

	if (smallcaps_shape_)
		return smallcapsWidth(ucs2);

	int w = 0;
	for (int i = 0; i < ls; ++i)
		w += width(ucs2[i].unicode());

	return w;
}


int GuiFontMetrics::signedWidth(docstring const & s) const
{
	if (s[0] == '-')
		return -FontMetrics::width(s.substr(1, s.length() - 1));
	else
		return FontMetrics::width(s);
}


void GuiFontMetrics::rectText(docstring const & str,
	int & w, int & ascent, int & descent) const
{
	static int const d = 2;
	w = FontMetrics::width(str) + d * 2 + 2;
	ascent = metrics_.ascent() + d;
	descent = metrics_.descent() + d;
}



void GuiFontMetrics::buttonText(docstring const & str,
	int & w, int & ascent, int & descent) const
{
	static int const d = 3;
	w = FontMetrics::width(str) + d * 2 + 2;
	ascent = metrics_.ascent() + d;
	descent = metrics_.descent() + d;
}

#ifdef USE_LYX_FONTCACHE
int GuiFontMetrics::width(unsigned short val) const
{
	GuiFontMetrics::WidthCache::const_iterator cit = widthcache.find(val);
	if (cit != widthcache.end())
		return cit->second;

	int const w = metrics_.width(QChar(val));
	widthcache[val] = w;
	return w;
}
#endif

}
}
