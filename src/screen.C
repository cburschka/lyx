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
#include "font.h"
#include "insets/insettext.h"

using std::max;
using std::min;

static
GC createGC()
{
	XGCValues val;
	val.foreground = BlackPixel(fl_display, 
				    DefaultScreen(fl_display));
	
	val.function=GXcopy;
	val.graphics_exposures = false;
	val.line_style = LineSolid;
	val.line_width = 0;
	return XCreateGC(fl_display, RootWindow(fl_display, 0), 
			 GCForeground | GCFunction | GCGraphicsExposures
			 | GCLineWidth | GCLineStyle , &val);
}


// Constructor
LyXScreen::LyXScreen(WorkArea & o) //, LyXText * text_ptr)
	: owner(o), force_clear(true) //, text(text_ptr)
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


void LyXScreen::Redraw(LyXText * text)
{
	DrawFromTo(text, 0, owner.height());
	expose(0, 0, owner.workWidth(), owner.height());
	if (cursor_visible) {
		cursor_visible = false;
		ShowCursor(text);
	}
}


void LyXScreen::expose(int x, int y, int exp_width, int exp_height)
{
	XCopyArea(fl_display,
		  owner.getPixmap(),
		  owner.getWin(),
		  gc_copy,
		  x, y,
		  exp_width, exp_height,
		  x + owner.xpos(),
		  y + owner.ypos());
}


void LyXScreen::DrawFromTo(LyXText * text,
			   int y1, int y2, int y_offset, int x_offset)
{
	int y_text = text->first + y1;
   
	// get the first needed row 
	Row * row = text->GetRowNearY(y_text);
	// y_text is now the real beginning of the row
   
	int y = y_text - text->first;
	// y1 is now the real beginning of row on the screen
	
	while (row != 0 && y < y2) {
		LyXText::text_status st = text->status;
		do {
			text->status = st;
			text->GetVisibleRow(owner.owner(), y+y_offset,
					    x_offset, row, y + text->first);
		} while (text->status == LyXText::CHANGED_IN_DRAW);
		text->status = st;
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


void LyXScreen::DrawOneRow(LyXText * text, Row * row, int y_text,
			   int y_offset, int x_offset)
{
	int y = y_text - text->first + y_offset;
      
	if (y + row->height() > 0
	    && y - row->height() <= owner.height()) {
		// ok there is something visible
		LyXText::text_status st = text->status;
		do {
			text->status = st;
			text->GetVisibleRow(owner.owner(), y, x_offset, row,
					    y + text->first);
		} while (text->status == LyXText::CHANGED_IN_DRAW);
		text->status = st;
	}
	force_clear = false;
}


/* draws the screen, starting with textposition y. uses as much already
* printed pixels as possible */
void LyXScreen::Draw(LyXText * text, unsigned int y)
{
	if (cursor_visible) HideCursor();

	unsigned int old_first = text->first;
	text->first = y;

	// is any optimiziation possible?
	if ((y - old_first) < owner.height()
	    && (old_first - y) < owner.height()) {
		if (text->first < old_first) {
			DrawFromTo(text, 0, old_first - text->first);
			XCopyArea (fl_display,
				   owner.getWin(),
				   owner.getWin(),
				   gc_copy,
				   owner.xpos(),
				   owner.ypos(),
				   owner.workWidth(),
				   owner.height() - old_first + text->first,
				   owner.xpos(),
				   owner.ypos() + old_first - text->first
				);
			// expose the area drawn
			expose(0, 0,
			       owner.workWidth(),
			       old_first - text->first);
		} else  {
			DrawFromTo(text,
				   owner.height() + old_first - text->first,
				   owner.height());
			XCopyArea (fl_display,
				   owner.getWin(),
				   owner.getWin(),
				   gc_copy,
				   owner.xpos(),
				   owner.ypos() + text->first - old_first,
				   owner.workWidth(),
				   owner.height() + old_first - text->first,
				   owner.xpos(),
				   owner.ypos());
			// expose the area drawn
			expose(0, owner.height() + old_first - text->first,
			       owner.workWidth(), text->first - old_first);
		}
	} else {
		// make a dumb new-draw 
		DrawFromTo(text, 0, owner.height());
		expose(0, 0, owner.workWidth(), owner.height());
	}
}


void LyXScreen::ShowCursor(LyXText const * text)
{
	if (!cursor_visible) {
		Cursor_Shape shape = BAR_SHAPE;
		if (text->real_current_font.language() !=
		    owner.owner()->buffer()->params.language_info
		    || text->real_current_font.isVisibleRightToLeft()
		    != owner.owner()->buffer()->params.language_info->RightToLeft())
			shape = (text->real_current_font.isVisibleRightToLeft())
				? REVERSED_L_SHAPE : L_SHAPE;
		ShowManualCursor(text, text->cursor.x(), text->cursor.y(),
				 lyxfont::maxAscent(text->real_current_font),
				 lyxfont::maxDescent(text->real_current_font),
				 shape);
	}
}


/* returns true if first has changed, otherwise false */ 
bool LyXScreen::FitManualCursor(LyXText * text,
				int /*x*/, int y, int asc, int desc)
{
	int newtop = text->first;
  
	if (y + desc - text->first >= owner.height())
		newtop = y - 3 * owner.height() / 4;  // the scroll region must be so big!!
	else if (y - asc < text->first
		&& text->first > 0) {
		newtop = y - owner.height() / 4;
	}

	newtop = max(newtop, 0); // can newtop ever be < 0? (Lgb)
  
	if (newtop != text->first) {
		Draw(text, newtop);
		text->first = newtop;
		return true;
	}
	return false;
}


void LyXScreen::ShowManualCursor(LyXText const * text, int x, int y,
				 int asc, int desc, Cursor_Shape shape)
{
	unsigned int y1 = max(y - text->first - asc, 0U);
	typedef unsigned int uint;
	
	unsigned int y2 = min(y - text->first + desc, owner.height());

	// Secure against very strange situations
	y2 = max(y2, y1);
	
	if (cursor_pixmap){
		XFreePixmap(fl_display, cursor_pixmap);
		cursor_pixmap = 0;
	}

	if (y2 > 0 && y1 < owner.height()) {
		cursor_pixmap_h = y2 - y1 + 1;
		cursor_pixmap_y = y1;

		switch(shape) {
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
			XCreatePixmap (fl_display,
				       fl_root,
				       cursor_pixmap_w,
				       cursor_pixmap_h,
				       fl_get_visual_depth());
		XCopyArea (fl_display,
			   owner.getWin(),
			   cursor_pixmap,
			   gc_copy,
			   owner.xpos() + cursor_pixmap_x,
			   owner.ypos() + cursor_pixmap_y,
			   cursor_pixmap_w,
			   cursor_pixmap_h,
			   0, 0);
		XDrawLine(fl_display,
			  owner.getWin(),
			  gc_copy,
			  x + owner.xpos(),
			  y1 + owner.ypos(),
			  x + owner.xpos(),
			  y2 + owner.ypos());
		switch(shape) {
		case BAR_SHAPE:
			break;
		case L_SHAPE:
		case REVERSED_L_SHAPE:
			int rectangle_h = (cursor_pixmap_h+10)/20;
			XFillRectangle(fl_display,
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


void LyXScreen::HideCursor()
{
	if (!cursor_visible) return;

	if (cursor_pixmap){
		XCopyArea (fl_display, 
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


void LyXScreen::CursorToggle(LyXText const * text)
{
	if (cursor_visible)
		HideCursor();
	else
		ShowCursor(text);
}


/* returns a new top so that the cursor is visible */ 
unsigned int LyXScreen::TopCursorVisible(LyXText const * text)
{
	int newtop = text->first;

	if (text->cursor.y()
	    - text->cursor.row()->baseline()
	    + text->cursor.row()->height()
	    - text->first >= owner.height()) {
		if (text->cursor.row()->height() < owner.height()
		    && text->cursor.row()->height() > owner.height() / 4)
			newtop = text->cursor.y()
				+ text->cursor.row()->height()
				- text->cursor.row()->baseline() - owner.height();
		else
			newtop = text->cursor.y()
				- 3 * owner.height() / 4;   /* the scroll region must be so big!! */
	} else if (text->cursor.y() - text->cursor.row()->baseline() < text->first
		   && text->first > 0) {
		if (text->cursor.row()->height() < owner.height()
		    && text->cursor.row()->height() > owner.height() / 4)
			newtop = text->cursor.y() - text->cursor.row()->baseline();
		else {
			newtop = text->cursor.y() - owner.height() / 4;
			newtop = min(newtop, int(text->first));
		}
	}

	newtop = max(newtop, 0);

	return newtop;
}


/* scrolls the screen so that the cursor is visible, if necessary.
* returns true if a change was made, otherwise false */ 
bool LyXScreen::FitCursor(LyXText * text)
{
	// Is a change necessary?
	unsigned int newtop = TopCursorVisible(text);
	bool result = (newtop != text->first);
	if (result)
		Draw(text, newtop);
	return result;
}

   
void LyXScreen::Update(LyXText * text, int y_offset, int x_offset)
{
	switch(text->status) {
	case LyXText::NEED_MORE_REFRESH:
	{
		int y = max(int(text->refresh_y - text->first), 0);
		int height;
		if (text->inset_owner)
			height = text->inset_owner->ascent(owner.owner(),
							   text->real_current_font)
				+ text->inset_owner->descent(owner.owner(),
							     text->real_current_font);
		else
			height = owner.height();
		DrawFromTo(text, y, owner.height(), y_offset, x_offset);
		text->refresh_y = 0;
		text->status = LyXText::UNCHANGED;
		expose(0, y,
		       owner.workWidth(), owner.height() - y);
	}
	break;
	case LyXText::NEED_VERY_LITTLE_REFRESH:
	{
		// ok I will update the current cursor row
		DrawOneRow(text, text->refresh_row, text->refresh_y,
			   y_offset, x_offset);
		text->status = LyXText::UNCHANGED;
		expose(0, text->refresh_y - text->first + y_offset,
		       owner.workWidth(), text->refresh_row->height());
	}
	break;
	case LyXText::CHANGED_IN_DRAW: // just to remove the warning
	case LyXText::UNCHANGED:
		// Nothing needs done
		break;
	}
}


void LyXScreen::ToggleSelection(LyXText * text,  bool kill_selection,
				int y_offset, int x_offset)
{
	// only if there is a selection
	if (!text->selection) return;

	int bottom = min(max(text->sel_end_cursor.y()
			      - text->sel_end_cursor.row()->baseline()
			      + text->sel_end_cursor.row()->height(), text->first),
			  text->first + owner.height());
	int top = min(max(text->sel_start_cursor.y()
			   - text->sel_start_cursor.row()->baseline(), text->first),
		       text->first + owner.height());

	if (kill_selection)
		text->selection = 0;
	DrawFromTo(text, top - text->first, bottom - text->first,
		   y_offset, x_offset);
	expose(0, top - text->first,
	       owner.workWidth(),
	       bottom - text->first - (top - text->first));
}
  
   
void LyXScreen::ToggleToggle(LyXText * text, int y_offset, int x_offset)
{
	if (text->toggle_cursor.par() == text->toggle_end_cursor.par()
	    && text->toggle_cursor.pos() == text->toggle_end_cursor.pos())
		return;
	
	int top = text->toggle_cursor.y()
		- text->toggle_cursor.row()->baseline();
	int bottom = text->toggle_end_cursor.y()
		- text->toggle_end_cursor.row()->baseline() 
		+ text->toggle_end_cursor.row()->height();
	
	typedef unsigned int uint;
	
	bottom = min(max(uint(bottom), text->first), text->first + owner.height());
	top = min(max(uint(top), text->first), text->first + owner.height());

	DrawFromTo(text, top - text->first, bottom - text->first, y_offset,
		   x_offset);
	expose(0, top - text->first, owner.workWidth(),
	       bottom - text->first - (top - text->first));
}
