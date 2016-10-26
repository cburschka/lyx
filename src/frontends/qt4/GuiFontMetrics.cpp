/**
 * \file GuiFontMetrics.cpp
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

#include "Dimension.h"
#include "Language.h"
#include "LyXRC.h"

#include "insets/Inset.h"

#include "support/lassert.h"

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

namespace {
/**
 * Convert a UCS4 character into a QChar.
 * This is a hack (it does only make sense for the common part of the UCS4
 * and UTF16 encodings) and should not be used.
 * This does only exist because of performance reasons (a real conversion
 * using iconv is too slow on windows).
 *
 * This is no real conversion but a simple cast in reality. This is the reason
 * why this works well for symbol fonts used in mathed too, even though
 * these are not real ucs4 characters. These are codepoints in the
 * computer modern fonts used, nothing unicode related.
 * See comment in GuiPainter::text() for more explanation.
 **/
inline QChar const ucs4_to_qchar(char_type const ucs4)
{
	LATTEST(is_utf16(ucs4));
	return QChar(static_cast<unsigned short>(ucs4));
}
} // anon namespace


// Limit strwidth_cache_ size to 512kB of string data
GuiFontMetrics::GuiFontMetrics(QFont const & font)
	: font_(font), metrics_(font, 0),
	  strwidth_cache_(1 << 19),
	  tl_cache_rtl_(false), tl_cache_wordspacing_(-1.0)
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


int GuiFontMetrics::em() const
{
	return QFontInfo(font_).pixelSize();
}


int GuiFontMetrics::lineWidth() const
{
	return metrics_.lineWidth();
}


int GuiFontMetrics::underlinePos() const
{
	return metrics_.underlinePos();
}


int GuiFontMetrics::strikeoutPos() const
{
	return metrics_.strikeOutPos();
}


int GuiFontMetrics::lbearing(char_type c) const
{
	if (!is_utf16(c))
		// FIXME: QFontMetrics::leftBearing does not support the
		//        full unicode range. Once it does, we could use:
		//return metrics_.leftBearing(toqstr(docstring(1, c)));
		return 0;

	return metrics_.leftBearing(ucs4_to_qchar(c));
}


namespace {
int const outOfLimitMetric = -10000;
}


int GuiFontMetrics::rbearing(char_type c) const
{
	int value = rbearing_cache_.value(c, outOfLimitMetric);
	if (value != outOfLimitMetric)
		return value;

	// Qt rbearing is from the right edge of the char's width().
	if (is_utf16(c)) {
		QChar sc = ucs4_to_qchar(c);
		value = width(c) - metrics_.rightBearing(sc);
	} else {
		// FIXME: QFontMetrics::leftBearing does not support the
		//        full unicode range. Once it does, we could use:
		// metrics_.rightBearing(toqstr(docstring(1, c)));
		value = width(c);
	}

	rbearing_cache_.insert(c, value);

	return value;
}


int GuiFontMetrics::width(docstring const & s) const
{
	QByteArray qba =
		QByteArray(reinterpret_cast<char const *>(s.data()),
		           s.size() * sizeof(docstring::value_type));
	int * pw = strwidth_cache_[qba];
	if (pw)
		return *pw;
	// For some reason QMetrics::width returns a wrong value with Qt5
	// int w = metrics_.width(toqstr(s));
	QTextLayout tl;
	tl.setText(toqstr(s));
	tl.setFont(font_);
	tl.beginLayout();
	QTextLine line = tl.createLine();
	tl.endLayout();
	int w = int(line.naturalTextWidth());

	strwidth_cache_.insert(qba, new int(w), qba.size());
	return w;
}


int GuiFontMetrics::width(QString const & ucs2) const
{
	return width(qstring_to_ucs4(ucs2));
}


int GuiFontMetrics::signedWidth(docstring const & s) const
{
	if (s.empty())
		return 0;

	if (s[0] == '-')
		return -width(s.substr(1, s.size() - 1));
	else
		return width(s);
}


QTextLayout const &
GuiFontMetrics::getTextLayout(docstring const & s, QFont font,
                              bool const rtl, double const wordspacing) const
{
	if (s != tl_cache_s_ || font != tl_cache_font_ || rtl != tl_cache_rtl_
	    || wordspacing != tl_cache_wordspacing_) {
		tl_cache_.setText(toqstr(s));
		font.setWordSpacing(wordspacing);
		tl_cache_.setFont(font);
		// Note that both setFlags and the enums are undocumented
		tl_cache_.setFlags(rtl ? Qt::TextForceRightToLeft : Qt::TextForceLeftToRight);
		tl_cache_.beginLayout();
		tl_cache_.createLine();
		tl_cache_.endLayout();
		tl_cache_s_ = s;
		tl_cache_font_ = font;
		tl_cache_rtl_ = rtl;
		tl_cache_wordspacing_ = wordspacing;
	}
	return tl_cache_;
}


int GuiFontMetrics::pos2x(docstring const & s, int const pos, bool const rtl,
                          double const wordspacing) const
{
	QTextLayout const & tl = getTextLayout(s, font_, rtl, wordspacing);
	return static_cast<int>(tl.lineForTextPosition(pos).cursorToX(pos));
}


int GuiFontMetrics::x2pos(docstring const & s, int & x, bool const rtl,
                          double const wordspacing) const
{
	QTextLayout const & tl = getTextLayout(s, font_, rtl, wordspacing);
	int pos = tl.lineForTextPosition(0).xToCursor(x);
	// correct x value to the actual cursor position.
	x = static_cast<int>(tl.lineForTextPosition(0).cursorToX(pos));
	return pos;
}


bool GuiFontMetrics::breakAt(docstring & s, int & x, bool const rtl, bool const force) const
{
	if (s.empty())
		return false;
	QTextLayout tl;
	/* Qt will not break at a leading or trailing space, and we need
	 * that sometimes, see http://www.lyx.org/trac/ticket/9921.
	 *
	 * To work around the problem, we enclose the string between
	 * zero-width characters so that the QTextLayout algorithm will
	 * agree to break the text at these extremal spaces.
	*/
	// Unicode character ZERO WIDTH NO-BREAK SPACE
	QChar const zerow_nbsp(0xfeff);
	QString str = zerow_nbsp + toqstr(s) + zerow_nbsp;
#if 1
	/* Use unicode override characters to enforce drawing direction
	 * Source: http://www.iamcal.com/understanding-bidirectional-text/
	 */
	if (rtl)
		// Right-to-left override: forces to draw text right-to-left
		str = QChar(0x202E) + str;
	else
		// Left-to-right override: forces to draw text left-to-right
		str =  QChar(0x202D) + str;
	int const offset = 2;
#else
	// Alternative version that breaks with Qt5 and arabic text (#10436)
	// Note that both setFlags and the enums are undocumented
	tl.setFlags(rtl ? Qt::TextForceRightToLeft : Qt::TextForceLeftToRight);
	int const offset = 1;
#endif

	tl.setText(str);
	tl.setFont(font_);
	QTextOption to;
	to.setWrapMode(force ? QTextOption::WrapAnywhere : QTextOption::WordWrap);
	tl.setTextOption(to);
	tl.beginLayout();
	QTextLine line = tl.createLine();
	line.setLineWidth(x);
	tl.createLine();
	tl.endLayout();
	if ((force && line.textLength() == offset) || int(line.naturalTextWidth()) > x)
		return false;
	x = int(line.naturalTextWidth());
	// The offset is here to account for the extra leading characters.
	s = s.substr(0, line.textLength() - offset);
	return true;
}


void GuiFontMetrics::rectText(docstring const & str,
	int & w, int & ascent, int & descent) const
{
	static int const d = Inset::TEXT_TO_INSET_OFFSET / 2;

	w = width(str) + Inset::TEXT_TO_INSET_OFFSET;
	ascent = metrics_.ascent() + d;
	descent = metrics_.descent() + d;
}



void GuiFontMetrics::buttonText(docstring const & str,
	int & w, int & ascent, int & descent) const
{
	rectText(str, w, ascent, descent);
	w += Inset::TEXT_TO_INSET_OFFSET;
}


Dimension const GuiFontMetrics::defaultDimension() const
{
	return Dimension(0, maxAscent(), maxDescent());
}


Dimension const GuiFontMetrics::dimension(char_type c) const
{
	return Dimension(width(c), ascent(c), descent(c));
}


GuiFontMetrics::AscendDescend const GuiFontMetrics::fillMetricsCache(
		char_type c) const
{
	QRect r;
	if (is_utf16(c))
		r = metrics_.boundingRect(ucs4_to_qchar(c));
	else
		r = metrics_.boundingRect(toqstr(docstring(1, c)));

	AscendDescend ad = { -r.top(), r.bottom() + 1};
	// We could as well compute the width but this is not really
	// needed for now as it is done directly in width() below.
	metrics_cache_.insert(c, ad);

	return ad;
}


int GuiFontMetrics::width(char_type c) const
{
	int value = width_cache_.value(c, outOfLimitMetric);
	if (value != outOfLimitMetric)
		return value;

	if (is_utf16(c))
		value = metrics_.width(ucs4_to_qchar(c));
	else
		value = metrics_.width(toqstr(docstring(1, c)));

	width_cache_.insert(c, value);

	return value;
}


int GuiFontMetrics::ascent(char_type c) const
{
	static AscendDescend const outOfLimitAD =
		{outOfLimitMetric, outOfLimitMetric};
	AscendDescend value = metrics_cache_.value(c, outOfLimitAD);
	if (value.ascent != outOfLimitMetric)
		return value.ascent;

	value = fillMetricsCache(c);
	return value.ascent;
}


int GuiFontMetrics::descent(char_type c) const
{
	static AscendDescend const outOfLimitAD =
		{outOfLimitMetric, outOfLimitMetric};
	AscendDescend value = metrics_cache_.value(c, outOfLimitAD);
	if (value.descent != outOfLimitMetric)
		return value.descent;

	value = fillMetricsCache(c);
	return value.descent;
}

} // namespace frontend
} // namespace lyx
