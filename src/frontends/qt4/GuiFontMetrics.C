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
	// Caution: The following ucs4_to_something conversions work for
	// symbol fonts only because they are no real conversions but simple
	// casts in reality.

	if (ls == 1 && !smallcaps_shape_) {
		return width(s[0]);
	}

	if (smallcaps_shape_) {
		QString ucs2;
		ucs4_to_qstring(s, ls, ucs2);
		return smallcapsWidth(ucs2);
	}

	int w = 0;
	for (unsigned int i = 0; i < ls; ++i)
		w += width(s[i]);

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
	if (s.empty())
		return 0;

	if (s[0] == '-')
		return -width(&(s[1]), s.length() - 1);
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

#ifndef USE_LYX_FONTCACHE

int GuiFontMetrics::ascent(char_type c) const
{
	QRect const & r = metrics_.boundingRect(ucs4_to_qchar(c));
	return -r.top();
}


int GuiFontMetrics::descent(char_type c) const
{
	QRect const & r = metrics_.boundingRect(ucs4_to_qchar(c));
	return r.bottom() + 1;
}

#else

void GuiFontMetrics::fillMetricsCache(char_type c) const
{
	QRect const & r = metrics_.boundingRect(ucs4_to_qchar(c));
	AscendDescend ad = { -r.top(), r.bottom() + 1};
	// We could as well compute the width but this is not really
	// needed for now as it is done directly in width() below.
	metrics_cache_.insert(c, ad);
}


int GuiFontMetrics::width(char_type c) const
{
	if (!width_cache_.contains(c)) {
		width_cache_.insert(c, metrics_.width(ucs4_to_qchar(c)));
	}

	return width_cache_.value(c);
}


int GuiFontMetrics::ascent(char_type c) const
{
	if (!metrics_cache_.contains(c))
		fillMetricsCache(c);

	return metrics_cache_.value(c).ascent;
}


int GuiFontMetrics::descent(char_type c) const
{
	if (!metrics_cache_.contains(c))
		fillMetricsCache(c);

	return metrics_cache_.value(c).descent;
}

#endif

} // frontend
} // lyx
