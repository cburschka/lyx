/* This file is part of
* ======================================================
*
*           LyX, The Document Processor
*
*	    Copyright 1995 Matthias Ettrich
*           Copyright 1995-1998 The LyX Team
*
* ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation "lyxscreen.h"
#endif

#include <algorithm>

#include "lyxscreen.h"
#include "lyxtext.h"
#include "lyxrow.h"
#include "Painter.h"
#include "WorkArea.h"
#include "buffer.h"
#include "BufferView.h"
#include "font.h"
#include "insets/insettext.h"
#include "ColorHandler.h"
#include "language.h"

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
			 | GCLineWidth | GCLineStyle , &val);
}

} // namespace anon


// Constructor
LyXScreen::LyXScreen(WorkArea & o)
	: owner(o), force_clear(true)
{
	// the cursor isnt yet visible
	cursor_visible = false;
	cursor_pixmap = 0;
	cursor_pixmap_x = 0;
	cursor_pixmap_y = 0;
	cursor_pixmap_w = 0;
	cursor_pixmap_h = 0;

	// We need this GC
	gc_copy = createGC();
}


LyXScreen::~LyXScreen()
{
	XFreeGC(fl_get_display(), gc_copy);
}


void LyXScreen::setCursorColor()
{
	if (!lyxColorHandler.get()) return;

	GC gc = lyxColorHandler->getGCForeground(LColor::cursor);

	XGCValues val;
	XGetGCValues(fl_get_display(),
		     gc, GCForeground, &val);
	XChangeGC(fl_get_display(), gc_copy, GCForeground, &val);
}


void LyXScreen::redraw(LyXText * text, BufferView * bv)
{
	drawFromTo(text, bv, 0, owner.height(), 0, 0, text == bv->text);
	expose(0, 0, owner.workWidth(), owner.height());
	if (cursor_visible) {
		cursor_visible = false;
		bv->showCursor();
	}
}


void LyXScreen::expose(int x, int y, int exp_width, int exp_height)
{
	XCopyArea(fl_get_display(),
		  owner.getPixmap(),
		  owner.getWin(),
		  gc_copy,
		  x, y,
		  exp_width, exp_height,
		  x + owner.xpos(),
		  y + owner.ypos());
}


void LyXScreen::drawFromTo(LyXText * text, BufferView * bv,
			   int y1, int y2, int y_offset, int x_offset,
			   bool internal)
{
	int y_text = text->first_y + y1;

	// get the first needed row
	Row * row = text->getRowNearY(y_text);
	// y_text is now the real beginning of the row

	int y = y_text - text->first_y;
	// y1 is now the real beginning of row on the screen

	while (row != 0 && y < y2) {
		LyXText::text_status st = text->status();
		text->getVisibleRow(bv, y + y_offset,
				    x_offset, row, y + text->first_y);
		internal = internal && (st != LyXText::CHANGED_IN_DRAW);
		while (internal && text->status() == LyXText::CHANGED_IN_DRAW) {
			text->fullRebreak(bv);
			st = LyXText::NEED_MORE_REFRESH;
			text->setCursor(bv, text->cursor.par(),
					text->cursor.pos());
			text->status(bv, st);
			// we should be sure our row-pointer is still valid, so it's
			// better to recompute it.
			y_text = y + text->first_y;
			row = text->getRowNearY(y_text);
			y = y_text - text->first_y;
			text->getVisibleRow(bv, y + y_offset,
					    x_offset, row, y + text->first_y);
		}
		y += row->height();
		row = row->next();
	}
	force_clear = false;

	// maybe we have to clear the screen at the bottom
	if ((y < y2) && text->bv_owner) {
		owner.getPainter().fillRectangle(0, y,
						 owner.workWidth(),
						 y2 - y,
					       LColor::bottomarea);
	}
}


void LyXScreen::drawOneRow(LyXText * text, BufferView * bv, Row * row,
			   int y_text, int y_offset, int x_offset)
{
	int const y = y_text - text->first_y + y_offset;

	if (((y + row->height()) > 0) &&
	    ((y - row->height()) <= static_cast<int>(owner.height()))) {
		// ok there is something visible
		text->getVisibleRow(bv, y, x_offset, row, y + text->first_y);
	}
	force_clear = false;
}


/* draws the screen, starting with textposition y. uses as much already
 * printed pixels as possible */
void LyXScreen::draw(LyXText * text, BufferView * bv, unsigned int y)
{
	if (cursor_visible) hideCursor();

	int const old_first = text->first_y;
	bool internal = (text == bv->text);
	text->first_y = y;

	// is any optimiziation possible?
	if ((y - old_first) < owner.height()
	    && (old_first - y) < owner.height())
	{
		if (text->first_y < old_first) {
			drawFromTo(text, bv, 0,
				   old_first - text->first_y, 0, 0, internal);
			XCopyArea (fl_get_display(),
				   owner.getWin(),
				   owner.getWin(),
				   gc_copy,
				   owner.xpos(),
				   owner.ypos(),
				   owner.workWidth(),
				   owner.height() - old_first + text->first_y,
				   owner.xpos(),
				   owner.ypos() + old_first - text->first_y
				);
			// expose the area drawn
			expose(0, 0,
			       owner.workWidth(),
			       old_first - text->first_y);
		} else  {
			drawFromTo(text, bv,
				   owner.height() + old_first - text->first_y,
				   owner.height(), 0, 0, internal);
			XCopyArea (fl_get_display(),
				   owner.getWin(),
				   owner.getWin(),
				   gc_copy,
				   owner.xpos(),
				   owner.ypos() + text->first_y - old_first,
				   owner.workWidth(),
				   owner.height() + old_first - text->first_y,
				   owner.xpos(),
				   owner.ypos());
			// expose the area drawn
			expose(0, owner.height() + old_first - text->first_y,
			       owner.workWidth(), text->first_y - old_first);
		}
	} else {
		// make a dumb new-draw
		drawFromTo(text, bv, 0, owner.height(), 0, 0, internal);
		expose(0, 0, owner.workWidth(), owner.height());
	}
}


void LyXScreen::showCursor(LyXText const * text, BufferView const * bv)
{
	if (!cursor_visible) {
		Cursor_Shape shape = BAR_SHAPE;
		if (text->real_current_font.language() !=
		    bv->buffer()->params.language
		    || text->real_current_font.isVisibleRightToLeft()
		    != bv->buffer()->params.language->RightToLeft())
			shape = (text->real_current_font.isVisibleRightToLeft())
				? REVERSED_L_SHAPE : L_SHAPE;
		showManualCursor(text, text->cursor.x(), text->cursor.y(),
				 lyxfont::maxAscent(text->real_current_font),
				 lyxfont::maxDescent(text->real_current_font),
				 shape);
	}
}


/* returns true if first has changed, otherwise false */
bool LyXScreen::fitManualCursor(LyXText * text, BufferView * bv,
				int /*x*/, int y, int asc, int desc)
{
	int newtop = text->first_y;

	if (y + desc - text->first_y >= static_cast<int>(owner.height()))
		newtop = y - 3 * owner.height() / 4;  // the scroll region must be so big!!
	else if (y - asc < text->first_y
		&& text->first_y > 0) {
		newtop = y - owner.height() / 4;
	}

	newtop = max(newtop, 0); // can newtop ever be < 0? (Lgb)

	if (newtop != text->first_y) {
		draw(text, bv, newtop);
		text->first_y = newtop;
		return true;
	}
	return false;
}


void LyXScreen::showManualCursor(LyXText const * text, int x, int y,
				 int asc, int desc, Cursor_Shape shape)
{
	// Update the cursor color.
	setCursorColor();

	int const y1 = max(y - text->first_y - asc, 0);
	int const y_tmp = min(y - text->first_y + desc,
			      static_cast<int>(owner.height()));

	// Secure against very strange situations
	int const y2 = max(y_tmp, y1);

	if (cursor_pixmap) {
		XFreePixmap(fl_get_display(), cursor_pixmap);
		cursor_pixmap = 0;
	}

	if (y2 > 0 && y1 < int(owner.height())) {
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
			   owner.getWin(),
			   cursor_pixmap,
			   gc_copy,
			   owner.xpos() + cursor_pixmap_x,
			   owner.ypos() + cursor_pixmap_y,
			   cursor_pixmap_w,
			   cursor_pixmap_h,
			   0, 0);
		XDrawLine(fl_get_display(),
			  owner.getWin(),
			  gc_copy,
			  x + owner.xpos(),
			  y1 + owner.ypos(),
			  x + owner.xpos(),
			  y2 + owner.ypos());
		switch (shape) {
		case BAR_SHAPE:
			break;
		case L_SHAPE:
		case REVERSED_L_SHAPE:
			int const rectangle_h = (cursor_pixmap_h + 10) / 20;
			XFillRectangle(fl_get_display(),
				       owner.getWin(),
				       gc_copy,
				       cursor_pixmap_x + owner.xpos(),
				       y2 - rectangle_h + 1 + owner.ypos(),
				       cursor_pixmap_w - 1, rectangle_h);
			break;
		}

	}
	cursor_visible = true;
}


void LyXScreen::hideCursor()
{
	if (!cursor_visible) return;

	if (cursor_pixmap) {
		XCopyArea (fl_get_display(),
			   cursor_pixmap,
			   owner.getWin(),
			   gc_copy,
			   0, 0,
			   cursor_pixmap_w, cursor_pixmap_h,
			   cursor_pixmap_x + owner.xpos(),
			   cursor_pixmap_y + owner.ypos());
	}
	cursor_visible = false;
}


void LyXScreen::cursorToggle(BufferView * bv) const
{
	if (cursor_visible)
		bv->hideCursor();
	else
		bv->showCursor();
}


/* returns a new top so that the cursor is visible */
unsigned int LyXScreen::topCursorVisible(LyXText const * text)
{
	int newtop = text->first_y;

	Row * row = text->cursor.row();

	// Is this a hack? Yes, probably... (Lgb)
	if (!row)
		return max(newtop, 0);

	if (text->cursor.y() - row->baseline() + row->height()
	    - text->first_y >= owner.height()) {
		if (row->height() < owner.height()
		    && row->height() > owner.height() / 4) {
			newtop = text->cursor.y()
				+ row->height()
				- row->baseline() - owner.height();
		} else {
			// scroll down
			newtop = text->cursor.y()
				- owner.height() / 2;   /* the scroll region must be so big!! */
		}

	} else if (static_cast<int>((text->cursor.y()) - row->baseline()) <
		   text->first_y && text->first_y > 0) {
		if (row->height() < owner.height()
		    && row->height() > owner.height() / 4) {
			newtop = text->cursor.y() - row->baseline();
		} else {
			// scroll up
			newtop = text->cursor.y() - owner.height() / 2;
			newtop = min(newtop, text->first_y);
		}
	}

	newtop = max(newtop, 0);

	return newtop;
}


/* scrolls the screen so that the cursor is visible, if necessary.
* returns true if a change was made, otherwise false */
bool LyXScreen::fitCursor(LyXText * text, BufferView * bv)
{
	// Is a change necessary?
	int const newtop = topCursorVisible(text);
	bool const result = (newtop != text->first_y);
	if (result)
		draw(text, bv, newtop);
	return result;
}


void LyXScreen::update(LyXText * text, BufferView * bv,
		       int y_offset, int x_offset)
{
	switch (text->status()) {
	case LyXText::NEED_MORE_REFRESH:
	{
		int const y = max(int(text->refresh_y - text->first_y), 0);
		drawFromTo(text, bv, y, owner.height(), y_offset, x_offset);
		text->refresh_y = 0;
		// otherwise this is called ONLY from BufferView_pimpl(update)
		// or we should see to set this flag accordingly
		if (text != bv->text)
			text->status(bv, LyXText::UNCHANGED);
		expose(0, y, owner.workWidth(), owner.height() - y);
	}
	break;
	case LyXText::NEED_VERY_LITTLE_REFRESH:
	{
		// ok I will update the current cursor row
		drawOneRow(text, bv, text->refresh_row, text->refresh_y,
			   y_offset, x_offset);
		// this because if we had a major update the refresh_row could
		// have been set to 0!
		if (text->refresh_row) {
			// otherwise this is called ONLY from BufferView_pimpl(update)
			// or we should see to set this flag accordingly
			if (text != bv->text)
				text->status(bv, LyXText::UNCHANGED);
			expose(0, text->refresh_y - text->first_y + y_offset,
				   owner.workWidth(), text->refresh_row->height());
		}
	}
	break;
	case LyXText::CHANGED_IN_DRAW: // just to remove the warning
	case LyXText::UNCHANGED:
		// Nothing needs done
		break;
	}
}


void LyXScreen::toggleSelection(LyXText * text, BufferView * bv,
				bool kill_selection,
				int y_offset, int x_offset)
{
	// only if there is a selection
	if (!text->selection.set()) return;

	int const bottom = min(
		max(static_cast<int>(text->selection.end.y()
				     - text->selection.end.row()->baseline()
				     + text->selection.end.row()->height()),
		    text->first_y),
		static_cast<int>(text->first_y + owner.height()));
	int const top = min(
		max(static_cast<int>(text->selection.start.y() -
				     text->selection.start.row()->baseline()),
		    text->first_y),
		static_cast<int>(text->first_y + owner.height()));

	if (kill_selection)
		text->selection.set(false);
	drawFromTo(text, bv, top - text->first_y, bottom - text->first_y,
		   y_offset, x_offset);
	expose(0, top - text->first_y,
	       owner.workWidth(),
	       bottom - text->first_y - (top - text->first_y));
}


void LyXScreen::toggleToggle(LyXText * text, BufferView * bv,
			     int y_offset, int x_offset)
{
	if (text->toggle_cursor.par() == text->toggle_end_cursor.par()
	    && text->toggle_cursor.pos() == text->toggle_end_cursor.pos())
		return;

	int const top_tmp = text->toggle_cursor.y()
		- text->toggle_cursor.row()->baseline();
	int const bottom_tmp = text->toggle_end_cursor.y()
		- text->toggle_end_cursor.row()->baseline()
		+ text->toggle_end_cursor.row()->height();

	int const offset = y_offset < 0 ? y_offset : 0;
	int const bottom = min(max(bottom_tmp, text->first_y),
		     static_cast<int>(text->first_y + owner.height()))-offset;
	int const top = min(max(top_tmp, text->first_y),
		  static_cast<int>(text->first_y + owner.height()))-offset;

	drawFromTo(text, bv, top - text->first_y,
		   bottom - text->first_y, y_offset,
		   x_offset);
	expose(0, top - text->first_y, owner.workWidth(),
	       bottom - text->first_y - (top - text->first_y));
}
