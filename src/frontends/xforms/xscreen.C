/**
 * \file xscreen.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <algorithm>
#include <X11/Xlib.h>

#include "frontends/screen.h"
#include "frontends/font_metrics.h"
#include "XWorkArea.h"
#include "xscreen.h"
#include "lyxtext.h"
#include "lyxrow.h"
#include "Painter.h"
#include "WorkArea.h"
#include "buffer.h"
#include "BufferView.h"
#include "insets/insettext.h"
#include "ColorHandler.h"
#include "language.h"
#include "debug.h"

using std::endl;
using std::max;
using std::min;

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


// Constructor
XScreen::XScreen(XWorkArea & o)
	: LyXScreen(), owner_(o)
{
	// the cursor isnt yet visible
	cursor_pixmap = 0;
	cursor_pixmap_x = 0;
	cursor_pixmap_y = 0;
	cursor_pixmap_w = 0;
	cursor_pixmap_h = 0;

	// We need this GC
	gc_copy = createGC();
}


XScreen::~XScreen()
{
	XFreeGC(fl_get_display(), gc_copy);
}


void XScreen::setCursorColor()
{
	if (!lyxColorHandler.get()) return;

	GC gc = lyxColorHandler->getGCForeground(LColor::cursor);

	XGCValues val;
	XGetGCValues(fl_get_display(),
		     gc, GCForeground, &val);
	XChangeGC(fl_get_display(), gc_copy, GCForeground, &val);
}


void XScreen::showManualCursor(LyXText const * text, int x, int y,
				 int asc, int desc, Cursor_Shape shape)
{
	// Update the cursor color.
	setCursorColor();

	int const y1 = max(y - text->first_y - asc, 0);
	int const y_tmp = min(y - text->first_y + desc,
			      static_cast<int>(owner_.workHeight()));

	// Secure against very strange situations
	int const y2 = max(y_tmp, y1);

	if (cursor_pixmap) {
		XFreePixmap(fl_get_display(), cursor_pixmap);
		cursor_pixmap = 0;
	}

	if (y2 > 0 && y1 < int(owner_.workHeight())) {
		cursor_pixmap_h = y2 - y1 + 1;
		cursor_pixmap_y = y1;

		switch (shape) {
		case BAR_SHAPE:
			cursor_pixmap_w = 1;
			cursor_pixmap_x = x;
			break;
		case L_SHAPE:
			cursor_pixmap_w = cursor_pixmap_h/3;
			cursor_pixmap_x = x;
			break;
		case REVERSED_L_SHAPE:
			cursor_pixmap_w = cursor_pixmap_h/3;
			cursor_pixmap_x = x - cursor_pixmap_w + 1;
			break;
		}

		cursor_pixmap =
			XCreatePixmap (fl_get_display(),
				       fl_root,
				       cursor_pixmap_w,
				       cursor_pixmap_h,
				       fl_get_visual_depth());
		XCopyArea (fl_get_display(),
			   owner_.getWin(),
			   cursor_pixmap,
			   gc_copy,
			   owner_.xpos() + cursor_pixmap_x,
			   owner_.ypos() + cursor_pixmap_y,
			   cursor_pixmap_w,
			   cursor_pixmap_h,
			   0, 0);
		XDrawLine(fl_get_display(),
			  owner_.getWin(),
			  gc_copy,
			  x + owner_.xpos(),
			  y1 + owner_.ypos(),
			  x + owner_.xpos(),
			  y2 + owner_.ypos());
		switch (shape) {
		case BAR_SHAPE:
			break;
		case L_SHAPE:
		case REVERSED_L_SHAPE:
			int const rectangle_h = (cursor_pixmap_h + 10) / 20;
			XFillRectangle(fl_get_display(),
				       owner_.getWin(),
				       gc_copy,
				       cursor_pixmap_x + owner_.xpos(),
				       y2 - rectangle_h + 1 + owner_.ypos(),
				       cursor_pixmap_w - 1, rectangle_h);
			break;
		}

	}
	cursor_visible_ = true;
}


void XScreen::hideCursor()
{
	if (!cursor_visible_) return;

	if (cursor_pixmap) {
		XCopyArea (fl_get_display(),
			   cursor_pixmap,
			   owner_.getWin(),
			   gc_copy,
			   0, 0,
			   cursor_pixmap_w, cursor_pixmap_h,
			   cursor_pixmap_x + owner_.xpos(),
			   cursor_pixmap_y + owner_.ypos());
	}
	cursor_visible_ = false;
}


void XScreen::expose(int x, int y, int w, int h)
{
	lyxerr[Debug::GUI] << "expose " << w << "x" << h
		<< "+" << x << "+" << y << endl;
	XCopyArea(fl_get_display(),
		  owner_.getPixmap(),
		  owner_.getWin(),
		  gc_copy,
		  x, y, w, h,
		  x + owner_.xpos(),
		  y + owner_.ypos());
}


void XScreen::draw(LyXText * text, BufferView * bv, unsigned int y)
{
	if (cursor_visible_) hideCursor();

	int const old_first = text->first_y;
	bool const internal = (text == bv->text);
	text->first_y = y;

	// is any optimization possible?
	if ((y - old_first) < owner_.workHeight()
	    && (old_first - y) < owner_.workHeight())
	{
		if (text->first_y < old_first) {
			drawFromTo(text, bv, 0,
				   old_first - text->first_y, 0, 0, internal);
			XCopyArea (fl_get_display(),
				   owner_.getWin(),
				   owner_.getWin(),
				   gc_copy,
				   owner_.xpos(),
				   owner_.ypos(),
				   owner_.workWidth(),
				   owner_.workHeight() - old_first + text->first_y,
				   owner_.xpos(),
				   owner_.ypos() + old_first - text->first_y
				);
			// expose the area drawn
			expose(0, 0,
			       owner_.workWidth(),
			       old_first - text->first_y);
		} else  {
			drawFromTo(text, bv,
				   owner_.workHeight() + old_first - text->first_y,
				   owner_.workHeight(), 0, 0, internal);
			XCopyArea (fl_get_display(),
				   owner_.getWin(),
				   owner_.getWin(),
				   gc_copy,
				   owner_.xpos(),
				   owner_.ypos() + text->first_y - old_first,
				   owner_.workWidth(),
				   owner_.workHeight() + old_first - text->first_y,
				   owner_.xpos(),
				   owner_.ypos());
			// expose the area drawn
			expose(0, owner_.workHeight() + old_first - text->first_y,
			       owner_.workWidth(), text->first_y - old_first);
		}
	} else {
		// make a dumb new-draw
		drawFromTo(text, bv, 0, owner_.workHeight(), 0, 0, internal);
		expose(0, 0, owner_.workWidth(), owner_.workHeight());
	}

	XSync(fl_get_display(), 0);
}
