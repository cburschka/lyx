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
#include "support/debug.h"
#include "support/lassert.h"
#include "support/lyxlib.h"
#include "support/textutils.h"

#define DISABLE_PMPROF
#include "support/pmprof.h"

#include <QByteArray>
#include <QRawFont>
#include <QtEndian>

#if QT_VERSION >= 0x050100
#include <QtMath>
#else
#define qDegreesToRadians(degree) (degree * (M_PI / 180))
#endif

using namespace std;
using namespace lyx::support;

/* Define what mechanisms are used to enforce text direction. There
 * are two methods that work with different Qt versions. Here we try
 * to use both methods together.
 */
// Define to use unicode override characters to force direction
#define BIDI_USE_OVERRIDE
// Define to use QTextLayout flag to force direction
#define BIDI_USE_FLAG

#if !defined(BIDI_USE_OVERRIDE) && !defined(BIDI_USE_FLAG)
#  error "Define at least one of BIDI_USE_OVERRIDE or BIDI_USE_FLAG"
#endif


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


namespace {
// Maximal size/cost for various caches. See QCache documentation to
// see what cost means.

// Limit strwidth_cache_ total cost to 1MB of string data.
int const strwidth_cache_max_cost = 1024 * 1024;
// Limit breakstr_cache_ total cost to 10MB of string data.
// This is useful for documents with very large insets.
int const breakstr_cache_max_cost = 10 * 1024 * 1024;
// Qt 5.x already has its own caching of QTextLayout objects
// but it does not seem to work well on MacOS X.
#if defined(Q_OS_MAC)
// For some reason, the built-in cache of QTextLayout does not work or
// exist on macOS.
// Limit qtextlayout_cache_ size to 500 elements (we do not know the
// size of the QTextLayout objects anyway).
int const qtextlayout_cache_max_size = 500;
#else
// Disable the cache
int const qtextlayout_cache_max_size = 0;
#endif


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
	  strwidth_cache_(strwidth_cache_max_cost),
	  breakstr_cache_(breakstr_cache_max_cost),
	  qtextlayout_cache_(qtextlayout_cache_max_size)
{
	// Determine italic slope
	double const defaultSlope = tan(qDegreesToRadians(19.0));
	QRawFont raw = QRawFont::fromFont(font);
	QByteArray post(raw.fontTable("post"));
	if (post.length() == 0) {
		slope_ = defaultSlope;
		LYXERR(Debug::FONT, "Screen font doesn't have 'post' table.");
	} else {
		// post table description:
		// https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6post.html
		int32_t italicAngle = qFromBigEndian(*reinterpret_cast<int32_t *>(post.data() + 4));
		double angle = italicAngle / 65536.0; // Fixed-point 16.16 to floating-point
		slope_ = -tan(qDegreesToRadians(angle));
		// Correct italic fonts with zero slope
		if (slope_ == 0.0 && font.italic())
			slope_ = defaultSlope;
		LYXERR(Debug::FONT, "Italic slope: " << slope_);
	}
}


int GuiFontMetrics::maxAscent() const
{
	return metrics_.ascent();
}


int GuiFontMetrics::maxDescent() const
{
	// We add 1 as the value returned by QT is different than X
	// See http://doc.trolltech.com/2.3/qfontmetrics.html#200b74
	// FIXME: check this
	return metrics_.descent() + 1;
}


int GuiFontMetrics::em() const
{
	return QFontInfo(font_).pixelSize();
}


int GuiFontMetrics::xHeight() const
{
//	LATTEST(metrics_.xHeight() == ascent('x'));
	return metrics_.xHeight();
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


bool GuiFontMetrics::italic() const
{
	return font_.italic();
}


double GuiFontMetrics::italicSlope() const
{
	return slope_;
}


namespace {
int const outOfLimitMetric = -10000;
}


int GuiFontMetrics::lbearing(char_type c) const
{
	int value = lbearing_cache_.value(c, outOfLimitMetric);
	if (value != outOfLimitMetric)
		return value;

	if (is_utf16(c))
		value = metrics_.leftBearing(ucs4_to_qchar(c));
	else {
		// FIXME: QFontMetrics::leftBearing does not support the
		//        full unicode range. Once it does, we could use:
		// metrics_.leftBearing(toqstr(docstring(1, c)));
		value = 0;
	}

	lbearing_cache_.insert(c, value);

	return value;
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
	if (int * wid_p = strwidth_cache_.object_ptr(s))
		return *wid_p;
	PROFILE_CACHE_MISS(width);
	/* Several problems have to be taken into account:
	 * * QFontMetrics::width does not returns a wrong value with Qt5 with
	 *   some arabic text, since the glyph-shaping operations are not
	 *   done (documented in Qt5).
	 * * QTextLayout is broken for single characters with null width
	 *   (like \not in mathed).
	 * * While QTextLine::horizontalAdvance is the right thing to use
     *   for text strings, it does not give a good result with some
     *   characters like the \int (gyph 4) of esint.

	 * The metrics of some of our math fonts (eg. esint) are such that
	 * QTextLine::horizontalAdvance leads, more or less, in the middle
	 * of a symbol. This is the horizontal position where a subscript
	 * should be drawn, so that the superscript has to be moved rightward.
	 * This is done when the kerning() method of the math insets returns
	 * a positive value. The problem with this choice is that navigating
	 * a formula becomes weird. For example, a selection extends only over
	 * about half of the symbol. In order to avoid this, with our math
	 * fonts we use QTextLine::naturalTextWidth, so that a superscript can
	 * be drawn right after the symbol, and move the subscript leftward by
	 * recording a negative value for the kerning.
	*/
	int w = 0;
	// is the string a single character from a math font ?
	bool const math_char = s.length() == 1 && font_.styleName() == "LyX";
	if (math_char) {
		QString const qs = toqstr(s);
		int br_width = metrics_.boundingRect(qs).width();
#if QT_VERSION >= 0x050b00
		int s_width = metrics_.horizontalAdvance(qs);
#else
		int s_width = metrics_.width(qs);
#endif
		// keep value 0 for math chars with width 0
		if (s_width != 0)
			w = max(br_width, s_width);
	} else {
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


uint qHash(TextLayoutKey const & key)
{
	double params = (2 * key.rtl - 1) * key.ws;
	return std::qHash(key.s) ^ ::qHash(params);
}


// This holds a translation table between the original string and the
// QString that we can use with QTextLayout.
struct TextLayoutHelper
{
	/// Create the helper
	/// \c s is the original string
	/// \c isrtl is true if the string is right-to-left
	TextLayoutHelper(docstring const & s, bool isrtl);

	/// translate QString index to docstring index
	docstring::size_type qpos2pos(int qpos) const
	{
		return lower_bound(pos2qpos_.begin(), pos2qpos_.end(), qpos) - pos2qpos_.begin();
	}

	/// Translate docstring index to QString index
	int pos2qpos(docstring::size_type pos) const { return pos2qpos_[pos]; }

	// The original string
	docstring docstr;
	// The mirror string
	QString qstr;
	// is string right-to-left?
	bool rtl;

private:
	// This vector contains the QString pos for each string position
	vector<int> pos2qpos_;
};


TextLayoutHelper::TextLayoutHelper(docstring const & s, bool isrtl)
	: docstr(s), rtl(isrtl)
{
	// Reserve memory for performance purpose
	pos2qpos_.reserve(s.size());
	qstr.reserve(2 * s.size());

	/* Qt will not break at a leading or trailing space, and we need
	 * that sometimes, see http://www.lyx.org/trac/ticket/9921.
	 *
	 * To work around the problem, we enclose the string between
	 * word joiner characters so that the QTextLayout algorithm will
	 * agree to break the text at these extremal spaces.
	 */
	// Unicode character WORD JOINER
	QChar const word_joiner(0x2060);
	qstr += word_joiner;

#ifdef BIDI_USE_OVERRIDE
	/* Unicode override characters enforce drawing direction
	 * Source: http://www.iamcal.com/understanding-bidirectional-text/
	 * Left-to-right override is 0x202d and right-to-left override is 0x202e.
	 */
	qstr += QChar(rtl ? 0x202e : 0x202d);
#endif

	// Now translate the string character-by-character.
	bool was_space = false;
	for (char_type const c : s) {
		// insert a word joiner character between consecutive spaces
		bool const is_space = isSpace(c);
		if (is_space && was_space)
			qstr += word_joiner;
		was_space = is_space;
		// Remember the QString index at this point
		pos2qpos_.push_back(qstr.size());
		// Performance: UTF-16 characters are easier
		if (is_utf16(c))
			qstr += ucs4_to_qchar(c);
		else
			qstr += toqstr(c);
	}

	// Final word joiner (see above)
	qstr += word_joiner;

	// Add virtual position at the end of the string
	pos2qpos_.push_back(qstr.size());

	//QString dump = qstr;
	//LYXERR0("TLH: " << dump.replace(word_joiner, "|").toStdString());
}


namespace {

shared_ptr<QTextLayout>
getTextLayout_helper(TextLayoutHelper const & tlh, double const wordspacing,
                     QFont font)
{
	auto const ptl = make_shared<QTextLayout>();
	ptl->setCacheEnabled(true);
	font.setWordSpacing(wordspacing);
	ptl->setFont(font);
#ifdef BIDI_USE_FLAG
	/* Use undocumented flag to enforce drawing direction
	 * FIXME: This does not work with Qt 5.11 (ticket #11284).
	 */
	ptl->setFlags(tlh.rtl ? Qt::TextForceRightToLeft : Qt::TextForceLeftToRight);
#endif
	ptl->setText(tlh.qstr);

	ptl->beginLayout();
	ptl->createLine();
	ptl->endLayout();

	return ptl;
}

}

shared_ptr<QTextLayout const>
GuiFontMetrics::getTextLayout(TextLayoutHelper const & tlh,
                              double const wordspacing) const
{
	PROFILE_THIS_BLOCK(getTextLayout_TLH);
	TextLayoutKey key{tlh.docstr, tlh.rtl, wordspacing};
	if (auto ptl = qtextlayout_cache_[key])
		return ptl;
	PROFILE_CACHE_MISS(getTextLayout_TLH);
	auto const ptl = getTextLayout_helper(tlh, wordspacing, font_);
	qtextlayout_cache_.insert(key, ptl);
	return ptl;
}


shared_ptr<QTextLayout const>
GuiFontMetrics::getTextLayout(docstring const & s, bool const rtl,
                              double const wordspacing) const
{
	PROFILE_THIS_BLOCK(getTextLayout);
	TextLayoutKey key{s, rtl, wordspacing};
	if (auto ptl = qtextlayout_cache_[key])
		return ptl;
	PROFILE_CACHE_MISS(getTextLayout);
	TextLayoutHelper tlh(s, rtl);
	auto const ptl = getTextLayout_helper(tlh, wordspacing, font_);
	qtextlayout_cache_.insert(key, ptl);
	return ptl;
}


int GuiFontMetrics::pos2x(docstring const & s, int pos, bool const rtl,
                          double const wordspacing) const
{
	TextLayoutHelper tlh(s, rtl);
	auto ptl = getTextLayout(tlh, wordspacing);
	// pos can be negative, see #10506.
	int const qpos = tlh.pos2qpos(max(pos, 0));
	return static_cast<int>(ptl->lineForTextPosition(qpos).cursorToX(qpos));
}


int GuiFontMetrics::x2pos(docstring const & s, int & x, bool const rtl,
                          double const wordspacing) const
{
	TextLayoutHelper tlh(s, rtl);
	auto ptl = getTextLayout(tlh, wordspacing);
	QTextLine const & tline = ptl->lineForTextPosition(0);
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

	return tlh.qpos2pos(qpos);
}


FontMetrics::Breaks
GuiFontMetrics::breakString_helper(docstring const & s, int first_wid, int wid,
                                   bool rtl, bool force) const
{
	TextLayoutHelper const tlh(s, rtl);

	QTextLayout tl;
#ifdef BIDI_USE_FLAG
	/* Use undocumented flag to enforce drawing direction
	 * FIXME: This does not work with Qt 5.11 (ticket #11284).
	 */
	tl.setFlags(rtl ? Qt::TextForceRightToLeft : Qt::TextForceLeftToRight);
#endif
	tl.setText(tlh.qstr);
	tl.setFont(font_);
	QTextOption to;
	/*
	 * Some Asian languages split lines anywhere (no notion of
	 * word). It seems that QTextLayout is not aware of this fact.
	 * See for reference:
	 *    https://en.wikipedia.org/wiki/Line_breaking_rules_in_East_Asian_languages
	 *
	 * FIXME: Something shall be done about characters which are
	 * not allowed at the beginning or end of line.
	 */
	to.setWrapMode(force ? QTextOption::WrapAtWordBoundaryOrAnywhere
	                     : QTextOption::WordWrap);
	tl.setTextOption(to);

	bool first = true;
	tl.beginLayout();
	while(true) {
		QTextLine line = tl.createLine();
		if (!line.isValid())
			break;
		line.setLineWidth(first ? first_wid : wid);
		first = false;
	}
	tl.endLayout();

	Breaks breaks;
	int pos = 0;
	for (int i = 0 ; i < tl.lineCount() ; ++i) {
		QTextLine const & line = tl.lineAt(i);
		int const line_epos = line.textStart() + line.textLength();
		int const epos = tlh.qpos2pos(line_epos);
		// This does not take trailing spaces into account, except for the last line.
		int const wid = iround(line.naturalTextWidth());
		// If the line is not the last one, trailing space is always omitted.
		int nspc_wid = wid;
		// For the last line, compute the width without trailing space
		if (i + 1 == tl.lineCount() && !s.empty() && isSpace(s.back())
		    && line.textStart() <= tlh.pos2qpos(s.size() - 1))
			nspc_wid = iround(line.cursorToX(tlh.pos2qpos(s.size() - 1)));
		breaks.emplace_back(epos - pos, wid, nspc_wid);
		pos = epos;
	}

	return breaks;
}


uint qHash(BreakStringKey const & key)
{
	// assume widths are less than 10000. This fits in 32 bits.
	uint params = key.force + 2 * key.rtl + 4 * key.first_wid + 10000 * key.wid;
	return std::qHash(key.s) ^ ::qHash(params);
}


FontMetrics::Breaks GuiFontMetrics::breakString(docstring const & s, int first_wid, int wid,
                                                bool rtl, bool force) const
{
	PROFILE_THIS_BLOCK(breakString);
	if (s.empty())
		return Breaks();

	BreakStringKey key{s, first_wid, wid, rtl, force};
	Breaks brks;
	if (auto * brks_ptr = breakstr_cache_.object_ptr(key))
		brks = *brks_ptr;
	else {
		PROFILE_CACHE_MISS(breakString);
		brks = breakString_helper(s, first_wid, wid, rtl, force);
		breakstr_cache_.insert(key, brks, sizeof(key) + s.size() * sizeof(char_type));
	}
	return brks;
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

#if QT_VERSION >= 0x050b00
	if (is_utf16(c))
		value = metrics_.horizontalAdvance(ucs4_to_qchar(c));
	else
		value = metrics_.horizontalAdvance(toqstr(docstring(1, c)));
#else
	if (is_utf16(c))
		value = metrics_.width(ucs4_to_qchar(c));
	else
		value = metrics_.width(toqstr(docstring(1, c)));
#endif

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
