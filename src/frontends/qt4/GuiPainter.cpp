/**
 * \file GuiPainter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiPainter.h"

#include "ColorCache.h"
#include "GuiApplication.h"
#include "GuiFontLoader.h"
#include "GuiFontMetrics.h"
#include "GuiImage.h"
#include "qt_helpers.h"

#include "Font.h"
#include "Language.h"
#include "LyXRC.h"

#include "insets/Inset.h"

#include "support/lassert.h"
#include "support/debug.h"

#include <QPixmapCache>
#include <QTextLayout>

// Set USE_PIXMAP_CACHE to 1 for enabling the use of a Pixmap cache when
// drawing text. This is especially useful for older PPC/Mac systems.
#if defined(Q_WS_X11) || defined(QPA_XCB)
#define USE_PIXMAP_CACHE 0
#else
#define USE_PIXMAP_CACHE 1
#endif

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

const int Painter::thin_line = 0;

GuiPainter::GuiPainter(QPaintDevice * device, double pixel_ratio)
	: QPainter(device), Painter(pixel_ratio),
	  use_pixmap_cache_(lyxrc.use_pixmap_cache && USE_PIXMAP_CACHE)
{
	// new QPainter has default QPen:
	current_color_ = guiApp->colorCache().get(Color_black);
	current_ls_ = line_solid;
	current_lw_ = thin_line;
}


GuiPainter::~GuiPainter()
{
	QPainter::end();
	//lyxerr << "GuiPainter::end()" << endl;
}


void GuiPainter::setQPainterPen(QColor const & col,
	Painter::line_style ls, int lw)
{
	if (col == current_color_ && ls == current_ls_ && lw == current_lw_)
		return;

	current_color_ = col;
	current_ls_ = ls;
	current_lw_ = lw;

	QPen pen = QPainter::pen();
	pen.setColor(col);

	switch (ls) {
		case line_solid: pen.setStyle(Qt::SolidLine); break;
		case line_onoffdash: pen.setStyle(Qt::DotLine); break;
	}

	pen.setWidth(lw);

	setPen(pen);
}


QString GuiPainter::generateStringSignature(QString const & str, FontInfo const & f)
{
	QString sig = str;
	sig.append(QChar(static_cast<short>(f.family())));
	sig.append(QChar(static_cast<short>(f.series())));
	sig.append(QChar(static_cast<short>(f.realShape())));
	sig.append(QChar(static_cast<short>(f.size())));
	Color const & color = f.realColor();
	sig.append(QChar(static_cast<short>(color.baseColor)));
	sig.append(QChar(static_cast<short>(color.mergeColor)));
	if (!monochrome_min_.empty()) {
		QColor const & min = monochrome_min_.top();
		QColor const & max = monochrome_max_.top();
		sig.append(QChar(static_cast<short>(min.red())));
		sig.append(QChar(static_cast<short>(min.green())));
		sig.append(QChar(static_cast<short>(min.blue())));
		sig.append(QChar(static_cast<short>(max.red())));
		sig.append(QChar(static_cast<short>(max.green())));
		sig.append(QChar(static_cast<short>(max.blue())));
	}
	return sig;
}


QColor GuiPainter::computeColor(Color col)
{
	return filterColor(guiApp->colorCache().get(col));
}


QColor GuiPainter::filterColor(QColor const & col)
{
	if (monochrome_min_.empty())
		return col;

	// map into [min,max] interval
	QColor const & min = monochrome_min_.top();
	QColor const & max = monochrome_max_.top();
			
	qreal v = col.valueF();
	v *= v; // make it a bit steeper (i.e. darker)
		
	qreal minr, ming, minb;
	qreal maxr, maxg, maxb;
	min.getRgbF(&minr, &ming, &minb);
	max.getRgbF(&maxr, &maxg, &maxb);
			
	QColor c;
	c.setRgbF(
		v * (minr - maxr) + maxr,
		v * (ming - maxg) + maxg,
		v * (minb - maxb) + maxb);
	return c;
}


void GuiPainter::enterMonochromeMode(Color const & min, Color const & max)
{
	QColor qmin = filterColor(guiApp->colorCache().get(min));
	QColor qmax = filterColor(guiApp->colorCache().get(max));
	monochrome_min_.push(qmin);
	monochrome_max_.push(qmax);
}


void GuiPainter::leaveMonochromeMode()
{
	LASSERT(!monochrome_min_.empty(), return);
	monochrome_min_.pop();
	monochrome_max_.pop();
}


void GuiPainter::point(int x, int y, Color col)
{
	if (!isDrawingEnabled())
		return;

	setQPainterPen(computeColor(col));
	drawPoint(x, y);
}


void GuiPainter::line(int x1, int y1, int x2, int y2,
	Color col,
	line_style ls,
	int lw)
{
	if (!isDrawingEnabled())
		return;

	setQPainterPen(computeColor(col), ls, lw);
	bool const do_antialiasing = renderHints() & TextAntialiasing
		&& x1 != x2 && y1 != y2;
	setRenderHint(Antialiasing, do_antialiasing);
	drawLine(x1, y1, x2, y2);
	setRenderHint(Antialiasing, false);
}


void GuiPainter::lines(int const * xp, int const * yp, int np,
	Color col,
	fill_style fs,
	line_style ls,
	int lw)
{
	if (!isDrawingEnabled())
		return;

	// double the size if needed
	// FIXME THREAD
	static QVector<QPoint> points(32);
	if (np > points.size())
		points.resize(2 * np);

	bool antialias = false;
	for (int i = 0; i < np; ++i) {
		points[i].setX(xp[i]);
		points[i].setY(yp[i]);
		if (i != 0)
			antialias |= xp[i-1] != xp[i] && yp[i-1] != yp[i];
	}
	QColor const color = computeColor(col);
	setQPainterPen(color, ls, lw);
	bool const text_is_antialiased = renderHints() & TextAntialiasing;
	setRenderHint(Antialiasing, antialias && text_is_antialiased);
	if (fs == fill_none) {
		drawPolyline(points.data(), np);
	} else {
		QBrush const oldbrush = brush();
		setBrush(QBrush(color));
		drawPolygon(points.data(), np, fs == fill_oddeven ?
		            Qt::OddEvenFill : Qt::WindingFill);
		setBrush(oldbrush);
	}
	setRenderHint(Antialiasing, false);
}


void GuiPainter::path(int const * xp, int const * yp,
	int const * c1x, int const * c1y,
	int const * c2x, int const * c2y,
	int np,
	Color col,
	fill_style fs,
	line_style ls,
	int lw)
{
	if (!isDrawingEnabled())
		return;

	QPainterPath bpath;
	// This is the starting point, so its control points are meaningless
	bpath.moveTo(xp[0], yp[0]);

	for (int i = 1; i < np; ++i) {
		bool line = c1x[i] == xp[i - 1] && c1y[i] == yp[i - 1] &&
			    c2x[i] == xp[i] && c2y[i] == yp[i];
		if (line)
			bpath.lineTo(xp[i], yp[i]);
		else
			bpath.cubicTo(c1x[i], c1y[i],  c2x[i], c2y[i], xp[i], yp[i]);
	}
	QColor const color = computeColor(col);
	setQPainterPen(color, ls, lw);
	bool const text_is_antialiased = renderHints() & TextAntialiasing;
	setRenderHint(Antialiasing, text_is_antialiased);
	drawPath(bpath);
	if (fs != fill_none)
		fillPath(bpath, QBrush(color));
	setRenderHint(Antialiasing, false);
}


void GuiPainter::rectangle(int x, int y, int w, int h,
	Color col,
	line_style ls,
	int lw)
{
	if (!isDrawingEnabled())
		return;

	setQPainterPen(computeColor(col), ls, lw);
	drawRect(x, y, w, h);
}


void GuiPainter::fillRectangle(int x, int y, int w, int h, Color col)
{
	if (!isDrawingEnabled())
		return;

	fillRect(x, y, w, h, guiApp->colorCache().get(col));
}


void GuiPainter::arc(int x, int y, unsigned int w, unsigned int h,
	int a1, int a2, Color col)
{
	if (!isDrawingEnabled())
		return;

	// LyX usings 1/64ths degree, Qt usings 1/16th
	setQPainterPen(computeColor(col));
	bool const do_antialiasing = renderHints() & TextAntialiasing;
	setRenderHint(Antialiasing, do_antialiasing);
	drawArc(x, y, w, h, a1 / 4, a2 / 4);
	setRenderHint(Antialiasing, false);
}


void GuiPainter::image(int x, int y, int w, int h, graphics::Image const & i)
{
	graphics::GuiImage const & qlimage =
		static_cast<graphics::GuiImage const &>(i);

	fillRectangle(x, y, w, h, Color_graphicsbg);

	if (!isDrawingEnabled())
		return;

	QImage const image = qlimage.image();
	QRectF const drect = QRectF(x, y, w, h);
	QRectF const srect = QRectF(0, 0, image.width(), image.height());
	drawImage(drect, image, srect);
}


int GuiPainter::text(int x, int y, char_type c, FontInfo const & f)
{
	return text(x, y, docstring(1, c), f);
}


void GuiPainter::do_drawText(int x, int y, QString str, bool rtl, FontInfo const & f, QFont ff)
{
	setQPainterPen(computeColor(f.realColor()));
	if (font() != ff)
		setFont(ff);

	 /* In LyX, the character direction is forced by the language.
	  * Therefore, we have to signal that fact to Qt.
	  */
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
	drawText(x, y, str);
#else
	/* This looks like a cleaner solution, but it has drawbacks
	 * - does not work reliably (Mac OS X, ...)
	 * - it is not really documented
	 * Keep it here for now, in case it can be helpful
	 */
	//This is much stronger than setLayoutDirection.
	int flag = rtl ? Qt::TextForceRightToLeft : Qt::TextForceLeftToRight;
	drawText(x + (rtl ? textwidth : 0), y - fm.maxAscent(), 0, 0,
		 flag | Qt::TextDontClip,
		 str);
#endif
}


int GuiPainter::text(int x, int y, docstring const & s,
                     FontInfo const & f, bool const rtl,
                     double const wordspacing)
{
	//LYXERR0("text: x=" << x << ", s=" << s);
	if (s.empty())
		return 0;

	/* Caution: The following ucs4 to QString conversions work for symbol fonts
	only because they are no real conversions but simple casts in reality.
	When we want to draw a symbol or calculate the metrics we pass the position
	of the symbol in the font (as given in lib/symbols) as a char_type to the
	frontend. This is just wrong, because the symbol is no UCS4 character at
	all. You can think of this number as the code point of the symbol in a
	custom symbol encoding. It works because this char_type is lateron again
	interpreted as a position in the font again.
	The correct solution would be to have extra functions for symbols, but that
	would require to duplicate a lot of frontend and mathed support code.
	*/
	QString str = toqstr(s);

#if 0
	// HACK: QT3 refuses to show single compose characters
	//       Still needed with Qt4?
	if (ls == 1 && str[0].unicode() >= 0x05b0 && str[0].unicode() <= 0x05c2)
		str = ' ' + str;
#endif

	QFont ff = getFont(f);
	ff.setWordSpacing(wordspacing);
	GuiFontMetrics const & fm = getFontMetrics(f);

	// Here we use the font width cache instead of
	//   textwidth = fontMetrics().width(str);
	// because the above is awfully expensive on MacOSX
	// Note that we have to take in account space stretching (word spacing)
	int const textwidth = fm.width(s) + count(s.begin(), s.end(), ' ') * wordspacing;

	if (!isDrawingEnabled())
		return textwidth;

	textDecoration(f, x, y, textwidth);

	if (use_pixmap_cache_) {
		QPixmap pm;
		QString key = generateStringSignature(str, f);

		// Warning: Left bearing is in general negative! Only the case
		// where left bearing is negative is of interest WRT the
		// pixmap width and the text x-position.
		// Only the left bearing of the first character is important
		// as we always write from left to right, even for
		// right-to-left languages.
		// FIXME: this is probably broken for RTL now that we draw full strings.
		// Morover the first/last element is possibly not the right one since the glyph may have changed.
		int const lb = min(fm.lbearing(s[0]), 0);
		int const mA = fm.maxAscent();
		if (QPixmapCache::find(key, pm)) {
			// Draw the cached pixmap.
			drawPixmap(x + lb, y - mA, pm);
			return textwidth;
		}

		// Only the right bearing of the last character is
		// important as we always write from left to right,
		// even for right-to-left languages.
		int const rb = fm.rbearing(s[s.size()-1]);
		int const w = textwidth + rb - lb;
		int const mD = fm.maxDescent();
		int const h = mA + mD;
		if (w > 0 && h > 0) {
			pm = QPixmap(static_cast<int>(pixelRatio() * w),
						 static_cast<int>(pixelRatio() * h));
#if QT_VERSION >= 0x050000
			pm.setDevicePixelRatio(pixelRatio());
#endif
			pm.fill(Qt::transparent);
			GuiPainter p(&pm, pixelRatio());
			p.do_drawText(-lb, mA, str, rtl, f, ff);
			QPixmapCache::insert(key, pm);
			//LYXERR(Debug::PAINTING, "h=" << h << "  mA=" << mA << "  mD=" << mD
			//	<< "  w=" << w << "  lb=" << lb << "  tw=" << textwidth
			//	<< "  rb=" << rb);

			// Draw the new cached pixmap.
			drawPixmap(x + lb, y - mA, pm);
			//rectangle(x-lb, y-mA, w, h, Color_green);
		}
		return textwidth;
	}

	// don't use the pixmap cache,
	do_drawText(x, y, str, rtl, f, ff);
	//LYXERR(Debug::PAINTING, "draw " << string(str.toUtf8())
	//	<< " at " << x << "," << y);
	return textwidth;
}


int GuiPainter::text(int x, int y, docstring const & str, Font const & f,
                     double const wordspacing)
{
	return text(x, y, str, f.fontInfo(), f.isVisibleRightToLeft(), wordspacing);
}


int GuiPainter::text(int x, int y, docstring const & str, Font const & f,
                     Color other, size_type const from, size_type const to,
                     double const wordspacing)
{
	GuiFontMetrics const & fm = getFontMetrics(f.fontInfo());
	FontInfo fi = f.fontInfo();
	bool const rtl = f.isVisibleRightToLeft();

	// dimensions
	int const ascent = fm.maxAscent();
	int const height = fm.maxAscent() + fm.maxDescent();
	int xmin = fm.pos2x(str, from, rtl, wordspacing);
	int xmax = fm.pos2x(str, to, rtl, wordspacing);
	if (xmin > xmax)
		swap(xmin, xmax);

	// First the part in other color
	Color const orig = fi.realColor();
	fi.setPaintColor(other);
	QRegion const clip(x + xmin, y - ascent, xmax - xmin, height);
	setClipRegion(clip);
	int const textwidth = text(x, y, str, fi, rtl, wordspacing);

	// Then the part in normal color
	// Note that in Qt5, it is not possible to use Qt::UniteClip,
	// therefore QRegion is used.
	fi.setPaintColor(orig);
	QRegion region(viewport());
	setClipRegion(region - clip);
	text(x, y, str, fi, rtl, wordspacing);
	setClipping(false);

	return textwidth;
}


void GuiPainter::textDecoration(FontInfo const & f, int x, int y, int width)
{
	if (f.underbar() == FONT_ON)
		underline(f, x, y, width);
	if (f.strikeout() == FONT_ON)
		strikeoutLine(f, x, y, width);
	if (f.uuline() == FONT_ON)
		doubleUnderline(f, x, y, width);
	if (f.uwave() == FONT_ON)
		// f.color() doesn't work on some circumstances
		wavyHorizontalLine(x, y, width,  f.realColor().baseColor);
}


static int max(int a, int b) { return a > b ? a : b; }


void GuiPainter::button(int x, int y, int w, int h, bool mouseHover)
{
	if (mouseHover)
		fillRectangle(x, y, w, h, Color_buttonhoverbg);
	else
		fillRectangle(x, y, w, h, Color_buttonbg);
	buttonFrame(x, y, w, h);
}


void GuiPainter::buttonFrame(int x, int y, int w, int h)
{
	line(x, y, x, y + h - 1, Color_buttonframe);
	line(x - 1 + w, y, x - 1 + w, y + h - 1, Color_buttonframe);
	line(x, y - 1, x - 1 + w, y - 1, Color_buttonframe);
	line(x, y + h - 1, x - 1 + w, y + h - 1, Color_buttonframe);
}


void GuiPainter::rectText(int x, int y, docstring const & str,
	FontInfo const & font, Color back, Color frame)
{
	int width;
	int ascent;
	int descent;

	FontMetrics const & fm = theFontMetrics(font);
	fm.rectText(str, width, ascent, descent);

	if (back != Color_none)
		fillRectangle(x + 1, y - ascent + 1, width - 1,
			      ascent + descent - 1, back);

	if (frame != Color_none)
		rectangle(x, y - ascent, width, ascent + descent, frame);

	text(x + 3, y, str, font);
}


void GuiPainter::buttonText(int x, int y, docstring const & str,
	FontInfo const & font, bool mouseHover)
{
	int width;
	int ascent;
	int descent;

	FontMetrics const & fm = theFontMetrics(font);
	fm.buttonText(str, width, ascent, descent);

	static int const d = Inset::TEXT_TO_INSET_OFFSET / 2;

	button(x + d, y - ascent, width - d, descent + ascent, mouseHover);
	text(x + Inset::TEXT_TO_INSET_OFFSET, y, str, font);
}


int GuiPainter::preeditText(int x, int y, char_type c,
	FontInfo const & font, preedit_style style)
{
	FontInfo temp_font = font;
	FontMetrics const & fm = theFontMetrics(font);
	int ascent = fm.maxAscent();
	int descent = fm.maxDescent();
	int height = ascent + descent;
	int width = fm.width(c);

	switch (style) {
		case preedit_default:
			// default unselecting mode.
			fillRectangle(x, y - height + 1, width, height, Color_background);
			dashedUnderline(font, x, y - descent + 1, width);
			break;
		case preedit_selecting:
			// We are in selecting mode: white text on black background.
			fillRectangle(x, y - height + 1, width, height, Color_black);
			temp_font.setColor(Color_white);
			break;
		case preedit_cursor:
			// The character comes with a cursor.
			fillRectangle(x, y - height + 1, width, height, Color_background);
			underline(font, x, y - descent + 1, width);
			break;
	}
	text(x, y - descent + 1, c, temp_font);

	return width;
}


void GuiPainter::underline(FontInfo const & f, int x, int y, int width,
                           line_style ls)
{
	FontMetrics const & fm = theFontMetrics(f);
	int const pos = fm.underlinePos();

	line(x, y + pos, x + width, y + pos,
	     f.realColor(), ls, fm.lineWidth());
}


void GuiPainter::strikeoutLine(FontInfo const & f, int x, int y, int width)
{
	FontMetrics const & fm = theFontMetrics(f);
	int const pos = fm.strikeoutPos();

	line(x, y - pos, x + width, y - pos,
	     f.realColor(), line_solid, fm.lineWidth());
}


void GuiPainter::doubleUnderline(FontInfo const & f, int x, int y, int width)
{
	FontMetrics const & fm = theFontMetrics(f);
	int const pos1 = fm.underlinePos() + fm.lineWidth();
	int const pos2 = fm.underlinePos() - fm.lineWidth() + 1;

	line(x, y + pos1, x + width, y + pos1,
		 f.realColor(), line_solid, fm.lineWidth());
	line(x, y + pos2, x + width, y + pos2,
		 f.realColor(), line_solid, fm.lineWidth());
}


void GuiPainter::dashedUnderline(FontInfo const & f, int x, int y, int width)
{
	FontMetrics const & fm = theFontMetrics(f);

	int const below = max(fm.maxDescent() / 2, 2);
	int height = max((fm.maxDescent() / 4) - 1, 1);

	if (height >= 2)
		height += below;

	for (int n = 0; n != height; ++n)
		line(x, y + below + n, x + width, y + below + n, f.realColor(), line_onoffdash);
}


void GuiPainter::wavyHorizontalLine(int x, int y, int width, ColorCode col)
{
	setQPainterPen(computeColor(col));
	int const step = 2;
	int const xend = x + width;
	int height = 1;
	//FIXME: I am not sure if Antialiasing gives the best effect.
	//setRenderHint(Antialiasing, true);
	while (x < xend) {
		height = - height;
		drawLine(x, y - height, x + step, y + height);
		x += step;
		drawLine(x, y + height, x + step/2, y + height);
		x += step/2;
	}
	//setRenderHint(Antialiasing, false);
}

} // namespace frontend
} // namespace lyx
