/**
 * \file GPainter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <gtkmm.h>

#include "GPainter.h"
#include "debug.h"
#include "GWorkArea.h"
#include "lyxrc.h"
#include "encoding.h"
#include "language.h"
#include "LColor.h"
#include "xftFontLoader.h"
#include "xformsImage.h"
#include "frontends/font_metrics.h"
#include "codeConvert.h"

#include "support/lstrings.h"

#include <boost/scoped_array.hpp>

#include <X11/Xft/Xft.h>

#include <cmath>

using std::string;

namespace font_metrics {

int width(wchar_t const *s, size_t n, LyXFont const & f);

} // namespace font_metrics


namespace lyx {
namespace frontend {

GPainter::GPainter(GWorkArea & xwa)
	: Painter(), owner_(xwa)
{
}


int GPainter::paperWidth() const
{
	return owner_.workWidth();
}


int GPainter::paperHeight() const
{
	return owner_.workHeight();
}


void GPainter::setForeground(Glib::RefPtr<Gdk::GC> gc, LColor_color clr)
{
	Gdk::Color * gclr = owner_.getColorHandler().getGdkColor(clr);
	gc->set_foreground(*gclr);
}


void GPainter::setLineParam(Glib::RefPtr<Gdk::GC> gc,
			    line_style ls, line_width lw)
{
	int width = 0;
	switch (lw) {
	case Painter::line_thin:
		width = 0;
		break;
	case Painter::line_thick:
		width = 2;
		break;
	}

	Gdk::LineStyle style = Gdk::LINE_SOLID;
	switch (ls) {
	case Painter::line_solid:
		style = Gdk::LINE_SOLID;
		break;
	case Painter::line_onoffdash:
		style = Gdk::LINE_ON_OFF_DASH;
		break;
	}
	gc->set_line_attributes(width, style,
				Gdk::CAP_NOT_LAST, Gdk::JOIN_MITER);
}


void GPainter::point(int x, int y, LColor_color c)
{
	setForeground(owner_.getGC(), c);
	owner_.getPixmap()->draw_point(owner_.getGC(), x, y);
}


void GPainter::line(int x1, int y1,
	int x2, int y2,
	LColor_color col,
	line_style ls,
	line_width lw)
{
	setForeground(owner_.getGC(), col);
	setLineParam(owner_.getGC(), ls, lw);
	owner_.getPixmap()->draw_line(owner_.getGC(), x1, y1, x2, y2);
}


void GPainter::lines(int const * xp, int const * yp, int np,
	LColor_color col,
	line_style ls,
	line_width lw)
{
	setForeground(owner_.getGC(), col);
	setLineParam(owner_.getGC(), ls, lw);
	std::vector<Gdk::Point> points(np);

	for (int i = 0; i < np; ++i) {
		points[i].set_x(xp[i]);
		points[i].set_y(yp[i]);
	}
	owner_.getPixmap()->draw_lines(owner_.getGC(), points);
}


void GPainter::rectangle(int x, int y, int w, int h,
	LColor_color col,
	line_style ls,
	line_width lw)
{
	setForeground(owner_.getGC(), col);
	setLineParam(owner_.getGC(), ls, lw);
	owner_.getPixmap()->draw_rectangle(owner_.getGC(), false, x, y, w, h);
}


void GPainter::fillRectangle(int x, int y, int w, int h,
	LColor_color col)
{
	setForeground(owner_.getGC(), col);
	owner_.getPixmap()->draw_rectangle(owner_.getGC(), true, x, y, w, h);
}


void GPainter::fillPolygon(int const * xp, int const * yp,
	int np, LColor_color col)
{
	setForeground(owner_.getGC(), col);
	std::vector<Gdk::Point> points(np);

	for (int i = 0; i < np; ++i) {
		points[i].set_x(xp[i]);
		points[i].set_y(yp[i]);
	}
	owner_.getPixmap()->draw_polygon(owner_.getGC(), true, points);
}


void GPainter::arc(int x, int y, unsigned int w, unsigned int h,
	int a1, int a2, LColor_color col)
{
	setForeground(owner_.getGC(), col);
	owner_.getPixmap()->draw_arc(owner_.getGC(),
				     false, x, y, w, h, a1, a2);
}


void GPainter::image(int x, int y, int w, int h,
	graphics::Image const & i)
{
	graphics::xformsImage const & image =
		static_cast<graphics::xformsImage const &>(i);
	Pixmap pixmap = GDK_PIXMAP_XID(owner_.getPixmap()->gobj());
	GC gc = GDK_GC_XGC(owner_.getGC()->gobj());
	XCopyArea(owner_.getDisplay(), image.getPixmap(), pixmap,
		  gc, 0, 0, w, h, x, y);
}


void GPainter::text(int x, int y, std::string const & s, LyXFont const & f)
{
	size_t size = s.length() + 1;
	wchar_t * wcs = (wchar_t *) alloca(size * sizeof(wchar_t));
	size = mbstowcs(wcs, s.c_str(), size);
	return text(x, y, wcs, size, f);
}


void GPainter::text(int x, int y, char c, LyXFont const & f)
{
	char s[2] = { c, '\0' };
	text(x, y, s, 1, f);
}


inline XftFont * getXftFont(LyXFont const & f)
{
	return fontLoader.load(f.family(), f.series(),
			       f.realShape(), f.size());
}


void GPainter::text(int x, int y, wchar_t const * s, int ls, LyXFont const & f)
{
	XftFont * font = getXftFont(f);
	XftColor * xftClr = owner_.getColorHandler().
		getXftColor(f.realColor());
//	getXftColor(f.realColor());
	XftDraw * draw = owner_.getXftDraw();
	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		XftDrawString32(draw, xftClr, font, x, y,
				wcsToXftChar32StrFast(s), ls);
	} else {
		LyXFont smallfont(f);
		smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		XftFont * fontS = getXftFont(smallfont);
		wchar_t c;
		int tmpx = x;
		for(int i = 0; i < ls; ++i) {
			c = support::uppercase(s[i]);
			if(c != s[i]) {
				XftDrawString32(draw, xftClr, fontS, tmpx, y,
						wcsToXftChar32StrFast(&c), 1);
				tmpx += font_metrics::width(c, smallfont);
			} else {
				XftDrawString32(draw, xftClr, font, tmpx, y,
						wcsToXftChar32StrFast(&c), 1);
				tmpx += font_metrics::width(c, f);
			}
		}
	}
	if (f.underbar() == LyXFont::ON)
		underline(f, x, y, font_metrics::width(s, ls, f));
}


void GPainter::text(int x, int y, char const * s, size_t ls, LyXFont const & f)
{
	boost::scoped_array<wchar_t> wcs(new wchar_t[ls + 1]);
	size_t len;
	if (fontLoader.isSpecial(f)) {
		unsigned char const * us =
			reinterpret_cast<unsigned char const *>(s);
		len = ls;
		std::copy(us, us + ls, wcs.get());
	} else
		len = mbstowcs(wcs.get(), s, ls + 1);
	text(x, y, wcs.get(), len, f);
}

} // namespace frontend
} // namespace lyx
