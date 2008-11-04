/**
 * \file QLPainter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QLPainter.h"

#include "GuiApplication.h"
#include "GuiFontMetrics.h"
#include "QLImage.h"

#include "GuiApplication.h"
#include "qt_helpers.h"

#include "debug.h"
#include "Language.h"
#include "Color.h"
#include "LyXRC.h"

#include "support/unicode.h"

#include <QPixmapCache>
#include <QTextLayout>

// Set USE_PIXMAP_CACHE to 1 for enabling the use of a Pixmap cache when
// drawing text. This is especially useful for older PPC/Mac systems.
#if (QT_VERSION < 0x040200) || defined(Q_WS_X11)
#define USE_PIXMAP_CACHE 0
#else
#define USE_PIXMAP_CACHE 1
#endif

using std::endl;
using std::string;

namespace lyx {
namespace frontend {

namespace {


QString generateStringSignature(QString const & str, Font const & f)
{
	QString sig = str;
	sig.append(QChar(static_cast<short>(f.family())));
	sig.append(QChar(static_cast<short>(f.series())));
	sig.append(QChar(static_cast<short>(f.realShape())));
	sig.append(QChar(static_cast<short>(f.size())));
	sig.append(QChar(static_cast<short>(f.color())));
	return sig;
}

} // anon namespace

QLPainter::QLPainter(QPaintDevice * device)
	: QPainter(device), Painter(),
	use_pixmap_cache_(lyxrc.use_pixmap_cache && USE_PIXMAP_CACHE)
{
	// new QPainter has default QPen:
	current_color_ = Color::black;
	current_ls_ = line_solid;
	current_lw_ = line_thin;
}


QLPainter::~QLPainter()
{
	QPainter::end();
	//lyxerr << "QLPainter::end()" << endl;
}


void QLPainter::setQPainterPen(Color_color col,
	Painter::line_style ls, Painter::line_width lw)
{
	if (col == current_color_ && ls == current_ls_ && lw == current_lw_)
		return;

	current_color_ = col;
	current_ls_ = ls;
	current_lw_ = lw;

	QPen pen = QPainter::pen();

	pen.setColor(guiApp->colorCache().get(col));

	switch (ls) {
		case line_solid: pen.setStyle(Qt::SolidLine); break;
		case line_onoffdash: pen.setStyle(Qt::DotLine); break;
	}

	switch (lw) {
		case line_thin: pen.setWidth(0); break;
		case line_thick: pen.setWidth(3); break;
	}

	setPen(pen);
}


void QLPainter::point(int x, int y, Color_color col)
{
	if (!isDrawingEnabled())
		return;

	setQPainterPen(col);
	drawPoint(x, y);
}


void QLPainter::line(int x1, int y1, int x2, int y2,
	Color_color col,
	line_style ls,
	line_width lw)
{
	if (!isDrawingEnabled())
		return;

	setQPainterPen(col, ls, lw);
	bool const do_antialiasing = renderHints() & TextAntialiasing
		&& x1 != x2 && y1 != y2;
	setRenderHint(Antialiasing, do_antialiasing);
	drawLine(x1, y1, x2, y2);
	setRenderHint(Antialiasing, false);
}


void QLPainter::lines(int const * xp, int const * yp, int np,
	Color_color col,
	line_style ls,
	line_width lw)
{
	if (!isDrawingEnabled())
		return;

	// double the size if needed
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
	setQPainterPen(col, ls, lw);
	bool const text_is_antialiased = renderHints() & TextAntialiasing;
	setRenderHint(Antialiasing, antialias && text_is_antialiased);
	drawPolyline(points.data(), np);
	setRenderHint(Antialiasing, false);
}


void QLPainter::rectangle(int x, int y, int w, int h,
	Color_color col,
	line_style ls,
	line_width lw)
{
	if (!isDrawingEnabled())
		return;

	setQPainterPen(col, ls, lw);
	drawRect(x, y, w, h);
}


void QLPainter::fillRectangle(int x, int y, int w, int h, Color_color col)
{
	fillRect(x, y, w, h, guiApp->colorCache().get(col));
}


void QLPainter::arc(int x, int y, unsigned int w, unsigned int h,
	int a1, int a2, Color_color col)
{
	if (!isDrawingEnabled())
		return;

	// LyX usings 1/64ths degree, Qt usings 1/16th
	setQPainterPen(col);
	bool const do_antialiasing = renderHints() & TextAntialiasing;
	setRenderHint(Antialiasing, do_antialiasing);
	drawArc(x, y, w, h, a1 / 4, a2 / 4);
	setRenderHint(Antialiasing, false);
}


void QLPainter::image(int x, int y, int w, int h, graphics::Image const & i)
{
	graphics::QLImage const & qlimage =
		static_cast<graphics::QLImage const &>(i);

	fillRectangle(x, y, w, h, Color::graphicsbg);

	if (!isDrawingEnabled())
		return;

	drawImage(x, y, qlimage.qimage(), 0, 0, w, h);
}


int QLPainter::text(int x, int y, char_type c, Font const & f)
{
	docstring s(1, c);
	return text(x, y, s, f);
}


int QLPainter::smallCapsText(int x, int y,
	QString const & s, Font const & f)
{
	Font smallfont(f);
	smallfont.decSize().decSize().setShape(Font::UP_SHAPE);

	QFont const & qfont = guiApp->guiFontLoader().get(f);
	QFont const & qsmallfont = guiApp->guiFontLoader().get(smallfont);

	setQPainterPen(f.realColor());
	int textwidth = 0;
	size_t const ls = s.length();
	for (unsigned int i = 0; i < ls; ++i) {
		QChar const c = s[i].toUpper();
		if (c != s.at(i)) {
			setFont(qsmallfont);
		} else {
			setFont(qfont);
		}
		if (isDrawingEnabled())
			drawText(x + textwidth, y, c);
		textwidth += fontMetrics().width(c);
	}
	return textwidth;
}


int QLPainter::text(int x, int y, docstring const & s,
		Font const & f)
{
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

	QLFontInfo & fi = guiApp->guiFontLoader().fontinfo(f);

	int textwidth;

	if (f.realShape() == Font::SMALLCAPS_SHAPE) {
		textwidth = smallCapsText(x, y, str, f);
		if (f.underbar() == Font::ON)
			underline(f, x, y, textwidth);
		return textwidth;
	}

	// Here we use the font width cache instead of
	//   textwidth = fontMetrics().width(str);
	// because the above is awfully expensive on MacOSX
	textwidth = fi.metrics->width(s);
	if (f.underbar() == Font::ON)
		underline(f, x, y, textwidth);

	if (!isDrawingEnabled())
		return textwidth;

	// Qt4 does not display a glyph whose codepoint is the
	// same as that of a soft-hyphen (0x00ad), unless it
	// occurs at a line-break. As a kludge, we force Qt to
	// render this glyph using a one-column line.
	if (s.size() == 1 && str[0].unicode() == 0x00ad) {
		setQPainterPen(f.realColor());
		QTextLayout adsymbol(str);
		adsymbol.setFont(fi.font);
		adsymbol.beginLayout();
		QTextLine line = adsymbol.createLine();
		line.setNumColumns(1);
		line.setPosition(QPointF(0, -line.ascent()));
		adsymbol.endLayout();
		line.draw(this, QPointF(x, y));
		return textwidth;
	}

	if (use_pixmap_cache_) {
		QPixmap pm;
		QString key = generateStringSignature(str, f);
		// Warning: Left bearing is in general negative! Only the case
		// where left bearing is negative is of interest WRT the the 
		// pixmap width and the text x-position.
		// Only the left bearing of the first character is important
		// as we always write from left to right, even for
		// right-to-left languages.
		int const lb = std::min(fi.metrics->lbearing(s[0]), 0);
		int const mA = fi.metrics->maxAscent();
		if (QPixmapCache::find(key, pm)) {
                        // Draw the cached pixmap.
			drawPixmap(x + lb, y - mA, pm);
			return textwidth;
		}

		// Only the right bearing of the last character is
		// important as we always write from left to right,
		// even for right-to-left languages.
		int const rb = fi.metrics->rbearing(s[s.size()-1]);
		int const w = textwidth + rb - lb;
		int const mD = fi.metrics->maxDescent();
		int const h = mA + mD;
		if (w > 0 && h > 0) {
			pm = QPixmap(w, h);
			pm.fill(Qt::transparent);
			QLPainter p(&pm);
			p.setQPainterPen(f.realColor());
			if (p.font() != fi.font)
				p.setFont(fi.font);
			// We need to draw the text as LTR as we use our own bidi code.
			p.setLayoutDirection(Qt::LeftToRight);
			p.drawText(-lb, mA, str);
			QPixmapCache::insert(key, pm);
			/*
			LYXERR(Debug::PAINTING) << "h=" << h << "  mA=" << mA << "  mD=" << mD
				<< "  w=" << w << "  lb=" << lb << "  tw=" << textwidth
				<< "  rb=" << rb << endl;
			*/
			// Draw the new cached pixmap.
			drawPixmap(x + lb, y - mA, pm);
			return textwidth;
		}
	}

	// don't use the pixmap cache,
	// draw directly onto the painting device
	setQPainterPen(f.realColor());
	if (font() != fi.font)
		setFont(fi.font);
	// We need to draw the text as LTR as we use our own bidi code.
	QPainter::setLayoutDirection(Qt::LeftToRight);
	drawText(x, y, str);
	//LYXERR(Debug::PAINTING) << "draw " << std::string(str.toUtf8())
	//	<< " at " << x << "," << y << std::endl;
	return textwidth;
}


} // namespace frontend
} // namespace lyx
