/**
 * \file xscreen.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "xscreen.h"

#include "ColorHandler.h"
#include "XWorkArea.h"

#include "debug.h"
#include "LColor.h"

using std::endl;


namespace {

GC createGC()
{
	XGCValues val;
	val.foreground = BlackPixel(fl_get_display(),
				    DefaultScreen(fl_get_display()));

	val.function = GXcopy;
	val.graphics_exposures = false;
	val.line_style = LineSolid;
	val.line_width = 0;
	return XCreateGC(fl_get_display(), RootWindow(fl_get_display(), 0),
			 GCForeground | GCFunction | GCGraphicsExposures
			 | GCLineWidth | GCLineStyle, &val);
}

} // namespace anon


XScreen::XScreen(XWorkArea & o)
	: LyXScreen(), owner_(o), nocursor_pixmap_(0),
	cursor_x_(0), cursor_y_(0), cursor_w_(0), cursor_h_(0)
{
	// We need this GC
	gc_copy = createGC();
}


XScreen::~XScreen()
{
	XFreeGC(fl_get_display(), gc_copy);
}


WorkArea & XScreen::workarea() const
{
	return owner_;
}


void XScreen::setCursorColor()
{
	if (!lyxColorHandler.get())
		return;

	GC gc = lyxColorHandler->getGCForeground(LColor::cursor);

	XGCValues val;
	XGetGCValues(fl_get_display(),
		     gc, GCForeground, &val);
	XChangeGC(fl_get_display(), gc_copy, GCForeground, &val);
}


void XScreen::showCursor(int x, int y, int h, Cursor_Shape shape)
{
	// Update the cursor color. (a little slow doing it like this ??)
	setCursorColor();

	cursor_x_ = x;
	cursor_y_ = y;
	cursor_h_ = h;

	switch (shape) {
		case BAR_SHAPE:
			cursor_w_ = 1;
			break;
		case L_SHAPE:
			cursor_w_ = cursor_h_ / 3;
			break;
		case REVERSED_L_SHAPE:
			cursor_w_ = cursor_h_ / 3;
			cursor_x_ = x - cursor_w_ + 1;
			break;
	}

	if (nocursor_pixmap_) {
		XFreePixmap(fl_get_display(), nocursor_pixmap_);
		nocursor_pixmap_ = 0;
	}
	nocursor_pixmap_ = XCreatePixmap(fl_get_display(),
		fl_root, cursor_w_, cursor_h_, fl_get_visual_depth());

	// save old area
	XCopyArea(fl_get_display(),
		owner_.getWin(), nocursor_pixmap_, gc_copy,
		owner_.xpos() + cursor_x_,
		owner_.ypos() + cursor_y_,
		cursor_w_, cursor_h_, 0, 0);

// xforms equivalent needed here
#if 0
	if (!qApp->focusWidget())
		return;
#endif

	XDrawLine(fl_get_display(), owner_.getWin(), gc_copy,
		owner_.xpos() + x, owner_.ypos() + y,
		owner_.xpos() + x, owner_.ypos() + y + h - 1);

	switch (shape) {
		case BAR_SHAPE:
			break;
		case REVERSED_L_SHAPE:
		case L_SHAPE:
			XDrawLine(fl_get_display(), owner_.getWin(), gc_copy,
				owner_.xpos() + cursor_x_,
				owner_.ypos() + y + h - 1,
				owner_.xpos() + cursor_x_ + cursor_w_ - 1,
				owner_.ypos() + y + h - 1);
			break;
	}
}


void XScreen::removeCursor()
{
	// before first showCursor
	if (!nocursor_pixmap_)
		return;

	XCopyArea(fl_get_display(), nocursor_pixmap_, owner_.getWin(),
		gc_copy, 0, 0, cursor_w_, cursor_h_,
		owner_.xpos() + cursor_x_,
		owner_.ypos() + cursor_y_);
}


void XScreen::expose(int x, int y, int w, int h)
{
	lyxerr[Debug::GUI] << "XScreen::expose " << w << 'x' << h
		<< '+' << x << '+' << y << endl;

	XEvent ev;

	ev.type = Expose;
	ev.xexpose.window = owner_.getWin();
	// Adjust the x,y data so that XWorkArea can handle XEvents
	// received from here in identical fashion to those it receives
	// direct from X11.
	ev.xexpose.x = owner_.xpos() + x;
	ev.xexpose.y = owner_.ypos() + y;
	ev.xexpose.width = w;
	ev.xexpose.height = h;
	ev.xexpose.count = 0;

	XSendEvent(fl_get_display(), owner_.getWin(), False, 0, &ev);
}
