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

#include "support/convert.h"
#include "support/lassert.h"
#include "support/lyxlib.h"

#define DISABLE_PMPROF
#include "support/pmprof.h"

#include <QByteArray>

using namespace std;
using namespace lyx::support;

namespace std {

/*
 * Argument-dependent lookup implies that this function shall be
 * declared in the namespace of its argument. But this is std
 * namespace, since lyx::docstring is just std::basic_string<wchar_t>.
 */
uint qHash(lyx::docstring const & s)
{
	return qHash(QByteArray(reinterpret_cast<char const *>(s.data()),
	                        s.size() * sizeof(lyx::docstring::value_type)));
}

} // namespace std

namespace lyx {
namespace frontend {


/*
 * Limit (strwidth|breakat)_cache_ size to 512kB of string data.
 * Limit qtextlayout_cache_ size to 500 elements (we do not know the
 * size of the QTextLayout objects anyway).
 * Note that all these numbers are arbitrary.
 * Also, setting size to 0 is tantamount to disabling the cache.
 */
int cache_metrics_width_size = 1 << 19;
int cache_metrics_breakat_size = 1 << 19;
// Qt 5.x already has its own caching of QTextLayout objects
// but it does not seem to work well on MacOS X.
#if (QT_VERSION < 0x050000) || defined(Q_OS_MAC)
int cache_metrics_qtextlayout_size = 500;
#else
int cache_metrics_qtextlayout_size = 0;
#endif


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
} // namespace


GuiFontMetrics::GuiFontMetrics(QFont const & font)
	: font_(font), metrics_(font, 0),
	  strwidth_cache_(cache_metrics_width_size),
	  breakat_cache_(cache_metrics_breakat_size),
	  qtextlayout_cache_(cache_metrics_qtextlayout_size)
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
	PROFILE_THIS_BLOCK(width);
	if (strwidth_cache_.contains(s))
		return strwidth_cache_[s];
	PROFILE_CACHE_MISS(width);
	/* For some reason QMetrics::width returns a wrong value with Qt5
	 * with some arabic text. OTOH, QTextLayout is broken for single
	 * characters with null width (like \not in mathed). Also, as a
	 * safety measure, always use QMetrics::width with our math fonts.
	*/
	int w = 0;
	if (s.length() == 1
#if QT_VERSION >= 0x040800
	    || font_.styleName() == "LyX"
#endif
	    )
		w = metrics_.width(toqstr(s));
	else {
		QTextLayout tl;
		tl.setText(toqstr(s));
		tl.setFont(font_);
		tl.beginLayout();
		QTextLine line = tl.createLine();
		tl.endLayout();
		w = iround(line.horizontalAdvance());
	}
	strwidth_cache_.insert(s, w, s.size() * sizeof(char_type));
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


shared_ptr<QTextLayout const>
GuiFontMetrics::getTextLayout(docstring const & s, bool const rtl,
                              double const wordspacing) const
{
	PROFILE_THIS_BLOCK(getTextLayout);
	docstring const s_cache =
		s + (rtl ? "r" : "l") + convert<docstring>(wordspacing);
	if (auto ptl = qtextlayout_cache_[s_cache])
		return ptl;
	PROFILE_CACHE_MISS(getTextLayout);
	auto const ptl = make_shared<QTextLayout>();
	ptl->setCacheEnabled(true);
	ptl->setText(toqstr(s));
	QFont copy = font_;
	copy.setWordSpacing(wordspacing);
	ptl->setFont(copy);
	// Note that both setFlags and the enums are undocumented
	ptl->setFlags(rtl ? Qt::TextForceRightToLeft : Qt::TextForceLeftToRight);
	ptl->beginLayout();
	ptl->createLine();
	ptl->endLayout();
	qtextlayout_cache_.insert(s_cache, ptl);
	return ptl;
}


int GuiFontMetrics::pos2x(docstring const & s, int pos, bool const rtl,
                          double const wordspacing) const
{
	if (pos <= 0)
		pos = 0;
	shared_ptr<QTextLayout const> tl = getTextLayout(s, rtl, wordspacing);
	/* Since QString is UTF-16 and docstring is UCS-4, the offsets may
	 * not be the same when there are high-plan unicode characters
	 * (bug #10443).
	 */
	int const qpos = toqstr(s.substr(0, pos)).length();
	return static_cast<int>(tl->lineForTextPosition(qpos).cursorToX(qpos));
}


int GuiFontMetrics::x2pos(docstring const & s, int & x, bool const rtl,
                          double const wordspacing) const
{
	shared_ptr<QTextLayout const> tl = getTextLayout(s, rtl, wordspacing);
	QTextLine const & tline = tl->lineForTextPosition(0);
	int qpos = tline.xToCursor(x);
	int newx = static_cast<int>(tline.cursorToX(qpos));
	// The value of qpos may be wrong in rtl text (see ticket #10569).
	// To work around this, let's have a look at adjacent positions to
	// see whether we find closer matches.
	if (rtl && newx < x) {
		while (qpos > 0) {
			int const xm = static_cast<int>(tline.cursorToX(qpos - 1));
			if (abs(xm - x) < abs(newx - x)) {
				--qpos;
				newx = xm;
			} else
				break;
		}
	} else if (rtl && newx > x) {
		while (qpos < tline.textLength()) {
			int const xp = static_cast<int>(tline.cursorToX(qpos + 1));
			if (abs(xp - x) < abs(newx - x)) {
				++qpos;
				newx = xp;
			} else
				break;
		}
	}
	// correct x value to the actual cursor position.
	x = newx;

	/* Since QString is UTF-16 and docstring is UCS-4, the offsets may
	 * not be the same when there are high-plan unicode characters
	 * (bug #10443).
	 */
#if QT_VERSION < 0x040801 || QT_VERSION >= 0x050100
	return qstring_to_ucs4(tl->text().left(qpos)).length();
#else
	/* Due to QTBUG-25536 in 4.8.1 <= Qt < 5.1.0, the string returned
	 * by QString::toUcs4 (used by qstring_to_ucs4) may have wrong
	 * length. We work around the problem by trying all docstring
	 * positions until the right one is found. This is slow only if
	 * there are many high-plane Unicode characters. It might be
	 * worthwhile to implement a dichotomy search if this shows up
	 * under a profiler.
	 */
	int pos = min(qpos, static_cast<int>(s.length()));
	while (pos >= 0 && toqstr(s.substr(0, pos)).length() != qpos)
		--pos;
	LASSERT(pos > 0 || qpos == 0, /**/);
	return pos;
#endif
}


int GuiFontMetrics::countExpanders(docstring const & str) const
{
	// Numbers of characters that are expanded by inter-word spacing.  These
	// characters are spaces, except for characters 09-0D which are treated
	// specially.  (From a combination of testing with the notepad found in qt's
	// examples, and reading the source code.)  In addition, consecutive spaces
	// only count as one expander.
	bool wasspace = false;
	int nexp = 0;
	for (char_type c : str)
		if (c > 0x0d && QChar(c).isSpace()) {
			if (!wasspace) {
				++nexp;
				wasspace = true;
			}
		} else
			wasspace = false;
	return nexp;
}


pair<int, int>
GuiFontMetrics::breakAt_helper(docstring const & s, int const x,
                               bool const rtl, bool const force) const
{
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
	QString qs = zerow_nbsp + toqstr(s) + zerow_nbsp;
#if 1
	/* Use unicode override characters to enforce drawing direction
	 * Source: http://www.iamcal.com/understanding-bidirectional-text/
	 */
	if (rtl)
		// Right-to-left override: forces to draw text right-to-left
		qs = QChar(0x202E) + qs;
	else
		// Left-to-right override: forces to draw text left-to-right
		qs =  QChar(0x202D) + qs;
	int const offset = 2;
#else
	// Alternative version that breaks with Qt5 and arabic text (#10436)
	// Note that both setFlags and the enums are undocumented
	tl.setFlags(rtl ? Qt::TextForceRightToLeft : Qt::TextForceLeftToRight);
	int const offset = 1;
#endif

	tl.setText(qs);
	tl.setFont(font_);
	QTextOption to;
	to.setWrapMode(force ? QTextOption::WrapAtWordBoundaryOrAnywhere
	                     : QTextOption::WordWrap);
	tl.setTextOption(to);
	tl.beginLayout();
	QTextLine line = tl.createLine();
	line.setLineWidth(x);
	tl.createLine();
	tl.endLayout();
	int const line_wid = iround(line.horizontalAdvance());
	if ((force && line.textLength() == offset) || line_wid > x)
		return {-1, -1};
	/* Since QString is UTF-16 and docstring is UCS-4, the offsets may
	 * not be the same when there are high-plan unicode characters
	 * (bug #10443).
	 */
	// The variable `offset' is here to account for the extra leading characters.
	// The ending character zerow_nbsp has to be ignored if the line is complete.
	int const qlen = line.textLength() - offset - (line.textLength() == qs.length());
#if QT_VERSION < 0x040801 || QT_VERSION >= 0x050100
	int len = qstring_to_ucs4(qs.mid(offset, qlen)).length();
#else
	/* Due to QTBUG-25536 in 4.8.1 <= Qt < 5.1.0, the string returned
	 * by QString::toUcs4 (used by qstring_to_ucs4) may have wrong
	 * length. We work around the problem by trying all docstring
	 * positions until the right one is found. This is slow only if
	 * there are many high-plane Unicode characters. It might be
	 * worthwhile to implement a dichotomy search if this shows up
	 * under a profiler.
	 */
	int len = min(qlen, static_cast<int>(s.length()));
	while (len >= 0 && toqstr(s.substr(0, len)).length() != qlen)
		--len;
	LASSERT(len > 0 || qlen == 0, /**/);
#endif
	return {len, line_wid};
}


bool GuiFontMetrics::breakAt(docstring & s, int & x, bool const rtl, bool const force) const
{
	PROFILE_THIS_BLOCK(breakAt);
	if (s.empty())
		return false;

	docstring const s_cache =
		s + convert<docstring>(x) + (rtl ? "r" : "l") + (force ? "f" : "w");
	pair<int, int> pp;

	if (breakat_cache_.contains(s_cache))
		pp = breakat_cache_[s_cache];
	else {
		PROFILE_CACHE_MISS(breakAt);
		pp = breakAt_helper(s, x, rtl, force);
		breakat_cache_.insert(s_cache, pp, s_cache.size() * sizeof(char_type));
	}
	if (pp.first == -1)
		return false;
	s = s.substr(0, pp.first);
	x = pp.second;
	return true;
}


void GuiFontMetrics::rectText(docstring const & str,
	int & w, int & ascent, int & descent) const
{
	// FIXME: let offset depend on font (this is Inset::TEXT_TO_OFFSET)
	int const offset = 4;

	w = width(str) + offset;
	ascent = metrics_.ascent() + offset / 2;
	descent = metrics_.descent() + offset / 2;
}


void GuiFontMetrics::buttonText(docstring const & str, const int offset,
	int & w, int & ascent, int & descent) const
{
	rectText(str, w, ascent, descent);
	w += offset;
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
