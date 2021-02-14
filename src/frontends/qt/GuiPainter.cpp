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
#include "LyXRC.h"

#include "support/debug.h"
#include "support/lassert.h"
#include "support/lyxlib.h"

#include <algorithm>

#include <QTextLayout>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

const int Painter::thin_line = 1;

GuiPainter::GuiPainter(QPaintDevice * device, double pixel_ratio, bool devel_mode)
	: QPainter(device), Painter(pixel_ratio, devel_mode)
{
	// set cache correctly
	current_color_ = pen().color();
	current_ls_ = pen().style() == Qt::DotLine ? line_onoffdash : line_solid;
	current_lw_ = pen().width();
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
	case line_solid:
	case line_solid_aliased:
		pen.setStyle(Qt::SolidLine); break;
	case line_onoffdash:
		pen.setStyle(Qt::DotLine); break;
	}

	pen.setWidth(lw);

	setPen(pen);
}


QColor GuiPainter::computeColor(Color col)
{
	return filterColor(guiApp->colorCache().get(col));
}


QColor GuiPainter::filterColor(QColor const & col)
{
	if (monochrome_blend_.empty())
		return col;

	QColor const blend = monochrome_blend_.top();
	return QColor::fromHsv(blend.hue(), blend.saturation(), qGray(col.rgb()));
}


void GuiPainter::enterMonochromeMode(Color const & blend)
{
	QColor qblend = filterColor(guiApp->colorCache().get(blend));
	monochrome_blend_.push(qblend);
}


void GuiPainter::leaveMonochromeMode()
{
	LASSERT(!monochrome_blend_.empty(), return);
	monochrome_blend_.pop();
}


void GuiPainter::point(int x, int y, Color col)
{
	setQPainterPen(computeColor(col));
	drawPoint(x, y);
}


void GuiPainter::line(int x1, int y1, int x2, int y2,
	Color col,
	line_style ls,
	int lw)
{
	setQPainterPen(computeColor(col), ls, lw);
	bool const do_antialiasing = renderHints() & TextAntialiasing
		&& x1 != x2 && y1 != y2 && ls != line_solid_aliased;
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
	// double the size if needed
	// FIXME THREAD
	static QVector<QPoint> points(32);
	if (np > points.size())
		points.resize(2 * np);

	// Note: the proper way to not get blurry vertical and horizontal lines is
	// to add 0.5 to all coordinates.
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
	setRenderHint(Antialiasing,
	              antialias && text_is_antialiased && ls != line_solid_aliased);
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
	setRenderHint(Antialiasing, text_is_antialiased && ls != line_solid_aliased);
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
	setQPainterPen(computeColor(col), ls, lw);
	drawRect(x, y, w, h);
}


void GuiPainter::fillRectangle(int x, int y, int w, int h, Color col)
{
	fillRect(x, y, w, h, guiApp->colorCache().get(col));
}


void GuiPainter::arc(int x, int y, unsigned int w, unsigned int h,
	int a1, int a2, Color col)
{
	// LyX usings 1/64ths degree, Qt usings 1/16th
	setQPainterPen(computeColor(col));
	bool const do_antialiasing = renderHints() & TextAntialiasing;
	setRenderHint(Antialiasing, do_antialiasing);
	drawArc(x, y, w, h, a1 / 4, a2 / 4);
	setRenderHint(Antialiasing, false);
}


void GuiPainter::image(int x, int y, int w, int h, graphics::Image const & i,
		       bool const revert_in_darkmode)
{
	graphics::GuiImage const & qlimage =
		static_cast<graphics::GuiImage const &>(i);

	fillRectangle(x, y, w, h, Color_graphicsbg);

	QImage image = qlimage.image();

	if (revert_in_darkmode && guiApp && guiApp->colorCache().isDarkMode())
		// FIXME this is only a cheap approximation
		// Ideally, replace colors as in GuiApplication::prepareForDarkmode()
		image.invertPixels();

	QRectF const drect = QRectF(x, y, w, h);
	QRectF const srect = QRectF(0, 0, image.width(), image.height());
	// Bilinear filtering is needed on a rare occasion for instant previews when
	// the user's configuration mixes low-dpi and high-dpi monitors (#10114).
	// This filter is optimised by qt on pixel-aligned images, so this does not
	// affect performances in other cases.
	setRenderHint(SmoothPixmapTransform);
	drawImage(drect, image, srect);
	setRenderHint(SmoothPixmapTransform, false);
}


void GuiPainter::text(int x, int y, char_type c, FontInfo const & f)
{
	text(x, y, docstring(1, c), f);
}


void GuiPainter::text(int x, int y, docstring const & s, FontInfo const & f)
{
	text(x, y, s, f, Auto, 0.0, 0.0);
}


void GuiPainter::text(int x, int y, docstring const & s,
                      FontInfo const & f, Direction const dir,
                      double const wordspacing, double const tw)
{
	//LYXERR0("text: x=" << x << ", s=" << s);
	if (s.empty())
		return;

	/* Caution: The following ucs4 to QString conversions work for symbol fonts
	only because they are no real conversions but simple casts in reality.
	When we want to draw a symbol or calculate the metrics we pass the position
	of the symbol in the font (as given in lib/symbols) as a char_type to the
	frontend. This is just wrong, because the symbol is no UCS4 character at
	all. You can think of this number as the code point of the symbol in a
	custom symbol encoding. It works because this char_type is later on again
	interpreted as a position in the font.
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

	int textwidth = 0;
	if (tw == 0.0)
		// Take into account space stretching (word spacing)
		textwidth = fm.width(s) +
			static_cast<int>(fm.countExpanders(s) * wordspacing);
	else
		textwidth = static_cast<int>(tw);

	textDecoration(f, x, y, textwidth);

	setQPainterPen(computeColor(f.realColor()));
	if (dir != Auto) {
		auto ptl = fm.getTextLayout(s, dir == RtL, wordspacing);
		QTextLine const & tline = ptl->lineForTextPosition(0);
		ptl->draw(this, QPointF(x, y - tline.ascent()));
	} else {
		if (font() != ff)
			setFont(ff);
		drawText(x, y, str);
	}
	//LYXERR(Debug::PAINTING, "draw " << string(str.toUtf8())
	//	<< " at " << x << "," << y);
}


void GuiPainter::text(int x, int y, docstring const & str, Font const & f,
                      double const wordspacing, double const tw)
{
	text(x, y, str, f.fontInfo(), f.isVisibleRightToLeft() ? RtL : LtR,
	     wordspacing, tw);
}


void GuiPainter::text(int x, int y, docstring const & str, Font const & f,
                      Color other, size_type const from, size_type const to,
                      double const wordspacing, double const tw)
{
	GuiFontMetrics const & fm = getFontMetrics(f.fontInfo());
	FontInfo fi = f.fontInfo();
	Direction const dir = f.isVisibleRightToLeft() ? RtL : LtR;

	// dimensions
	int const ascent = fm.maxAscent();
	int const height = fm.maxAscent() + fm.maxDescent();
	int xmin = fm.pos2x(str, from, dir == RtL, wordspacing);
	int xmax = fm.pos2x(str, to, dir == RtL, wordspacing);
	// Avoid this case, since it would make the `other' text spill in some cases
	if (xmin == xmax) {
		text(x, y, str, fi, dir, wordspacing, tw);
		return;
	} else if (xmin > xmax)
		swap(xmin, xmax);

	// First the part in other color
	Color const orig = fi.realColor();
	fi.setPaintColor(other);
	QRegion const clip(x + xmin, y - ascent, xmax - xmin, height);
	setClipRegion(clip);
	text(x, y, str, fi, dir, wordspacing, tw);

	// Then the part in normal color
	// Note that in Qt5, it is not possible to use Qt::UniteClip,
	// therefore QRegion is used.
	fi.setPaintColor(orig);
	QRegion region(viewport());
	setClipRegion(region - clip);
	text(x, y, str, fi, dir, wordspacing, tw);
	setClipping(false);
}


void GuiPainter::textDecoration(FontInfo const & f, int x, int y, int width)
{
	if (f.underbar() == FONT_ON)
		underline(f, x, y, width);
	if (f.strikeout() == FONT_ON)
		strikeoutLine(f, x, y, width);
	if (f.xout() == FONT_ON)
		crossoutLines(f, x, y, width);
	if (f.uuline() == FONT_ON)
		doubleUnderline(f, x, y, width);
	if (f.uwave() == FONT_ON)
		// f.color() doesn't work on some circumstances
		wavyHorizontalLine(x, y, width,  f.realColor().baseColor);
}


static int max(int a, int b) { return a > b ? a : b; }


void GuiPainter::rectText(int x, int y, docstring const & str,
	FontInfo const & font, Color back, Color frame)
{
	int width, ascent, descent;

	FontMetrics const & fm = theFontMetrics(font);
	fm.rectText(str, width, ascent, descent);

	if (back != Color_none)
		fillRectangle(x + 1, y - ascent + 1, width - 1,
			      ascent + descent - 1, back);

	if (frame != Color_none)
		rectangle(x, y - ascent, width, ascent + descent, frame);

	// FIXME: let offset depend on font
	text(x + 3, y, str, font);
}


void GuiPainter::buttonText(int x, int baseline, docstring const & s,
	FontInfo const & font, Color back, Color frame, int offset)
{
	int width, ascent, descent;

	FontMetrics const & fm = theFontMetrics(font);
	fm.buttonText(s, offset, width, ascent, descent);

	int const d = offset / 2;

	fillRectangle(x + d, baseline - ascent, width - offset,
			      ascent + descent, back);
	rectangle(x + d, baseline - ascent, width - offset, ascent + descent, frame);
	text(x + offset, baseline, s, font);
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


void GuiPainter::crossoutLines(FontInfo const & f, int x, int y, int width)
{
	FontInfo tmpf = f;
	tmpf.setXout(FONT_OFF);

	// the definition of \xout in ulem.sty is
    //  \def\xout{\bgroup \markoverwith{\hbox to.35em{\hss/\hss}}\ULon}
	// Let's mimic it somewhat.
	double offset = max(0.35 * theFontMetrics(tmpf).em(), 1);
	for (int i = 0 ; i < iround(width / offset) ; ++i)
		text(x + iround(i * offset), y, '/', tmpf);
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
